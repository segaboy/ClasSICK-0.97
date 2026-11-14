#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Compile and exercise shared pointer/window semantics as 286 guest code."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import re
import sys

ROOT=Path(__file__).resolve().parents[1]
SOURCES=(ROOT/'core/input/pointer.c',ROOT/'core/ui/window.c',ROOT/'probes/pc286/boot-window-core.c')

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--tool-root',required=True,type=Path);p.add_argument('--emulator',required=True,type=Path)
    p.add_argument('--output',required=True,type=Path);a=p.parse_args()
    try:
        spec=importlib.util.spec_from_file_location('helpers',ROOT/'scripts/Test-BootHushPC286BankPlan.py')
        h=importlib.util.module_from_spec(spec);spec.loader.exec_module(h)
        tool=a.tool_root.resolve(strict=True);exe=a.emulator.resolve(strict=True);out=a.output.resolve()
        if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside the repository')
        lock=json.loads((ROOT/'scripts/pc286-toolchain-lock.json').read_text(encoding='utf-8'))
        for row in lock['selected_members']:
            item=tool/row['path']
            if item.stat().st_size!=row['bytes'] or h.digest(item)!=row['sha256']:
                raise ValueError('pinned compiler identity mismatch: '+row['path'])
        out.mkdir(parents=True,exist_ok=False)
        env=os.environ.copy()
        for key in ('WCC','WATCOM','INCLUDE','LIB','WCL','WLINK'):env.pop(key,None)
        wcc,wlink=tool/'binnt64/wcc.exe',tool/'binnt64/wlink.exe'
        flags=['-zq','-2','-ms','-zl','-s','-os','-zastd=c99','-dCS_C16_REQUIRE_16BIT','-i='+str(tool/'h')]
        objects=[]
        for source in SOURCES[:2]:
            obj=out/(source.stem+'.obj')
            h.run([wcc,*flags,'-fo='+str(obj),source],out,env,out/(source.stem+'-compile.log'))
            objects.append(obj)
        fixed=SOURCES[1].read_text(encoding='utf-8')
        old='(e->buttons & ~(uint32_t)7u)!=0u || (f & ~(uint32_t)255u)!=0u'
        if fixed.count(old)!=1:raise ValueError('window mask control no longer matches source')
        broken=fixed.replace(old,'(e->buttons&~7u)!=0u || (f&~255u)!=0u')
        broken_src=out/'window-old-mask.c';broken_src.write_text(broken,encoding='utf-8')
        broken_obj=out/'window-old-mask.obj'
        h.run([wcc,*flags,'-i='+str(ROOT/'core/ui'),'-fo='+str(broken_obj),broken_src],
              out,env,out/'window-old-mask-compile.log')
        cases=[]
        for name,mutant,marker in (('baseline',False,0xa9),('wrong-expectation',True,0xdb),('old-mask',False,0xd9)):
            obj=out/(name+'.obj');binary=out/(name+'.bin');mapfile=out/(name+'.map')
            h.run([wcc,*flags,*(['-dCS_P286_BAD_EXPECTATION'] if mutant else []),'-fo='+str(obj),SOURCES[2]],
                  out,env,out/(name+'-compile.log'))
            linked=[objects[0],broken_obj if name=='old-mask' else objects[1],obj]
            h.run([wlink,'format','raw','bin','option','start=start_','option','nodefaultlibs',
                   'option','map='+str(mapfile),'name',binary,'file',','.join(str(x) for x in linked)],
                  out,env,out/(name+'-link.log'))
            match=re.search(r'Entry point address:\s*([0-9a-fA-F]+)',mapfile.read_text(encoding='utf-8',errors='replace'))
            if not match:raise ValueError('missing linked entry address')
            payload=binary.read_bytes();entry=int(match.group(1),16)
            image=out/(name+'.img')
            with image.open('xb') as f:f.write(h.floppy(payload,entry))
            before=h.digest(image)
            result=json.loads(h.run([exe,'--machine','pc286','--floppy',image,'--cycles','8000000',
                                      '--timeout-ms','20000','--until-post',hex(marker),
                                      '--inspect','0x60000:12'],out,env,out/(name+'-boot.log')))
            expected=bytes.fromhex('57323836702e0100966e005a') if name=='baseline' else bytes(12)
            if (result.get('status')!='condition_met' or result.get('post')!=marker or
                bytes.fromhex(result['memory']['bytes'])!=expected or h.digest(image)!=before):
                raise ValueError(name+': unexpected guest state or changed input')
            cases.append({'name':name,'result':result,'payload_sha256':h.digest(binary),
                          'payload_bytes':len(payload),'floppy_sha256':before,'floppy_unchanged':True})
        report={'status':'pass','compiler':lock['release'],'boothush_sha256':h.digest(exe),
                'source_sha256':{str(s.relative_to(ROOT)).replace('\\','/'):h.digest(s) for s in SOURCES},
                'cases':cases,'limits':'Near-data semantic fixture; no native hardware input, renderer, OS boot or physical qualification.'}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
        print(json.dumps(report,indent=2));return 0
    except (OSError,ValueError,KeyError) as exc:
        print('286 window-core test failed: '+str(exc),file=sys.stderr);return 1

if __name__=='__main__':sys.exit(main())
