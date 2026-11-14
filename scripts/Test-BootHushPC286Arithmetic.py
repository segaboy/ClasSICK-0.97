#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Compare independently authored C16 ABI arithmetic to host integers."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import random
import re
import sys

ROOT=Path(__file__).resolve().parents[1]
ASM_SHA='9e779dda024c6cee24943f917b0f761ce862951caec0a4c0fbadbc760895c0cb'
SOURCES=(ROOT/'arch/i286/watcom-arithmetic.asm',ROOT/'probes/pc286/boot-arithmetic.c')

def values():
    edges=[0,1,2,3,7,65535,65536,0x7fffffff,0x80000000,0x80000001,0xfffffffe,0xffffffff]
    rng=random.Random(0x286097)
    pairs=[(a,b) for a in edges for b in edges if b]
    pairs += [(rng.getrandbits(32),rng.getrandbits(32) or 1) for _ in range(128)]
    pairs=[(a,b) for a,b in pairs if not (a==0x80000000 and b==0xffffffff)]
    def signed(n):return n if n<0x80000000 else n-0x100000000
    rows=[]
    for a,b in pairs:
        x,y=signed(a),signed(b)
        q=abs(x)//abs(y)*(-1 if (x<0)!=(y<0) else 1)
        rows.append((a,b,(a*b)&0xffffffff,a//b,a%b,q&0xffffffff,(x-q*y)&0xffffffff))
    return rows

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--tool-root',required=True,type=Path);p.add_argument('--assembler',required=True,type=Path)
    p.add_argument('--emulator',required=True,type=Path);p.add_argument('--output',required=True,type=Path);a=p.parse_args()
    try:
        spec=importlib.util.spec_from_file_location('helpers',ROOT/'scripts/Test-BootHushPC286BankPlan.py')
        h=importlib.util.module_from_spec(spec);spec.loader.exec_module(h)
        tool=a.tool_root.resolve(strict=True);assembler=a.assembler.resolve(strict=True)
        exe=a.emulator.resolve(strict=True);out=a.output.resolve()
        if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside the repository')
        if h.digest(assembler)!=ASM_SHA:raise ValueError('assembler identity mismatch')
        lock=json.loads((ROOT/'scripts/pc286-toolchain-lock.json').read_text(encoding='utf-8'))
        for row in lock['selected_members']:
            item=tool/row['path']
            if item.stat().st_size!=row['bytes'] or h.digest(item)!=row['sha256']:raise ValueError('compiler identity mismatch')
        out.mkdir(parents=True,exist_ok=False);env=os.environ.copy()
        for key in ('WCC','WATCOM','INCLUDE','LIB','WCL','WLINK','WASM'):env.pop(key,None)
        rows=values();header=out/'arithmetic-cases.h'
        header.write_text('struct arithmetic_case {uint32_t a,b,product,uq,ur,sq,sr;};\n'
            +'#define ARITH_COUNT '+str(len(rows))+'u\nstatic const struct arithmetic_case arith[ARITH_COUNT]={\n'
            +''.join('{'+','.join('0x%08xUL'%v for v in row)+'},\n' for row in rows)+'};\n',encoding='ascii')
        wcc,wlink=tool/'binnt64/wcc.exe',tool/'binnt64/wlink.exe';arith=out/'arithmetic.obj'
        h.run([assembler,'-zq','-2','-ms','-fo='+str(arith),SOURCES[0]],out,env,out/'assemble.log')
        flags=['-zq','-2','-ms','-zl','-s','-os','-zastd=c99','-dCS_C16_REQUIRE_16BIT',
               '-i='+str(tool/'h'),'-i='+str(out)]
        cases=[]
        for name,bad,marker in (('oracle',False,0xad),('negative-control',True,0xcd)):
            obj=out/(name+'.obj');binary=out/(name+'.bin');mapping=out/(name+'.map')
            h.run([wcc,*flags,*(['-dCS_BAD_EXPECTATION'] if bad else []),'-fo='+str(obj),SOURCES[1]],out,env,out/(name+'-compile.log'))
            h.run([wlink,'format','raw','bin','option','start=start_','option','nodefaultlibs',
                   'option','map='+str(mapping),'name',binary,'file',str(obj)+','+str(arith)],out,env,out/(name+'-link.log'))
            text=mapping.read_text(encoding='utf-8',errors='replace')
            entry=re.search(r'Entry point address:\s*([0-9a-fA-F]+)',text)
            if not entry or 'clibs.lib' in text.lower():raise ValueError('unexpected entry or runtime library')
            image=out/(name+'.img')
            with image.open('xb') as f:f.write(h.floppy(binary.read_bytes(),int(entry.group(1),16)))
            before=h.digest(image)
            result=json.loads(h.run([exe,'--machine','pc286','--floppy',image,'--cycles','100000000','--timeout-ms','20000',
                                     '--until-post',hex(marker)],out,env,out/(name+'-boot.log')))
            if result.get('status')!='condition_met' or result.get('post')!=marker or h.digest(image)!=before:
                raise ValueError(name+': failed or input changed')
            cases.append({'name':name,'result':result,'payload_sha256':h.digest(binary),'floppy_sha256':before})
        report={'status':'pass','compiler':lock['release'],'assembler_sha256':h.digest(assembler),'boothush_sha256':h.digest(exe),
                'source_sha256':{str(s.relative_to(ROOT)).replace('\\','/'):h.digest(s) for s in SOURCES},
                'oracle_cases':len(rows),'oracle_header_sha256':hashlib.sha256(header.read_bytes()).hexdigest(),'cases':cases,
                'limits':'Experimental compiler ABI, synthetic modeled 286; no physical or release claim.'}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(report,indent=2));return 0
    except (OSError,ValueError,KeyError) as exc:
        print('286 arithmetic failed: '+str(exc),file=sys.stderr);return 1

if __name__=='__main__':sys.exit(main())
