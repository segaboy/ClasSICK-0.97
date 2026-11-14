#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Exercise shared window semantics through a bounded 286 keyboard/EGA fixture."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import re
import struct
import sys

ROOT=Path(__file__).resolve().parents[1]
SOURCES=(ROOT/'core/input/pointer.c',ROOT/'core/ui/window.c',ROOT/'probes/pc286/boot-window-keyboard.c')

def expected_frame(wx,wy,px,py,pattern):
    """Independent RGB oracle for the fixture's declared geometry."""
    width,height=640,350
    pixels=bytearray(width*height*3)
    def rect(l,t,r,b,white):
        l=max(0,l);t=max(0,t);r=min(512,r);b=min(342,b)
        if l>=r or t>=b:return
        row=bytes([255 if white else 0])*((r-l)*3)
        for y in range(t,b):
            offset=((y+4)*width+l+64)*3;pixels[offset:offset+len(row)]=row
    rect(0,0,512,342,True)
    rect(wx,wy,wx+224,wy+144,False)
    rect(wx+1,wy+1,wx+223,wy+17,True)
    rect(wx+1,wy+18,wx+223,wy+143,True)
    if pattern:
        for x in range(4,223,8):rect(wx+x,wy+18,wx+min(x+4,223),wy+143,False)
    else:
        for y in range(0,125,8):
            for x in range(0,223,8):
                if ((x//8)^(y//8))&1:rect(wx+max(x,1),wy+18+y,wx+min(x+8,223),wy+18+min(y+8,125),False)
    rect(px-1,py-7,px+2,py+8,True);rect(px-7,py-1,px+8,py+2,True)
    rect(px,py-7,px+1,py+8,False);rect(px-7,py,px+8,py+1,False)
    return b'P6\n640 350\n255\n'+pixels

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
        out.mkdir(parents=True,exist_ok=False);env=os.environ.copy()
        for key in ('WCC','WATCOM','INCLUDE','LIB','WCL','WLINK'):env.pop(key,None)
        wcc,wlink=tool/'binnt64/wcc.exe',tool/'binnt64/wlink.exe'
        flags=['-zq','-2','-ms','-zl','-s','-os','-zastd=c99','-dCS_C16_REQUIRE_16BIT','-i='+str(tool/'h')]
        objects=[]
        for source in SOURCES:
            obj=out/(source.stem+'.obj')
            h.run([wcc,*flags,'-fo='+str(obj),source],out,env,out/(source.stem+'-compile.log'))
            objects.append(obj)
        binary=out/'window.bin';mapfile=out/'window.map'
        h.run([wlink,'format','raw','bin','option','start=start_','option','nodefaultlibs',
               'option','map='+str(mapfile),'name',binary,'file',','.join(str(x) for x in objects)],
              out,env,out/'link.log')
        match=re.search(r'Entry point address:\s*([0-9a-fA-F]+)',mapfile.read_text(encoding='utf-8',errors='replace'))
        if not match:raise ValueError('missing linked entry address')
        image=out/'window.img'
        with image.open('xb') as f:f.write(h.floppy(binary.read_bytes(),int(match.group(1),16)))
        before=h.digest(image)
        # Set-2 makes: space toggle, arrows move. Break each key between makes.
        keys=[0x29,0x74,0x74,0x75,0x29,0x74,0x72,0x72,0x72,0x29,0x29]
        cases=(('initial',0,100,80,80,70,0,0,0),('pressed',1,100,80,80,70,0,1,1),
               ('moved',3,132,80,112,70,0,1,1),('released',5,132,64,112,54,0,0,0),
               ('post-release',6,148,64,112,54,0,0,0),('content',10,148,112,112,54,1,0,1),
               ('content-release',11,148,112,112,54,1,0,0))
        observations=[]
        for name,count,px,py,wx,wy,pattern,drag,button in cases:
            frame=out/(name+'.ppm')
            cmd=[exe,'--machine','pc286','--floppy',image,'--cycles','100000000','--timeout-ms','20000',
                 '--until-post',hex(0x90+count),'--inspect','0x60000:16','--frame',frame]
            for n,code in enumerate(keys[:count]):
                cycle=6000000+n*5000000
                for when,value in ((cycle,code),(cycle+100000,0xf0),(cycle+101000,code)):
                    cmd+=['--scancode',f'{when}:{value}']
            result=json.loads(h.run(cmd,out,env,out/(name+'.log')))
            expected=struct.pack('<4sBBHHHHBB',b'P286',count,button,px,py,wx,wy,pattern,drag)
            if (result.get('status')!='condition_met' or result.get('post')!=0x90+count or
                bytes.fromhex(result['memory']['bytes'])!=expected or h.digest(image)!=before):
                raise ValueError(name+': guest state mismatch or changed image')
            if frame.read_bytes()!=expected_frame(wx,wy,px,py,pattern):
                raise ValueError(name+': full EGA image differs from the independent RGB oracle')
            observations.append({'name':name,'result':result,'frame_sha256':h.digest(frame),
                                 'all_pixels_match':True,'floppy_unchanged':True})
        report={'status':'pass','compiler':lock['release'],'boothush_sha256':h.digest(exe),
                'source_sha256':{str(s.relative_to(ROOT)).replace('\\','/'):h.digest(s) for s in SOURCES},
                'payload_bytes':binary.stat().st_size,'payload_sha256':h.digest(binary),'floppy_sha256':before,
                'scheduled_set2_makes':keys,'cases':observations,
                'limits':'Preview experiment with original geometric renderer; not the shared compositor or a release.'}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
        print(json.dumps(report,indent=2));return 0
    except (OSError,ValueError,KeyError) as exc:
        print('286 keyboard window test failed: '+str(exc),file=sys.stderr);return 1

if __name__=='__main__':sys.exit(main())
