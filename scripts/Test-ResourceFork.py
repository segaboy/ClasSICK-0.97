# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Original declarative fixture and independent integer oracle; hosted tests only."""
import argparse, struct, subprocess
from pathlib import Path

def fixture():
    groups=[(0x72303121,[(-32768,b'ab',b'one!'),(32767,b'c',b'two!')]),
            (0x0000ff80,[(0,b'd',b'thr!'),(1,b'e',b'for!')]),
            (0x72503221,[(-1,b'f',b'five'),(128,b'gh',b'six!')])]
    payload=bytearray(); names=bytearray(); references=[]
    for kind,items in groups:
        encoded=bytearray()
        for identifier,name,data in items:
            encoded+=struct.pack('>HHB',identifier&65535,len(names),0)
            encoded+=len(payload).to_bytes(3,'big')+bytes(4)
            payload+=struct.pack('>I',len(data))+data
            names+=bytes([len(name)])+name
        references.append(encoded)
    table=bytearray(struct.pack('>H',len(groups)-1));cursor=2+8*len(groups)
    for (kind,items),refs in zip(groups,references):
        table+=struct.pack('>IHH',kind,len(items)-1,cursor);cursor+=len(refs)
    mapping=bytearray(24)+struct.pack('>HH',28,28+cursor)+table+b''.join(references)+names
    header=struct.pack('>IIII',256,256+len(payload),len(payload),len(mapping))
    result=bytearray(header+bytes(240)+payload+mapping)
    assert len(result)==444
    return result

