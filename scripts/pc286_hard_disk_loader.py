#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Build a raw fixed-disk diagnostic for BootHush's documented 286 model.

This owns the entire generated image. It is not an installer, partitioned
disk, production boot loader, or evidence of a physical machine's geometry.
"""
import struct

SECTOR = 512
DISK_BYTES = 615 * 4 * 17 * SECTOR
MAX_PAYLOAD = 59 * SECTOR
# Cylinder 0, heads 0..3; sector 1 of head 0 is occupied by the boot sector.
TRACKS = ((16, 0x0002, 0x0080, 0),
          (17, 0x0001, 0x0180, 16 * SECTOR),
          (17, 0x0001, 0x0280, 33 * SECTOR),
          (9, 0x0001, 0x0380, 50 * SECTOR))


def crc16(payload):
    """CRC-16, polynomial 0x1021, initial 0xffff, no reflection or final XOR."""
    value = 0xffff
    for byte in payload:
        value ^= byte << 8
        for _ in range(8):
            value = ((value << 1) ^ (0x1021 if value & 0x8000 else 0)) & 0xffff
    return value


def _retry_bios_call():
    # PUSH/POP leave the BIOS carry result intact while retaining our counter.
    return bytes.fromhex('57 cd13 5f')


def hard_disk(payload, entry, *, bss_end=None, poison_bss=False, verify_payload=False,
              verify_crc16=False, verify_geometry=False, retry_reads=False):
    if not isinstance(payload, bytes):
        raise TypeError('payload must be bytes')
    if not 1 <= len(payload) <= MAX_PAYLOAD or not 0 <= entry < len(payload):
        raise ValueError('payload or entry outside diagnostic loader bounds')
    if bss_end is not None and (not isinstance(bss_end, int) or not len(payload) <= bss_end <= 0xf000):
        raise ValueError('BSS end outside diagnostic near-data bounds')
    if poison_bss and bss_end is None:
        raise ValueError('BSS poison requires a BSS range')
    if verify_payload and verify_crc16:
        raise ValueError('choose one diagnostic payload check')
    # The test medium is explicitly drive 80h and has the model geometry
    # above. Real BIOS drive selection and geometry require separate work.
    code = bytearray.fromhex('fa31c08ed0bc007c8ed8fbb800108ec0')
    geometry_branches = []
    if verify_geometry:
        # Refuse a different built-in model geometry before reading payload.
        # This tests the model contract, not a physical-drive safety check.
        code += bytes.fromhex('b80008 ba8000 cd13 72')
        geometry_branches.append(len(code));code.append(0)
        code += bytes.fromhex('81f99166 75') # CX: max cylinder 614, 17 sectors
        geometry_branches.append(len(code));code.append(0)
        code += bytes.fromhex('81fa0103 75') # DX: max head 3, one drive
        geometry_branches.append(len(code));code.append(0)
        code += bytes.fromhex('eb06')
        geometry_failure = len(code);code += bytes.fromhex('b0efe680ebfe')
        for offset in geometry_branches:
            code[offset] = (geometry_failure-offset-1) & 255
        code += bytes.fromhex('b80010 8ec0') # restore destination segment
    if poison_bss and bss_end > len(payload):
        code += bytes.fromhex('fc bf') + struct.pack('<H', len(payload))
        code += b'\xb9' + struct.pack('<H', bss_end-len(payload)) + bytes.fromhex('b0a5 f3aa')
    remaining = (len(payload)+SECTOR-1)//SECTOR
    read_branches = [];near_fail_branches = []
    for capacity, cx, dx, bx in TRACKS:
        count = min(remaining, capacity)
        if not count:
            break
        if not retry_reads:
            code += b'\xbb' + struct.pack('<H', bx) + b'\xb8' + bytes((count, 2))
            code += b'\xb9' + struct.pack('<H', cx) + b'\xba' + struct.pack('<H', dx)
            code += bytes.fromhex('cd1372')
            read_branches.append(len(code));code.append(0)
        else:
            # Save the attempt counter around each modeled firmware call.
            code += bytes.fromhex('bf0300');retry_start=len(code)
            code += bytes.fromhex('b80010 8ec0')
            code += b'\xbb'+struct.pack('<H',bx)+b'\xb8'+bytes((count,2))
            code += b'\xb9'+struct.pack('<H',cx)+b'\xba'+struct.pack('<H',dx)
            code += _retry_bios_call()+b'\x73';success=len(code);code.append(0)
            # Decrement remaining attempts; zero takes the near failure jump.
            code += bytes.fromhex('4f 7503 e9');near_fail_branches.append(len(code));code += b'\0\0'
            code += bytes.fromhex('b80000 ba8000')+_retry_bios_call()+bytes.fromhex('7303 e9')
            near_fail_branches.append(len(code));code += b'\0\0'
            code += b'\xeb';back=len(code);code.append(0)
            for offset,target in ((success,len(code)),(back,retry_start)):
                delta=target-offset-1
                if not -128<=delta<=127:raise ValueError('retry branch out of range')
                code[offset]=delta & 255
        remaining -= count
    checksum_branch = None
    if verify_payload:
        # Same limited additive check as the separate floppy diagnostic.
        code += bytes.fromhex('b80010 8ed8 31db 31f6 b9') + struct.pack('<H', len(payload))
        code += bytes.fromhex('fc 31c0 ac 01c3 e2fb 81fb') + struct.pack('<H', sum(payload)&0xffff)
        code += b'\x75';checksum_branch = len(code);code.append(0)
    elif verify_crc16:
        # Original bitwise implementation of the specified CRC. Preserve CX
        # around the eight-bit inner loop; cover only linked payload bytes.
        code += bytes.fromhex('b80010 8ed8 bbffff 31f6 b9') + struct.pack('<H',len(payload))
        code += bytes.fromhex('fc')
        byte_start = len(code)
        code += bytes.fromhex('ac 30c7 51 b108') # LODSB; XOR AL,BH; PUSH CX; CL=8
        bit_start = len(code)
        code += bytes.fromhex('d1e3 7304 81f32110 fec9 75')
        code.append((bit_start-(len(code)+1)) & 255)
        code += bytes.fromhex('59 e2')
        code.append((byte_start-(len(code)+1)) & 255)
        code += bytes.fromhex('81fb') + struct.pack('<H',crc16(payload))
        code += b'\x75';checksum_branch = len(code);code.append(0)
    if bss_end is not None and bss_end > len(payload):
        code += bytes.fromhex('b80010 8ec0 fc bf') + struct.pack('<H', len(payload))
        code += b'\xb9' + struct.pack('<H', bss_end-len(payload)) + bytes.fromhex('30c0 f3aa')
    code += bytes.fromhex('fab800108ed88ed0bcfefffbea') + struct.pack('<HH', entry, 0x1000)
    read_failure = len(code);code += bytes.fromhex('b0eee680ebfe')
    checksum_failure = len(code)
    if verify_payload or verify_crc16:
        code += bytes.fromhex('b0ede680ebfe')
    for offset, target in [(i, read_failure) for i in read_branches] + (
            [(checksum_branch, checksum_failure)] if checksum_branch is not None else []):
        delta = target-(offset+1)
        if not -128 <= delta <= 127:
            raise ValueError('diagnostic branch out of range')
        code[offset] = delta & 255
    for offset in near_fail_branches:
        delta=read_failure-(offset+2)
        if not -32768<=delta<=32767:raise ValueError('retry failure branch out of range')
        code[offset:offset+2]=struct.pack('<h',delta)
    if len(code) > 510:
        raise ValueError('diagnostic boot sector overflow')
    code += bytes(510-len(code)) + b'\x55\xaa'
    return bytes(code) + payload + bytes(DISK_BYTES-SECTOR-len(payload))
