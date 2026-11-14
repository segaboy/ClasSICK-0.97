#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Original four-track diagnostic loader for an already linked 286 payload.

This builds test media only. It is not a production loader or installer.
"""
import struct

SECTOR=512
TRACKS=((14,0x0002,0x0000,0),(15,0x0001,0x0100,14*SECTOR),
        (15,0x0101,0x0000,29*SECTOR),(15,0x0101,0x0100,44*SECTOR))
MAX_PAYLOAD=59*SECTOR
FLOPPY_BYTES=1228800

def floppy(payload,entry,*,bss_end=None,poison_bss=False,poison_stack=None):
    if not isinstance(payload,bytes):raise TypeError('payload must be bytes')
    sectors=(len(payload)+SECTOR-1)//SECTOR
    if not 1<=len(payload)<=MAX_PAYLOAD or not 0<=entry<len(payload):
        raise ValueError('payload or entry outside diagnostic loader bounds')
    if bss_end is not None and (not isinstance(bss_end,int) or not len(payload)<=bss_end<=0xf000):
        raise ValueError('BSS end outside diagnostic near-data bounds')
    if poison_bss and bss_end is None:raise ValueError('BSS poison requires a BSS range')
    if poison_stack is not None and (bss_end is None or not isinstance(poison_stack,int) or not 0<=poison_stack<=255):
        raise ValueError('stack poison requires a BSS range and a byte')
    # Read each bounded track into 1000:0000; on any failure emit EE.
    code=bytearray.fromhex('fa31c08ed0bc007c8ed8fbb800108ec0')
    if poison_bss and bss_end>len(payload):
        # Test-only: establish nonzero initial memory before the BIOS reads.
        code+=bytes.fromhex('fc bf')+struct.pack('<H',len(payload))+b'\xb9'+struct.pack('<H',bss_end-len(payload))
        code+=bytes.fromhex('b0a5 f3aa')
    remaining=sectors;branches=[]
    for capacity,cx,dx,bx in TRACKS:
        count=min(remaining,capacity)
        if not count:break
        code+=b'\xbb'+struct.pack('<H',bx)+b'\xb8'+bytes((count,2))
        code+=b'\xb9'+struct.pack('<H',cx)+b'\xba'+struct.pack('<H',dx)+bytes.fromhex('cd1372')
        branches.append(len(code));code.append(0);remaining-=count
    if bss_end is not None and bss_end>len(payload):
        # The last read may include sector padding. Clear all linked BSS and
        # alignment padding explicitly; never assume power-on RAM is zero.
        code+=bytes.fromhex('b80010 8ec0 fc bf')+struct.pack('<H',len(payload))
        code+=b'\xb9'+struct.pack('<H',bss_end-len(payload))+bytes.fromhex('30c0 f3aa')
    if poison_stack is not None:
        # Diagnostic instrumentation only; the bootstrap still uses SS=0.
        code+=bytes.fromhex('b80010 8ec0 fc bf')+struct.pack('<H',bss_end)
        code+=b'\xb9'+struct.pack('<H',0xfffe-bss_end)+b'\xb0'+bytes([poison_stack])+bytes.fromhex('f3aa')
    code+=bytes.fromhex('fab800108ed88ed0bcfefffbea')+struct.pack('<HH',entry,0x1000)
    failure=len(code);code+=bytes.fromhex('b0eee680ebfe')
    for at in branches:
        delta=failure-(at+1)
        if not -128<=delta<=127:raise ValueError('loader branch out of range')
        code[at]=delta&255
    if len(code)>510:raise ValueError('loader exceeds boot sector')
    code+=bytes(510-len(code))+b'\x55\xaa'
    return bytes(code)+payload+bytes(FLOPPY_BYTES-SECTOR-len(payload))