def model(blob):
    # Python's integers do not wrap. A range is represented by mathematical
    # endpoints; this model does not import or invoke parser/encoder helpers.
    warnings=0;types_seen=0;refs_seen=0
    def reply(status,field=0,position=0):return [status,field,position,warnings,types_seen,refs_seen]
    def number(position,width):return int.from_bytes(blob[position:position+width],'big')
    def bounded(left,count,limit,field,position,status):
        right=left+count
        if right>0xffffffff:return reply(7,field,position)
        if right>limit:return reply(status,field,position)
        return right
    def intersects(a,b):return max(a[0],b[0])<min(a[1],b[1])
    size=len(blob)
    if size<16:return reply(4,1,0),None
    data_start,map_start,data_size,map_size=struct.unpack('>IIII',blob[:16])
    data_end=bounded(data_start,data_size,size,2,8,4)
    if isinstance(data_end,list):return data_end,None
    map_end=bounded(map_start,map_size,size,3,12,4)
    if isinstance(map_end,list):return map_end,None
    if intersects((0,16),(data_start,data_end)):return reply(6,2,0),None
    if intersects((0,16),(map_start,map_end)) or intersects((data_start,data_end),(map_start,map_end)):return reply(6,3,4),None
    if map_size<30:return reply(5,3,12),None
    type_offset=number(map_start+24,2);name_offset=number(map_start+26,2)
    if type_offset<28:return reply(6,4,map_start+24),None
    limit=bounded(type_offset,2,map_size,4,map_start+24,5)
    if isinstance(limit,list):return limit,None
    if name_offset<28:return reply(6,5,map_start+26),None
    if name_offset>map_size:return reply(5,5,map_start+26),None
    table_start=map_start+type_offset;name_start=map_start+name_offset
    stored=number(table_start,2);type_count=0 if stored==65535 else stored+1
    table_end=bounded(limit,8*type_count,map_size,4,table_start,5)
    if isinstance(table_end,list):return table_end,None
    if table_end>name_offset:return reply(6,4,table_start),None
    table_end+=map_start
    if data_start!=256 or map_start!=data_end or map_end!=size or type_offset!=28:warnings|=1
    if any(blob[map_start:map_start+22]):warnings|=2
    previous=table_end;groups=[]
    for index in range(type_count):
        at=table_start+2+8*index;types_seen+=1
        kind=number(at,4);stored=number(at+4,2);amount=0 if stored==65535 else stored+1
        start=bounded(table_start,number(at+6,2),map_end,6,at+6,5)
        if isinstance(start,list):return start,None
        finish=bounded(start,12*amount,map_end,6,at+4,5)
        if isinstance(finish,list):return finish,None
        if start<previous:return reply(6,6,at+6),None
        if finish>name_start:return reply(6,6,at+4),None
        if start!=previous:warnings|=1
        if amount==0:warnings|=32
        previous=finish;groups.append((at,kind,start,amount))
    if previous!=name_start:warnings|=1
    resources=[]
    for type_position,kind,start,amount in groups:
        records=[]
        for index in range(amount):
            at=start+12*index;refs_seen+=1
            identifier=struct.unpack('>h',blob[at:at+2])[0];name=number(at+2,2);attributes=blob[at+4]
            name_position=name_length=data_position=data_length=specification=0
            if name!=65535:
                prefix=bounded(name_start,name,map_end,7,at+2,8)
                if isinstance(prefix,list):return prefix,None
                if prefix==map_end:return reply(8,7,at+2),None
                name_position=prefix+1;name_length=blob[prefix]
                checked=bounded(name_position,name_length,map_end,7,at+2,8)
                if isinstance(checked,list):return checked,None
            if attributes&3:warnings|=8
            if attributes&128:
                specification=number(at+8,4);warnings|=16
            else:
                if number(at+8,4):warnings|=4
                prefix=bounded(data_start,number(at+5,3),data_end,8,at+5,5)
                if isinstance(prefix,list):return prefix,None
                data_position=bounded(prefix,4,data_end,8,at+5,5)
                if isinstance(data_position,list):return data_position,None
                data_length=number(prefix,4)
                checked=bounded(data_position,data_length,data_end,8,prefix,5)
                if isinstance(checked,list):return checked,None
                if data_length>32768:warnings|=64
            records.append([kind,identifier&0xffffffff,at,attributes,int(name!=65535),specification,name_position,name_length,data_position,data_length])
        resources.append((type_position,kind,records))
    opening=reply(0)
    # The lookup oracle uses the constructed mathematical records, not another
    # bytewise traversal copied from the reader's lookup implementation.
    matches=[g for g in resources if g[1]==0x72303121]
    base=[0,0,0,warnings,type_count,0]
    if not matches:base[:3]=[2,9,0];return opening,base+[0]*10
    if len(matches)>1:base[:3]=[9,9,matches[1][0]];return opening,base+[0]*10
    chosen=matches[0];base[5]=len(chosen[2])
    found=[e for e in chosen[2] if e[1]==0xffff8000]
    if not found:base[:3]=[2,10,chosen[0]];return opening,base+[0]*10
    if len(found)>1:base[:3]=[10,10,found[1][2]];return opening,base+[0]*10
    entry=found[0];base[:3]=[1 if entry[3]&128 else 0,0,entry[2]]
    return opening,base+entry

def main():
    p=argparse.ArgumentParser();p.add_argument('--executable',required=True);p.add_argument('--output',required=True);a=p.parse_args()
    out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=True)
    # Keep generated original evidence in the caller's build directory; no
    # recursive cleanup or writes to project research/reference locations.
    raw=out/'original-444.bin';oracle=out/'integer-model.bin';original=fixture();raw.write_bytes(original)
    with oracle.open('wb') as file:
        blob=bytearray(original)
        for position in range(444):
            for value in range(256):
                blob[position]=value;opening,lookup=model(blob)
                words=opening+([0]*16 if lookup is None else lookup)
                assert len(words)==22
                file.write(struct.pack('>22I',*words))
            blob[position]=original[position]
    subprocess.run([a.executable,'oracle',str(raw),str(oracle)],check=True)
    print('Resource reader PASS:113664 cases, delivered independent integer model and declarative generator.')
if __name__=='__main__':main()
