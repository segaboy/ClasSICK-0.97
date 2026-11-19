#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Original CODE metadata on generated MFS; no disk or implementation input."""
import hashlib
import struct
from resource_media_fixture import _item, _resource_fork


def original_media(rows=180, segments=20, *, after_header=False):
    if not 1 <= segments <= rows <= 8192:
        raise ValueError('original CODE fixture dimensions')
    table = bytearray()
    resources = []
    first = 0
    for index in range(segments):
        count = rows//segments + (index < rows % segments)
        body = struct.pack('>HH', first*8, count) + bytes((index*17+j*11)&255 for j in range(16))
        resources.append(_item(index+1, body))
        for slot in range(count):
            routine=(0 if after_header else 4)+2*(slot % 8)
            table.extend(struct.pack('>HHHH', routine, 0x3f3c, index+1, 0xa9f0))
        first += count
    code0 = struct.pack('>IIII', 32+len(table), 256, len(table), 32) + table
    shell, counts = _resource_fork([
        (0x50414421, [_item(0, bytes(65001))]),
        (0x434f4445, [_item(0, code0), *resources]),
    ])
    system, _ = _resource_fork([(0x4f574e21, [_item(1, bytes((1,2,3,4)))])])
    available = [3,8,4,9,5]+list(range(10,390))
    allocation_count = (len(shell)+1023)//1024
    chain = available[:allocation_count-1]+[7]
    if len(set(chain)) != len(chain) or len(shell) > len(chain)*1024:
        raise ValueError('original allocation shape')
    disk = bytearray(409600)
    struct.pack_into('>H4sHH',disk,0,0x4c4b,b'\xff\x80\x00\x7f',18,0)
    names = [b'Sys\0\x80',b'Shell\0\xff',b'DbgA',b'DbgB',b'Screen',b'Startup',b'Scrap']
    for index,name in enumerate(names):
        disk[10+16*index]=len(name);disk[11+16*index:11+16*index+len(name)]=name
    struct.pack_into('>HHIII',disk,122,10,20,17152,32768,49152)
    header=1024
    for offset,value in [(0,0xd2d7),(12,2),(14,4),(16,4),(18,392),(28,16)]:
        struct.pack_into('>H',disk,header+offset,value)
    for offset,value in [(2,1),(20,1024),(24,1024),(30,12)]:
        struct.pack_into('>I',disk,header+offset,value)
    disk[header+36]=7;disk[header+37:header+44]=b'OwnCODE'
    links=[0]*392
    for ordinal,(name,fork,blocks,at) in enumerate([(names[0],system,[2,6],2048),(names[1],shell,chain,2104)]):
        for index,block in enumerate(blocks):
            if links[block-2]:raise ValueError('duplicate original allocation')
            links[block-2]=blocks[index+1] if index+1<len(blocks) else 1
            start=(16+(block-2)*2)*512;part=fork[index*1024:(index+1)*1024]
            disk[start:start+len(part)]=part
        disk[at]=128;struct.pack_into('>I',disk,at+18,10+ordinal)
        struct.pack_into('>HII',disk,at+32,blocks[0],len(fork),len(blocks)*1024)
        disk[at+50]=len(name);disk[at+51:at+51+len(name)]=name
    struct.pack_into('>H',disk,header+34,links.count(0))
    for index in range(0,392,2):
        a,b=links[index:index+2];at=header+64+index*3//2
        disk[at:at+3]=bytes((a>>4,((a&15)<<4)|(b>>8),b&255))
    code0_at=256+4+65001+4
    # The second512-byte table batch starts257 bytes into its allocation.
    # Its FIRST256-byte callback crosses into the following physical sector;
    # transport reports that callback's start, not the failing sector boundary.
    fault_position=code0_at+16+64*8
    fault_sector_position=fault_position+255
    allocation,within=divmod(fault_sector_position,1024)
    fault_lba=16+(chain[allocation]-2)*2+within//512
    pc_lba=60+fault_lba;cylinder,rem=divmod(pc_lba,30);head,sector=divmod(rem,15)
    # This default fault lies in the first Mac12-sector zone. Variant host
    # fixtures do not reuse these native fault coordinates.
    track,mac_sector=divmod(fault_lba,12)
    logical=bytes(disk)
    expected=[0x43535431,2,0,0,0,rows,segments,32+rows*8,256,rows*8,32,counts['warnings']<<8,1,0,0xb8,1]
    fault_expected=[0x43535431,2,4,5,fault_position,64,8,0,0,0,0,0,0,1,0xd2,1]
    return logical, {'expected':expected,'fault_expected':fault_expected,
        'rows':rows,'segments':segments,'above_a5':32+rows*8,'below_a5':256,
        'table_length':rows*8,'table_offset':32,'code0_file_offset':code0_at,
        'segment1_file_offset':code0_at+len(code0)+4,'shell_chain':chain,
        'shell_length':len(shell),'resource_counts':counts,'fault_lba':fault_lba,
        'fault_position':fault_position,'fault_sector_position':fault_sector_position,
        'pc_fault':[cylinder,head,sector+1],
        'mac_fault':[track,mac_sector],'logical_sha256':hashlib.sha256(logical).hexdigest()}


if __name__=='__main__':
    import json
    print(json.dumps(original_media()[1],indent=2))
