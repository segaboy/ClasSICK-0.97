#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Compare the actual shared compositor on hosted and experimental 286 builds.

The caller supplies a pinned Open Watcom runtime library for an isolated test;
its license and inclusion in any release remain subject to separate review.
"""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import sys
from pc286_loader import floppy

ROOT=Path(__file__).resolve().parents[1]
SOURCES=('core/graphics/surface.c','core/graphics/cursor.c','core/input/pointer.c',
         'core/ui/window.c','apps/product-scene/scene.c','apps/window-scene/scene.c')
LIB_SHA='045469f2f05f62f8d190b0e1965abc0a1db774b6ca125672f6a1db341b870e03'

def compare(pbm,ppm,margin_white=False):
    ph=b'P4\n512 342\n';rgb=b'P6\n640 350\n255\n'
    if not pbm.startswith(ph) or len(pbm)!=len(ph)+21888:raise ValueError('unexpected host PBM')
    if not ppm.startswith(rgb) or len(ppm)!=len(rgb)+640*350*3:raise ValueError('unexpected EGA PPM')
    source=pbm[len(ph):];image=ppm[len(rgb):]
    for y in range(350):
        for x in range(640):
            inside=64<=x<576 and 4<=y<346
            white=(not (source[(y-4)*64+(x-64)//8] & (0x80>>((x-64)%8)))) if inside else margin_white
            v=255 if white else 0;offset=(y*640+x)*3
            if image[offset:offset+3]!=bytes((v,v,v)):raise ValueError(f'pixel mismatch at {x},{y}')

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--tool-root',required=True,type=Path);p.add_argument('--runtime-library',required=True,type=Path)
    p.add_argument('--host-cc',required=True,type=Path);p.add_argument('--emulator',required=True,type=Path)
    p.add_argument('--output',required=True,type=Path);a=p.parse_args()
    try:
        spec=importlib.util.spec_from_file_location('helpers',ROOT/'scripts/Test-BootHushPC286BankPlan.py')
        h=importlib.util.module_from_spec(spec);spec.loader.exec_module(h)
        tool=a.tool_root.resolve(strict=True);lib=a.runtime_library.resolve(strict=True)
        cc=a.host_cc.resolve(strict=True);exe=a.emulator.resolve(strict=True);out=a.output.resolve()
        if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside the repository')
        if h.digest(lib)!=LIB_SHA:raise ValueError('runtime library hash mismatch')
        lock=json.loads((ROOT/'scripts/pc286-toolchain-lock.json').read_text(encoding='utf-8'))
        for row in lock['selected_members']:
            item=tool/row['path']
            if item.stat().st_size!=row['bytes'] or h.digest(item)!=row['sha256']:
                raise ValueError('pinned compiler identity mismatch: '+row['path'])
        out.mkdir(parents=True,exist_ok=False);env=os.environ.copy()
        for key in ('WCC','WATCOM','INCLUDE','LIB','WCL','WLINK'):env.pop(key,None)
        wcc,wlink=tool/'binnt64/wcc.exe',tool/'binnt64/wlink.exe'
        flags=['-zq','-2','-ms','-zl','-s','-os','-zastd=c99','-dCS_C16_REQUIRE_16BIT',
               '-dCLASSICK_PRODUCT_ROM_CACHE=0','-i='+str(tool/'h')]
        sources=[ROOT/name for name in SOURCES];guest=ROOT/'probes/pc286/boot-window-compositor.c'
        host=ROOT/'probes/pc286/window-host-reference.c';objects=[]
        for source in (*sources,guest):
            obj=out/(str(source.relative_to(ROOT)).replace('\\','-').replace('/','-')+'.obj')
            h.run([wcc,*flags,'-fo='+str(obj),source],out,env,out/(obj.name+'.log'));objects.append(obj)
        binary=out/'compositor.bin';mapping=out/'compositor.map'
        h.run([wlink,'format','raw','bin','option','start=start_','option','nodefaultlibs',
               'option','map='+str(mapping),'name',binary,'file',','.join(str(x) for x in objects),
               'library',lib],out,env,out/'link.log')
        text=mapping.read_text(encoding='utf-8',errors='replace')
        entry=re.search(r'Entry point address:\s*([0-9a-fA-F]+)',text)
        used=sorted(set(re.findall(r'clibs\.lib\(([^)]+)\)',text,re.I)))
        if not entry or used!=['i4d','i4m','i8d086','i8m086']:
            raise ValueError('unexpected link entry or runtime module set')
        image=out/'compositor.img'
        with image.open('xb') as f:f.write(floppy(binary.read_bytes(),int(entry.group(1),16)))
        before=h.digest(image);frame=out/'ega.ppm'
        result=json.loads(h.run([exe,'--machine','pc286','--floppy',image,'--cycles','80000000',
                                 '--timeout-ms','20000','--until-post','0xab','--inspect','0x60000:5',
                                 '--frame',frame],out,env,out/'boot.log'))
        if (result.get('status')!='condition_met' or result.get('post')!=0xab or
            result.get('memory',{}).get('bytes')!='433238365a' or h.digest(image)!=before):
            raise ValueError('unexpected guest result or changed floppy')
        hostexe=out/'host-reference.exe';pbm=out/'host-reference.pbm'
        h.run([cc,'-std=c11','-O2','-Wall','-Wextra','-Werror','-DCLASSICK_PRODUCT_ROM_CACHE=0',
               host,*sources,'-o',hostexe],out,env,out/'host-build.log')
        h.run([hostexe,pbm],out,env,out/'host-run.log');compare(pbm.read_bytes(),frame.read_bytes())
        report={'status':'pass','toolchain':lock['release'],'boothush_sha256':h.digest(exe),
                'host_compiler_sha256':h.digest(cc),'runtime_library_sha256':h.digest(lib),'runtime_modules':used,
                'source_sha256':{str(x.relative_to(ROOT)).replace('\\','/'):h.digest(x) for x in (*sources,guest,host)},
                'payload_bytes':binary.stat().st_size,'payload_sha256':h.digest(binary),'floppy_sha256':before,
                'host_pbm_sha256':h.digest(pbm),'ega_ppm_sha256':h.digest(frame),'guest_result':result,
                'canvas_pixels_match':512*342,'margin_pixels_match':640*350-512*342,'floppy_unchanged':True,
                'limits':'Static same-source hosted/286 compositor; third-party runtime not cleared for release; no input, Mac boot or physical claim.'}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
        print(json.dumps(report,indent=2));return 0
    except (OSError,ValueError,KeyError) as exc:
        print('286 shared compositor test failed: '+str(exc),file=sys.stderr);return 1

if __name__=='__main__':sys.exit(main())
