#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Original native286 one-sector writes, explicit snapshots and fresh boots."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import subprocess

ROOT=Path(__file__).resolve().parents[1]
WRITE=ROOT/'platform/pc286/boothush-floppy-write.c'
READ=ROOT/'platform/pc286/boothush-floppy-read.c'
PROBE=ROOT/'probes/pc286/boot-floppy-write.c'
TARGETS=(60,74,75,89,90,2399)

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def original(sector):return bytes((sector^(i*13)^(i>>1)^0x5a)&255 for i in range(512))
def written(sector):return bytes(((sector*3)^(i*29)^(i>>3)^0xc7)&255 for i in range(512))

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('tool-root','emulator','model-contract','output'):parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args()
    tool,emulator,contract=(path.resolve(strict=True) for path in (args.tool_root,args.emulator,args.model_contract))
    out=args.output.resolve()
    if out==ROOT or ROOT in out.parents:raise ValueError('output must be a new external directory')
    lock_path=ROOT/'scripts/pc286-toolchain-lock.json';lock=json.loads(lock_path.read_text(encoding='utf-8'))
    tool_paths=[emulator,contract,lock_path]
    for item in lock['selected_members']:
        path=tool/item['path']
        if path.stat().st_size!=item['bytes'] or sha(path)!=item['sha256']:raise ValueError('compiler identity mismatch: '+item['path'])
        tool_paths.append(path)
    sources=[WRITE,WRITE.with_suffix('.h'),READ,READ.with_suffix('.h'),PROBE,ROOT/'core/storage/block-read.h',
             ROOT/'arch/i286/c16.h',ROOT/'scripts/pc286_loader.py',Path(__file__),
             ROOT/'docs/specifications/SPEC-0126-pc286-floppy-sector-write.md',
             ROOT/'provenance/records/SRC-0620-pc286-floppy-sector-write.md']
    before_sources={path.relative_to(ROOT).as_posix():sha(path) for path in sources}
    before_tools={str(path):sha(path) for path in tool_paths}
    spec=importlib.util.spec_from_file_location('writer_loader',ROOT/'scripts/pc286_loader.py')
    loader=importlib.util.module_from_spec(spec);spec.loader.exec_module(loader)
    out.mkdir(parents=True,exist_ok=False)
    env=os.environ.copy()
    for name in ('WCC','WATCOM','INCLUDE','LIB','WCL','WLINK'):env.pop(name,None)
    commands=[]
    def run(command,label):
        command=[str(value) for value in command];commands.append(command)
        result=subprocess.run(command,cwd=out,env=env,capture_output=True,timeout=35,
                              creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        (out/(label+'.log')).write_bytes(result.stdout+b'\n'+result.stderr)
        if result.returncode:raise ValueError(label+' failed; see its log')
        return result.stdout
    wcc,wlink,dmpobj=(tool/'binnt64'/name for name in ('wcc.exe','wlink.exe','dmpobj.exe'))
    flags=['-zq','-2','-ms','-zl','-s','-zastd=c99','-we','-dCS_C16_REQUIRE_16BIT',
           '-i='+str(tool/'h'),'-i='+str(WRITE.parent)]
    provider=WRITE.read_text(encoding='utf-8')
    mutations={
        'wrong-head':('    head=(uint16_t)((physical_sector/15u)%2u);',
                      '    head=(uint16_t)(((physical_sector/15u)%2u)^1u);'),
        'truncated-copy':('    for(offset=0;offset<CS_BLOCK_BYTES;++offset)writer->bounce[offset]=source[offset];',
                          '    for(offset=0;offset<CS_BLOCK_BYTES-1u;++offset)writer->bounce[offset]=source[offset];'),
        'ignored-protection':('    if(status==0x0301u)return CS_PC286_WRITE_PROTECTED;',
                              '    if(status==0x0301u)return CS_PC286_WRITE_OK;')}
    cases=[('os',WRITE,'-os',False,0xbb),('od',WRITE,'-od',False,0xbb),('readonly',WRITE,'-os',True,0xbd)]
    for name,(old,new) in mutations.items():
        if provider.count(old)!=1:raise ValueError('mutation anchor changed: '+name)
        path=out/(name+'.c');path.write_text(provider.replace(old,new),encoding='utf-8')
        cases.append((name,path,'-os',name=='ignored-protection',0xe3 if name=='truncated-copy' else 0xe4 if name=='ignored-protection' else 0xe2))
    observed=[];objects=[]
    def boot(label,image,payload,marker,snapshot):
        before=sha(image);inspection=[]
        for offset in range(0,len(payload),4096):inspection+=['--inspect',hex(65536+offset)+':'+str(min(4096,len(payload)-offset))]
        command=[emulator,'--machine','pc286','--floppy',image,'--cycles','8000000','--timeout-ms','20000',
                 '--until-post',hex(marker),*inspection]
        if snapshot is not None:command+=['--floppy-out',snapshot]
        response=json.loads(run(command,label+'-boot'))
        ranges=response.pop('memory_ranges',None) or [response.pop('memory',{})]
        memory=b''.join(bytes.fromhex(row.get('bytes','')) for row in ranges)
        if (response.get('status')!='condition_met' or response.get('post')!=marker or response.get('cs')!=0x1000 or
            ranges[0].get('address')!=0x10000 or memory!=payload or sha(image)!=before):raise ValueError(label+': wrong result, loaded payload or changed input')
        return {'case':label,'marker':marker,'result':response,'input_sha256':before,
                'input_unchanged':True,'complete_payload_matches':True,'payload_bytes':len(payload)}
    for name,path,opt,readonly,marker in cases:
        linked=[]
        for tag,source in [('read',READ),('write',path),('probe',PROBE)]:
            obj=out/(name+'-'+tag+'.obj');linked.append(obj)
            run([wcc,*flags,opt,*(['-dCS_FLOPPY_WRITE_READONLY'] if readonly and tag=='probe' else []),
                 '-fo='+str(obj),source],name+'-'+tag+'-compile')
            metadata=run([dmpobj,obj],name+'-'+tag+'-object').decode('utf-8',errors='replace')
            if '80286' not in metadata or 'USE16' not in metadata:raise ValueError('not native80286 USE16')
            references=set(re.findall(r"(?m)^\s*[0-9]+ - '([^']+)' Type:",metadata))
            definitions=set(re.findall(r"(?m)^\s*[0-9a-fA-F]+h - '([^']+)' Type:",metadata))
            imports=references-definitions
            allowed=set() if tag=='read' else {'cs_pc286_floppy_buffer_fits_'} if tag=='write' else {
                'cs_pc286_floppy_buffer_fits_','cs_pc286_floppy_bind_','cs_pc286_floppy_read_one_',
                'cs_pc286_floppy_write_bind_','cs_pc286_floppy_write_one_'}
            if not imports<=allowed or re.search(r'\b(?:COMDEF|LCOMDEF)\(',metadata):raise ValueError('unexpected runtime dependency: '+repr(imports))
            lengths=[int(value,16) for value in re.findall(r'USE16 Length ([0-9a-fA-F]+)h',metadata)]
            objects.append({'case':name,'module':tag,'sha256':sha(obj),'text_bytes':lengths[0],
                            'other_segment_bytes':sum(lengths[1:]),'imports':sorted(imports)})
        binary,mapping=out/(name+'.bin'),out/(name+'.map')
        run([wlink,'format','raw','bin','option','start=start_','option','nodefaultlibs','option','map='+str(mapping),
             'name',binary,'file',','.join(str(obj) for obj in linked)],name+'-link')
        text=mapping.read_text(encoding='utf-8',errors='replace')
        entry=re.search(r'Entry point address:\s*([0-9a-fA-F]+)',text)
        bss=re.search(r'^_BSS\s+BSS\s+DGROUP\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)',text,re.M)
        if not entry or not bss or '.lib' in text.lower():raise ValueError('missing entry/BSS or unwanted runtime')
        payload=binary.read_bytes();bss_start,bss_bytes=(int(value,16) for value in bss.groups())
        if len(payload)>59*512 or bss_start<len(payload):raise ValueError('payload overlaps fixture sectors or BSS')
        media=bytearray(loader.floppy(payload,int(entry.group(1),16),bss_end=bss_start+bss_bytes,
                                     poison_bss=True,verify_payload=True))
        for sector in range(60,2400):media[sector*512:(sector+1)*512]=original(sector)
        image=out/(name+'.img');image.write_bytes(media)
        expected=bytearray(media)
        if name in ('os','od'):
            for sector in TARGETS:expected[sector*512:(sector+1)*512]=written(sector)
        elif name=='wrong-head':expected[75*512:76*512]=written(60)
        elif name=='truncated-copy':expected[60*512:61*512]=written(60)[:511]+b'\x6c'
        for run_number in range(3 if name=='os' else 1):
            label=name+'-'+str(run_number+1)
            snapshot=None if readonly else out/(label+'-written.img')
            result=boot(label,image,payload,marker,snapshot)
            result.update(payload_sha256=sha(binary),bss_bytes=bss_bytes)
            if snapshot is not None:
                if snapshot.read_bytes()!=expected:raise ValueError(label+': snapshot differs outside exact committed bytes')
                result.update(snapshot_sha256=sha(snapshot),whole_snapshot_matches=True)
            observed.append(result)
            if name in ('os','od'):
                fresh=boot(label+'-fresh',snapshot,payload,0xbc,None)
                if snapshot.read_bytes()!=expected:raise ValueError('fresh readonly boot modified snapshot')
                fresh.update(whole_snapshot_matches=True);observed.append(fresh)
    if {path.relative_to(ROOT).as_posix():sha(path) for path in sources}!=before_sources:raise ValueError('declared source changed during verification')
    if {str(path):sha(path) for path in tool_paths}!=before_tools:raise ValueError('declared tool/contract changed during verification')
    report={'status':'pass','source_sha256':before_sources,'tool_sha256':before_tools,'commands':commands,
            'objects':objects,'observed':observed,'inputs_unchanged':True,
            'limits':'Original generated PC floppy only; exactly one sector per provider call. Multiple successful calls are independently committed, not an atomic group. Explicit new private snapshots; no original-media mutation, MFS write, physical BIOS/FDC or System boot claim.'}
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({'status':'pass','native_boots':len(observed),'report':str(out/'report.json')},indent=2))

if __name__=='__main__':main()
