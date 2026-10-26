#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Delta-only WCC C16 facade check. Own unlinked objects; no target execution."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--tool-root',required=True,type=Path)
    parser.add_argument('--output',required=True,type=Path)
    parser.add_argument('--polled-phase',action='store_true',help='Compile only the isolated phase model;no segmented backend/run claim')
    args=parser.parse_args()
    root=Path(__file__).resolve().parents[1]
    source=root/('probes/pc286/polled-phase.c' if args.polled_phase else 'probes/pc286/c16.c')
    lock=json.loads((root/'scripts/pc286-toolchain-lock.json').read_text())
    for row in lock['selected_members']:
        p=args.tool_root/row['path']
        if len(p.read_bytes())!=row['bytes'] or hashlib.sha256(p.read_bytes()).hexdigest()!=row['sha256']:
            raise ValueError('Pinned tool/header identity mismatch: '+row['path'])
    out=args.output.resolve();out.mkdir(parents=True,exist_ok=False)
    env=os.environ.copy()
    for key in ('WCC','WATCOM','INCLUDE','LIB','WCL','WLINK'):env.pop(key,None)
    tools=args.tool_root/'binnt64';rows=[]
    for model in ('ms','mm','ml'):
        for opt,bad in (('od',False),('os',False),('os',True)):
            name=model+'-'+opt+('-false-assert' if bad else '')
            obj=out/(name+'.obj')
            flags=['-zq','-2','-'+model,'-zl','-s','-'+opt,'-zastd=c99',
                   '-i='+str(args.tool_root/'h'),'-fo='+str(obj)]
            if args.polled_phase:flags+=['-dCS_C16_REQUIRE_16BIT']
            if bad:flags+=['-dCS_C16_FALSE_ASSERT']
            q=subprocess.run([str(tools/'wcc.exe'),*flags,str(source)],
                cwd=out,env=env,stdin=subprocess.DEVNULL,capture_output=True,timeout=20)
            log=q.stdout+q.stderr;(out/(name+'.log')).write_bytes(log)
            if (q.returncode==0)==bad:
                raise ValueError(name+': unexpected compile result; see private log')
            row={'case':name,'model':model,'flags':flags,'exit_code':q.returncode,
                 'expected_success':not bad,'target_executed':False}
            if bad:
                lines=source.read_text().splitlines()
                line=next(i+1 for i,s in enumerate(lines) if 'cs_probe_expected_refusal' in s)
                diagnostic=rf'{re.escape(source.name)}\({line}\): Error! E1020: Dimension cannot be 0 or negative'
                if not re.search(diagnostic,log.decode(errors='replace')) or obj.exists():
                    raise ValueError(name+': not the named assertion refusal')
            else:
                listing=out/(name+'.lst')
                for command in ([str(tools/'wdis.exe'),'-l='+str(listing),str(obj)],):
                    subprocess.run(command,cwd=out,env=env,stdin=subprocess.DEVNULL,
                        capture_output=True,timeout=20,check=True)
                meta=subprocess.check_output([str(tools/'dmpobj.exe'),str(obj)],cwd=out,
                    env=env,stdin=subprocess.DEVNULL,timeout=20).decode(errors='replace')
                externals=[]
                for block in re.findall(r'EXTDEF\(8c\).*?(?=\n\S+.*recnum:|\Z)',meta,re.S):
                    externals+=re.findall(r"\d+ - '([^']+)' Type:",block)
                if externals:raise ValueError(name+': helper/import not admitted: '+','.join(externals))
                text=listing.read_text(errors='replace')
                if not re.search(r'Processor\s+: 80286',meta):raise ValueError(name+': CPU metadata')
                segments=re.findall(r'Segment: (\S+)\s+\S+ USE(\d+) ([0-9A-F]+) bytes',text)
                if not segments or any(bits!='16' for _,bits,_ in segments):
                    raise ValueError(name+': missing/incorrect segment width')
                row.update(object_bytes=obj.stat().st_size,
                    object_sha256=hashlib.sha256(obj.read_bytes()).hexdigest(),
                    external_symbols=externals,compiler_metadata_80286=True,segments=segments)
            rows.append(row)
    files=['arch/i286/c16.h',str(source.relative_to(root)).replace('\\','/')]
    if args.polled_phase:files+=['platform/mac128k/polled-phase.c','platform/mac128k/polled-phase.h','core/input/pointer.h','scripts/Probe-PC286C16.py']
    inputs={f:hashlib.sha256((root/f).read_bytes().replace(b'\r\n',b'\n')).hexdigest() for f in files}
    report={'specifications':['SPEC-0067','SPEC-0074'] if args.polled_phase else ['SPEC-0067'],'compiler_pin':lock['release'],
            'compiler_calls':9,'objects':6,'expected_refusals':3,'inputs':inputs,
            'records':rows,'target_executed':False,'linked':False,
            'stack_policy':'-s is diagnostic only; no production waiver',
            'limits':'Metadata and helper checks are not a complete ISA or runtime proof.'}
    if args.polled_phase:report['flat_span_premise_is_not_segmented_pointer_qualification']=True
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print('C16 delta PASS: 9 compiler calls, 6 helper-free objects, 3 named refusals; no link/run.')

if __name__=='__main__':
    main()
