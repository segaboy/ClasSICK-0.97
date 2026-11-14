#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Check actual 286 EGA copy/clipping against a byte-aligned RGB oracle."""
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
CASES={'full':(0,0,512,342),'top-left':(0,0,1,1),'top-right':(511,0,512,1),
       'bottom-left':(0,341,1,342),'bottom-right':(511,341,512,342),
       'clipped':(-5,-4,13,3),'byte-cross':(7,20,9,22),'empty':(300,10,299,11)}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def compare(path,rect):
    raw=path.read_bytes();header=b'P6\n640 350\n255\n'
    if not raw.startswith(header) or len(raw)!=len(header)+640*350*3:raise ValueError('invalid PPM')
    left,top,right,bottom=rect
    left=max(0,left);top=max(0,top);right=min(512,right);bottom=min(342,bottom)
    first=left//8;last=(right+7)//8;pixels=raw[len(header):]
    for y in range(350):
        for x in range(640):
            gx=x-64;gy=y-4
            active=left<right and top<bottom and top<=gy<bottom and first<=gx//8<last and 0<=gx<512
            b=((gy*64+gx//8)*37+11)&255 if active else 0
            shade=0 if active and b & (0x80>>(gx%8)) else 255
            i=(y*640+x)*3
            if pixels[i:i+3]!=bytes((shade,shade,shade)):raise ValueError(f'{rect}: pixel mismatch at {x},{y}')

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--tool-root',required=True,type=Path);p.add_argument('--assembler',required=True,type=Path)
    p.add_argument('--emulator',required=True,type=Path)
    p.add_argument('--pc-results',required=True,type=Path);p.add_argument('--output',required=True,type=Path);a=p.parse_args()
    try:
        spec=importlib.util.spec_from_file_location('helpers',ROOT/'scripts/Test-BootHushPC286BankPlan.py')
        h=importlib.util.module_from_spec(spec);spec.loader.exec_module(h)
        tool=a.tool_root.resolve(strict=True);assembler=a.assembler.resolve(strict=True)
        exe=a.emulator.resolve(strict=True);pc=a.pc_results.resolve(strict=True);out=a.output.resolve()
        if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside the repository')
        report=json.loads((pc/'report.json').read_text(encoding='utf-8'))
        if report['status']!='pass' or report.get('timing_markers') or sha(exe)!=report['boothush_sha256']:
            raise ValueError('input build identity mismatch')
        for name,digest in report['source_sha256'].items():
            if sha(ROOT/name)!=digest:raise ValueError('source changed: '+name)
        if sha(assembler)!=report['assembler_sha256']:raise ValueError('assembler identity mismatch')
        lock=json.loads((ROOT/'scripts/pc286-toolchain-lock.json').read_text(encoding='utf-8'))
        for row in lock['selected_members']:
            item=tool/row['path']
            if item.stat().st_size!=row['bytes'] or sha(item)!=row['sha256']:raise ValueError('compiler mismatch')
        out.mkdir(parents=True,exist_ok=False);env=os.environ.copy()
        for key in ('WCC','WATCOM','INCLUDE','LIB','WCL','WLINK'):env.pop(key,None)
        wcc,wlink=tool/'binnt64/wcc.exe',tool/'binnt64/wlink.exe'
        flags=['-zq','-2','-ms','-zl','-s','-os','-zastd=c99','-dCS_C16_REQUIRE_16BIT',
               '-dCLASSICK_PRODUCT_ROM_CACHE=0','-i='+str(tool/'h'),'-i='+str(out)]
        sources=('core/graphics/surface.c','core/graphics/cursor.c','core/input/pointer.c',
                 'core/input/mouse.c','core/ui/window.c','apps/product-scene/scene.c',
                 'apps/window-scene/scene.c','apps/window-scene/input.c','platform/pc286/ega-mono.c')
        objects=[]
        for name in sources:
            obj=out/(name.replace('/','-')+'.obj')
            h.run([wcc,*flags,'-fo='+str(obj),ROOT/name],out,env,out/(obj.name+'.log'));objects.append(obj)
        obj=out/'original-arithmetic.obj'
        h.run([assembler,'-zq','-2','-ms','-fo='+str(obj),ROOT/'arch/i286/watcom-arithmetic.asm'],
              out,env,out/'arithmetic-assemble.log');objects.append(obj)
        observations=[]
        for name,rect in CASES.items():
            header=out/'present-case.h';header.write_text(''.join(f'#define {key} ({value})\n' for key,value in
                zip(('TEST_LEFT','TEST_TOP','TEST_RIGHT','TEST_BOTTOM'),rect)),encoding='ascii')
            obj=out/(name+'.obj');binary=out/(name+'.bin');mapping=out/(name+'.map')
            h.run([wcc,*flags,'-fo='+str(obj),ROOT/'probes/pc286/boot-present-edge.c'],out,env,out/(name+'-compile.log'))
            h.run([wlink,'format','raw','bin','option','start=start_','option','nodefaultlibs',
                   'option','map='+str(mapping),'name',binary,'file',','.join(str(x) for x in (*objects,obj))],
                  out,env,out/(name+'-link.log'))
            text=mapping.read_text(encoding='utf-8',errors='replace');entry=re.search(r'Entry point address:\s*([0-9a-fA-F]+)',text)
            bss=re.search(r'^_BSS\s+BSS\s+DGROUP\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)',text,re.M)
            if not entry or not bss or 'clibs.lib' in text.lower():raise ValueError('bad linker map')
            bss_end=sum(int(x,16) for x in bss.groups());image=out/(name+'.img');frame=out/(name+'.ppm')
            with image.open('xb') as f:f.write(floppy(binary.read_bytes(),int(entry.group(1),16),bss_end=bss_end,poison_bss=True))
            before=sha(image)
            result=json.loads(h.run([exe,'--machine','pc286','--floppy',image,'--cycles','20000000',
                '--timeout-ms','20000','--until-post','0xac','--frame',frame],out,env,out/(name+'-boot.log')))
            if result.get('status')!='condition_met' or result.get('post')!=0xac or sha(image)!=before:
                raise ValueError(name+': guest failed or input changed')
            compare(frame,rect)
            observations.append({'name':name,'rectangle':rect,'cycles':result['cycles'],'payload_sha256':sha(binary),
                                 'floppy_sha256':before,'frame_sha256':sha(frame),'pixels_checked':640*350})
        original=(out/'full.ppm').read_bytes();header=len(b'P6\n640 350\n255\n')
        for label,x,y in (('margin',0,0),('canvas',64,4)):
            changed=bytearray(original);changed[header+(y*640+x)*3]^=255
            mutant=out/(label+'-mutant.ppm')
            with mutant.open('xb') as f:f.write(changed)
            try:compare(mutant,CASES['full'])
            except ValueError:continue
            raise ValueError('oracle accepted mutated '+label+' pixel')
        summary={'status':'pass','boothush_sha256':sha(exe),'assembler_sha256':sha(assembler),
                 'source_sha256':{name:sha(ROOT/name) for name in (*sources,'arch/i286/watcom-arithmetic.asm',
                    'platform/pc286/ega-mono.h','probes/pc286/boot-window-shared.c',
                    'probes/pc286/boot-present-edge.c')},'cases':observations,
                 'pixel_mutations_rejected':2,
                 'limits':'Original modeled edge fixture; no physical display or release qualification.'}
        (out/'report.json').write_text(json.dumps(summary,indent=2)+'\n',encoding='utf-8');print(json.dumps(summary,indent=2));return 0
    except (OSError,ValueError,KeyError) as exc:
        print('286 EGA edge test failed: '+str(exc),file=sys.stderr);return 1

if __name__=='__main__':sys.exit(main())
