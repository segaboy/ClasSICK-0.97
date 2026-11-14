#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Exercise actual shared surface validation at C16 integer boundaries."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import re
import sys

ROOT=Path(__file__).resolve().parents[1]
SOURCES=(ROOT/'core/graphics/surface.c',ROOT/'probes/pc286/boot-surface-validate.c')

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--tool-root',required=True,type=Path)
    runtime=p.add_mutually_exclusive_group(required=True)
    runtime.add_argument('--runtime-library',type=Path);runtime.add_argument('--assembler',type=Path)
    p.add_argument('--emulator',required=True,type=Path);p.add_argument('--output',required=True,type=Path);a=p.parse_args()
    try:
        def module(name,path):
            spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
        h=module('helpers',ROOT/'scripts/Test-BootHushPC286BankPlan.py')
        common=module('compositor',ROOT/'scripts/Test-BootHushPC286SharedCompositor.py')
        tool=a.tool_root.resolve(strict=True)
        lib=a.runtime_library.resolve(strict=True) if a.runtime_library else None
        assembler=a.assembler.resolve(strict=True) if a.assembler else None
        exe=a.emulator.resolve(strict=True);out=a.output.resolve()
        if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside the repository')
        if lib and h.digest(lib)!=common.LIB_SHA:raise ValueError('runtime library hash mismatch')
        if assembler and h.digest(assembler)!=module('arith',ROOT/'scripts/Test-BootHushPC286Arithmetic.py').ASM_SHA:
            raise ValueError('assembler identity mismatch')
        lock=json.loads((ROOT/'scripts/pc286-toolchain-lock.json').read_text(encoding='utf-8'))
        for row in lock['selected_members']:
            item=tool/row['path']
            if item.stat().st_size!=row['bytes'] or h.digest(item)!=row['sha256']:raise ValueError('compiler identity mismatch')
        out.mkdir(parents=True,exist_ok=False);env=os.environ.copy()
        for key in ('WCC','WATCOM','INCLUDE','LIB','WCL','WLINK'):env.pop(key,None)
        wcc,wlink=tool/'binnt64/wcc.exe',tool/'binnt64/wlink.exe'
        flags=['-zq','-2','-ms','-zl','-s','-os','-zastd=c99','-dCS_C16_REQUIRE_16BIT','-i='+str(tool/'h')]
        surface=out/'surface.obj';h.run([wcc,*flags,'-fo='+str(surface),SOURCES[0]],out,env,out/'surface.log')
        original=out/'original-arithmetic.obj'
        if assembler:
            h.run([assembler,'-zq','-2','-ms','-fo='+str(original),ROOT/'arch/i286/watcom-arithmetic.asm'],
                  out,env,out/'arithmetic-assemble.log')
        cases=[]
        for name,bad,marker in (('boundary',False,0xaf),('negative-control',True,0xcd)):
            obj=out/(name+'.obj');binary=out/(name+'.bin');mapping=out/(name+'.map')
            h.run([wcc,*flags,*(['-dCS_BAD_EXPECTATION'] if bad else []),'-fo='+str(obj),SOURCES[1]],out,env,out/(name+'-compile.log'))
            h.run([wlink,'format','raw','bin','option','start=start_','option','nodefaultlibs',
                   'option','map='+str(mapping),'name',binary,'file',
                   ','.join(str(x) for x in [surface,obj]+([original] if assembler else [])),
                   *(['library',lib] if lib else [])],out,env,out/(name+'-link.log'))
            text=mapping.read_text(encoding='utf-8',errors='replace')
            entry=re.search(r'Entry point address:\s*([0-9a-fA-F]+)',text)
            if not entry or re.search(r'clibs\.lib\(i8',text,re.I) or (assembler and 'clibs.lib' in text.lower()):
                raise ValueError('unexpected entry or runtime dependency')
            image=out/(name+'.img')
            with image.open('xb') as f:f.write(h.floppy(binary.read_bytes(),int(entry.group(1),16)))
            before=h.digest(image)
            result=json.loads(h.run([exe,'--machine','pc286','--floppy',image,'--cycles','8000000','--timeout-ms','20000',
                                     '--until-post',hex(marker)],out,env,out/(name+'-boot.log')))
            if result.get('status')!='condition_met' or result.get('post')!=marker or h.digest(image)!=before:
                raise ValueError(name+': failed or input changed')
            cases.append({'name':name,'result':result,'payload_sha256':h.digest(binary),'floppy_sha256':before})
        report={'status':'pass','compiler':lock['release'],'boothush_sha256':h.digest(exe),
                'runtime_sha256':h.digest(lib) if lib else None,'assembler_sha256':h.digest(assembler) if assembler else None,
                'source_sha256':{str(s.relative_to(ROOT)).replace('\\','/'):h.digest(s) for s in
                                 (*SOURCES,*((ROOT/'arch/i286/watcom-arithmetic.asm',) if assembler else ()))},'cases':cases,
                'limits':'Synthetic BootHush 286; no physical or release claim.'}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(report,indent=2));return 0
    except (OSError,ValueError,KeyError) as exc:
        print('286 surface validation failed: '+str(exc),file=sys.stderr);return 1

if __name__=='__main__':sys.exit(main())
