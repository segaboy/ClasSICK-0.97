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


def hard_disk(payload, entry, *, bss_end=None, poison_bss=False, verify_payload=False):
    if not isinstance(payload, bytes):
        raise TypeError('payload must be bytes')
    if not 1 <= len(payload) <= MAX_PAYLOAD or not 0 <= entry < len(payload):
        raise ValueError('payload or entry outside diagnostic loader bounds')
    if bss_end is not None and (not isinstance(bss_end, int) or not len(payload) <= bss_end <= 0xf000):
        raise ValueError('BSS end outside diagnostic near-data bounds')
    if poison_bss and bss_end is None:
        raise ValueError('BSS poison requires a BSS range')
    # The test medium is explicitly drive 80h and has the model geometry
    # above. Real BIOS drive selection and geometry require separate work.
    code = bytearray.fromhex('fa31c08ed0bc007c8ed8fbb800108ec0')
    if poison_bss and bss_end > len(payload):
        code += bytes.fromhex('fc bf') + struct.pack('<H', len(payload))
        code += b'\xb9' + struct.pack('<H', bss_end-len(payload)) + bytes.fromhex('b0a5 f3aa')
    remaining = (len(payload)+SECTOR-1)//SECTOR
    read_branches = []
    for capacity, cx, dx, bx in TRACKS:
        count = min(remaining, capacity)
        if not count:
            break
        code += b'\xbb' + struct.pack('<H', bx) + b'\xb8' + bytes((count, 2))
        code += b'\xb9' + struct.pack('<H', cx) + b'\xba' + struct.pack('<H', dx)
        code += bytes.fromhex('cd1372')
        read_branches.append(len(code));code.append(0);remaining -= count
    checksum_branch = None
    if verify_payload:
        # Same limited additive check as the separate floppy diagnostic.
        code += bytes.fromhex('b80010 8ed8 31db 31f6 b9') + struct.pack('<H', len(payload))
        code += bytes.fromhex('fc 31c0 ac 01c3 e2fb 81fb') + struct.pack('<H', sum(payload)&0xffff)
        code += b'\x75';checksum_branch = len(code);code.append(0)
    if bss_end is not None and bss_end > len(payload):
        code += bytes.fromhex('b80010 8ec0 fc bf') + struct.pack('<H', len(payload))
        code += b'\xb9' + struct.pack('<H', bss_end-len(payload)) + bytes.fromhex('30c0 f3aa')
    code += bytes.fromhex('fab800108ed88ed0bcfefffbea') + struct.pack('<HH', entry, 0x1000)
    read_failure = len(code);code += bytes.fromhex('b0eee680ebfe')
    checksum_failure = len(code)
    if verify_payload:
        code += bytes.fromhex('b0ede680ebfe')
    for offset, target in [(i, read_failure) for i in read_branches] + (
            [(checksum_branch, checksum_failure)] if checksum_branch is not None else []):
        delta = target-(offset+1)
        if not -128 <= delta <= 127:
            raise ValueError('diagnostic branch out of range')
        code[offset] = delta & 255
    if len(code) > 510:
        raise ValueError('diagnostic boot sector overflow')
    code += bytes(510-len(code)) + b'\x55\xaa'
    return bytes(code) + payload + bytes(DISK_BYTES-SECTOR-len(payload))
