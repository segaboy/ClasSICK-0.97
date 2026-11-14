#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Test real shared window owner and compositor with 286 keyboard/EGA input."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import re
import struct
import sys
from pc286_loader import floppy

ROOT=Path(__file__).resolve().parents[1]
SOURCES=('core/graphics/surface.c','core/graphics/cursor.c','core/input/pointer.c',
         'core/input/mouse.c','core/ui/window.c','apps/product-scene/scene.c',
         'apps/window-scene/scene.c','apps/window-scene/input.c')

def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--tool-root',required=True,type=Path);p.add_argument('--runtime-library',required=True,type=Path)
    p.add_argument('--host-cc',required=True,type=Path);p.add_argument('--emulator',required=True,type=Path)
    p.add_argument('--output',required=True,type=Path);a=p.parse_args()
    try:
        h=module('helpers',ROOT/'scripts/Test-BootHushPC286BankPlan.py')
        oracle=module('compositor',ROOT/'scripts/Test-BootHushPC286SharedCompositor.py')
        tool=a.tool_root.resolve(strict=True);lib=a.runtime_library.resolve(strict=True)
        cc=a.host_cc.resolve(strict=True);exe=a.emulator.resolve(strict=True);out=a.output.resolve()
        if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside the repository')
        if h.digest(lib)!=oracle.LIB_SHA:raise ValueError('runtime library hash mismatch')
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
        sources=[ROOT/name for name in SOURCES];guest=ROOT/'probes/pc286/boot-window-shared.c'
        host=ROOT/'probes/pc286/window-host-sequence.c';objects=[]
        for source in (*sources,guest):
            obj=out/(str(source.relative_to(ROOT)).replace('\\','-').replace('/','-')+'.obj')
            h.run([wcc,*flags,'-fo='+str(obj),source],out,env,out/(obj.name+'.log'));objects.append(obj)
        binary=out/'session.bin';mapping=out/'session.map'
        h.run([wlink,'format','raw','bin','option','start=start_','option','nodefaultlibs',
               'option','map='+str(mapping),'name',binary,'file',','.join(str(x) for x in objects),
               'library',lib],out,env,out/'link.log')
        text=mapping.read_text(encoding='utf-8',errors='replace')
        entry=re.search(r'Entry point address:\s*([0-9a-fA-F]+)',text)
        used=sorted(set(re.findall(r'clibs\.lib\(([^)]+)\)',text,re.I)))
        if not entry or used!=['i4d','i4m','i8d086','i8m086']:
            raise ValueError('unexpected link entry or runtime module set')
        image=out/'session.img'
        with image.open('xb') as f:f.write(floppy(binary.read_bytes(),int(entry.group(1),16)))
        before=h.digest(image);hostexe=out/'host-sequence.exe'
        h.run([cc,'-std=c11','-O2','-Wall','-Wextra','-Werror','-DCLASSICK_PRODUCT_ROM_CACHE=0',
               host,*sources,'-o',hostexe],out,env,out/'host-build.log')
        keys=[0x29,0x74,0x74,0x75,0x29,0x74,0x72,0x72,0x72,0x29,0x29]
        cases=(('initial',0,100,80,80,70,0,0,0,0),('pressed',1,100,80,80,70,0,1,1,1),
               ('moved',3,132,80,112,70,0,1,1,1),('released',5,132,64,112,54,0,0,0,1),
               ('post-release',6,148,64,112,54,0,0,0,1),('content',10,148,112,112,54,1,0,1,2),
               ('content-release',11,148,112,112,54,1,0,0,2))
        observations=[]
        for name,count,px,py,wx,wy,pattern,drag,button,presses in cases:
            frame=out/(name+'.ppm');pbm=out/(name+'.pbm')
            cmd=[exe,'--machine','pc286','--floppy',image,'--cycles','100000000','--timeout-ms','20000',
                 '--until-post',hex(0xb0+count),'--inspect','0x60000:17','--frame',frame]
            for n,code in enumerate(keys[:count]):
                cycle=6000000+n*5000000
                for when,value in ((cycle,code),(cycle+100000,0xf0),(cycle+101000,code)):
                    cmd+=['--scancode',f'{when}:{value}']
            result=json.loads(h.run(cmd,out,env,out/(name+'-boot.log')))
            expected=struct.pack('<4sBBHHHHBBB',b'S286',count,button,px,py,wx,wy,pattern,drag,presses)
            if (result.get('status')!='condition_met' or result.get('post')!=0xb0+count or
                bytes.fromhex(result['memory']['bytes'])!=expected or h.digest(image)!=before):
                raise ValueError(name+': guest state mismatch or changed input')
            h.run([hostexe,str(count),pbm],out,env,out/(name+'-host.log'))
            oracle.compare(pbm.read_bytes(),frame.read_bytes(),margin_white=True)
            observations.append({'name':name,'guest_result':result,'host_pbm_sha256':h.digest(pbm),
                                 'ega_ppm_sha256':h.digest(frame),'canvas_pixels_match':512*342,
                                 'margin_pixels_match':640*350-512*342})
        # Prove the pixel comparison rejects one changed canvas pixel and one margin pixel.
        raw=(out/'initial.ppm').read_bytes();pbm=(out/'initial.pbm').read_bytes();header=len(b'P6\n640 350\n255\n')
        for x,y in ((100,100),(0,0)):
            altered=bytearray(raw);at=header+(y*640+x)*3;altered[at]^=255
            try:oracle.compare(pbm,bytes(altered),margin_white=True)
            except ValueError:continue
            raise ValueError('pixel comparison missed deliberate mutation')
        report={'status':'pass','toolchain':lock['release'],'boothush_sha256':h.digest(exe),
                'host_compiler_sha256':h.digest(cc),'runtime_library_sha256':h.digest(lib),'runtime_modules':used,
                'source_sha256':{str(x.relative_to(ROOT)).replace('\\','/'):h.digest(x) for x in (*sources,guest,host)},
                'payload_bytes':binary.stat().st_size,'payload_sha256':h.digest(binary),'floppy_sha256':before,
                'cases':observations,'floppy_unchanged':True,'pixel_mutations_rejected':2,
                'limits':'Same-source hosted/286 scripted input; runtime not cleared for release; no Mac boot, E3/E4 or physical claim.'}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
        print(json.dumps(report,indent=2));return 0
    except (OSError,ValueError,KeyError) as exc:
        print('286 shared session test failed: '+str(exc),file=sys.stderr);return 1

if __name__=='__main__':sys.exit(main())
