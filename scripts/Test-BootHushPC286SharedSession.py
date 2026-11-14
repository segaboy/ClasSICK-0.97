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
         'apps/window-scene/scene.c','apps/window-scene/input.c','platform/pc286/ega-mono.c',
         'platform/pc286/keyboard-pointer.c','platform/pc286/keyboard-set2.c',
         'platform/pc286/keyboard-config.c')

def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--tool-root',required=True,type=Path)
    runtime=p.add_mutually_exclusive_group(required=True)
    runtime.add_argument('--runtime-library',type=Path);runtime.add_argument('--assembler',type=Path)
    p.add_argument('--host-cc',required=True,type=Path);p.add_argument('--emulator',required=True,type=Path)
    p.add_argument('--output',required=True,type=Path);p.add_argument('--timing-markers',action='store_true')
    p.add_argument('--natural-start',action='store_true',help='test the continuous sequence from the shared pointer initial position')
    p.add_argument('--verify-payload',action='store_true',help='enable experimental diagnostic payload checksum')
    p.add_argument('--raw-keyboard',action='store_true',help='experimental translated set-1 make/break polling')
    p.add_argument('--irq-unmask-control',action='store_true',help='test-only unmask before taking IRQ1 ownership')
    p.add_argument('--config-control',choices=('bad-translation','pending-output'),help='test-only controller refusal');a=p.parse_args()
    try:
        h=module('helpers',ROOT/'scripts/Test-BootHushPC286BankPlan.py')
        oracle=module('compositor',ROOT/'scripts/Test-BootHushPC286SharedCompositor.py')
        tool=a.tool_root.resolve(strict=True)
        lib=a.runtime_library.resolve(strict=True) if a.runtime_library else None
        assembler=a.assembler.resolve(strict=True) if a.assembler else None
        cc=a.host_cc.resolve(strict=True);exe=a.emulator.resolve(strict=True);out=a.output.resolve()
        if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside the repository')
        if a.raw_keyboard and not a.natural_start:raise ValueError('raw keyboard test requires natural start')
        if a.irq_unmask_control and not a.raw_keyboard:raise ValueError('IRQ control requires raw keyboard')
        if a.config_control and not a.raw_keyboard:raise ValueError('config control requires raw keyboard')
        if lib and h.digest(lib)!=oracle.LIB_SHA:raise ValueError('runtime library hash mismatch')
        if assembler:
            arith=module('arithmetic',ROOT/'scripts/Test-BootHushPC286Arithmetic.py')
            if h.digest(assembler)!=arith.ASM_SHA:raise ValueError('assembler identity mismatch')
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
            extra=(['-dCS_P286_TIMING_MARKERS'] if a.timing_markers and source==guest else [])
            if not a.natural_start and source==guest:extra+=['-dCS_P286_PREPOSITION_TEST']
            if a.raw_keyboard and source==guest:extra+=['-dCS_P286_RAW_KEYBOARD']
            if a.irq_unmask_control and source==guest:extra+=['-dCS_P286_IRQ_TEST_UNMASK']
            if a.config_control and source==guest:
                extra+=['-dCS_P286_CONFIG_BAD_TRANSLATION' if a.config_control=='bad-translation' else '-dCS_P286_CONFIG_PENDING']
            h.run([wcc,*flags,*extra,'-fo='+str(obj),source],out,env,out/(obj.name+'.log'));objects.append(obj)
        if assembler:
            obj=out/'original-arithmetic.obj'
            h.run([assembler,'-zq','-2','-ms','-fo='+str(obj),ROOT/'arch/i286/watcom-arithmetic.asm'],
                  out,env,out/'arithmetic-assemble.log');objects.append(obj)
        binary=out/'session.bin';mapping=out/'session.map'
        h.run([wlink,'format','raw','bin','option','start=start_','option','nodefaultlibs',
               'option','map='+str(mapping),'name',binary,'file',','.join(str(x) for x in objects),
               *(['library',lib] if lib else [])],out,env,out/'link.log')
        text=mapping.read_text(encoding='utf-8',errors='replace')
        entry=re.search(r'Entry point address:\s*([0-9a-fA-F]+)',text)
        used=sorted(set(re.findall(r'clibs\.lib\(([^)]+)\)',text,re.I)))
        if not entry or used!=(['i4d','i4m'] if lib else []):
            raise ValueError('unexpected link entry or runtime module set')
        bss=re.search(r'^_BSS\s+BSS\s+DGROUP\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)',text,re.M)
        if not bss:raise ValueError('missing linked BSS bounds')
        bss_start,bss_size=(int(x,16) for x in bss.groups());bss_end=bss_start+bss_size
        if bss_start<binary.stat().st_size:raise ValueError('BSS overlaps initialized payload')
        image=out/'session.img'
        with image.open('xb') as f:f.write(floppy(binary.read_bytes(),int(entry.group(1),16),
                                                 bss_end=bss_end,poison_bss=True,verify_payload=a.verify_payload))
        before=h.digest(image)
        if a.config_control:
            cmd=[exe,'--machine','pc286','--floppy',image,'--cycles','10000000','--timeout-ms','20000',
                 '--until-post','0xe9','--inspect','0x60011:10']
            result=json.loads(h.run(cmd,out,env,out/'config-control-boot.log'))
            raw=bytes.fromhex(result['memory']['bytes'])
            if result.get('status')!='condition_met' or result.get('post')!=0xe9 or len(raw)!=10 or h.digest(image)!=before:
                raise ValueError('controller control did not reach refusal')
            old_flags,new_flags,old_mask,new_mask,reason,value,restored,status=struct.unpack('<HHBBBBBB',raw)
            expected=7 if a.config_control=='bad-translation' else 4
            if (reason!=expected or restored!=old_mask or new_mask!=(old_mask|2) or
                (old_flags&0x200)!=(new_flags&0x200) or (a.irq_unmask_control and old_mask&2) or
                (a.config_control=='bad-translation' and value&0x50) or
                (a.config_control=='pending-output' and not status&1)):
                raise ValueError('wrong controller refusal or IRQ restoration')
            report={'status':'pass','control':a.config_control,'boothush_sha256':h.digest(exe),
                    'payload_sha256':h.digest(binary),'floppy_sha256':before,'reason':reason,
                    'configuration_value':value,'status_after_refusal':status,'mask_before':old_mask,
                    'mask_during':new_mask,'mask_restored':restored,'flags_before':old_flags,
                    'flags_after':new_flags,'command':[str(x) for x in cmd],
                    'source_sha256':{str(x.relative_to(ROOT)).replace('\\','/'):h.digest(x) for x in
                                     (*sources,guest,ROOT/'platform/pc286/keyboard-config.h',ROOT/'scripts/pc286_loader.py')},
                    'limits':'Injected diagnostic refusal in BootHush, not a physical controller result.'}
            (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
            print(json.dumps(report,indent=2));return 0
        hostexe=out/'host-sequence.exe'
        h.run([cc,'-std=c11','-O2','-Wall','-Wextra','-Werror','-DCLASSICK_PRODUCT_ROM_CACHE=0',
               host,*sources,'-o',hostexe],out,env,out/'host-build.log')
        keys=[0x29,0x74,0x74,0x75,0x29,0x74,0x72,0x72,0x72,0x29,0x29]
        cases=(('initial',0,100,80,80,70,0,0,0,0),('pressed',1,100,80,80,70,0,1,1,1),
               ('moved',3,132,80,112,70,0,1,1,1),('released',5,132,64,112,54,0,0,0,1),
               ('post-release',6,148,64,112,54,0,0,0,1),('content',10,148,112,112,54,1,0,1,2),
               ('content-release',11,148,112,112,54,1,0,0,2))
        if a.natural_start:
            keys=[0x75]*6+[0x29,0x74,0x75,0x29]+[0x72]*3+[0x29,0x29]
            cases=(('initial',0,256,171,80,70,0,0,0,0),('title',6,256,75,80,70,0,0,0,0),
                   ('pressed',7,256,75,80,70,0,1,1,1),('moved',9,272,59,96,54,0,1,1,1),
                   ('released',10,272,59,96,54,0,0,0,1),('post-release',13,272,107,96,54,0,0,0,1),
                   ('content',14,272,107,96,54,1,0,1,2),('content-release',15,272,107,96,54,1,0,0,2))
        observations=[]
        for name,count,px,py,wx,wy,pattern,drag,button,presses in cases:
            frame=out/(name+'.ppm');pbm=out/(name+'.pbm')
            cmd=[exe,'--machine','pc286','--floppy',image,'--cycles','100000000','--timeout-ms','20000',
                 '--until-post',hex(0xb0+count),'--inspect','0x60000:17','--frame',frame]
            for n,code in enumerate(keys[:count]):
                cycle=6000000+n*5000000
                if a.raw_keyboard and code==0x29:
                    # Button press gets a repeated make; release is a real break.
                    sequence=((cycle,code),(cycle+100000,code)) if n in (6,13) else ((cycle,0xf0),(cycle+1000,code))
                elif a.raw_keyboard:
                    sequence=((cycle,0xe0),(cycle+1000,code),(cycle+100000,0xe0),(cycle+101000,0xf0),(cycle+102000,code))
                else:sequence=((cycle,code),(cycle+100000,0xf0),(cycle+101000,code))
                for when,value in sequence:
                    cmd+=['--scancode',f'{when}:{value}']
            result=json.loads(h.run(cmd,out,env,out/(name+'-boot.log')))
            expected=struct.pack('<4sBBHHHHBBB',b'S286',count,button,px,py,wx,wy,pattern,drag,presses)
            if (result.get('status')!='condition_met' or result.get('post')!=0xb0+count or
                bytes.fromhex(result['memory']['bytes'])!=expected or h.digest(image)!=before):
                raise ValueError(name+': guest state mismatch or changed input')
            h.run([hostexe,('n'+str(count)) if a.natural_start else str(count),pbm],out,env,out/(name+'-host.log'))
            oracle.compare(pbm.read_bytes(),frame.read_bytes(),margin_white=True)
            observations.append({'name':name,'guest_result':result,'host_pbm_sha256':h.digest(pbm),
                                 'ega_ppm_sha256':h.digest(frame),'canvas_pixels_match':512*342,
                                 'margin_pixels_match':640*350-512*342})
        irq_observation=None
        if a.raw_keyboard:
            cmd=[exe,'--machine','pc286','--floppy',image,'--cycles','10000000','--timeout-ms','20000',
                 '--until-post','0xb0','--inspect','0x60011:6']
            irq_result=json.loads(h.run(cmd,out,env,out/'irq-mask-boot.log'))
            raw=bytes.fromhex(irq_result['memory']['bytes'])
            if irq_result.get('status')!='condition_met' or len(raw)!=6:raise ValueError('missing IRQ observation')
            before_flags,after_flags,before_mask,after_mask=struct.unpack('<HHBB',raw)
            if (before_flags&0x200)!=(after_flags&0x200) or after_mask!=(before_mask|2):
                raise ValueError('keyboard IRQ mask or interrupt-enable state changed unexpectedly')
            if a.irq_unmask_control and before_mask&2:raise ValueError('IRQ control did not start unmasked')
            irq_observation={'before_flags':before_flags,'after_flags':after_flags,
                             'before_mask':before_mask,'after_mask':after_mask,'command':[str(x) for x in cmd]}
            cmd=[exe,'--machine','pc286','--floppy',image,'--cycles','10000000','--timeout-ms','20000',
                 '--until-post','0xb0','--inspect','0x60017:4']
            config_result=json.loads(h.run(cmd,out,env,out/'config-ready-boot.log'))
            config=bytes.fromhex(config_result['memory']['bytes'])
            if (config_result.get('status')!='condition_met' or len(config)!=4 or config[0]!=3 or
                (config[1]&0x50)!=0x40 or config[2]!=after_mask or config[3]&0xe1):
                raise ValueError('configuration query not ready or stream not empty')
            irq_observation['config_result']=list(config)
        # Prove the pixel comparison rejects one changed canvas pixel and one margin pixel.
        raw=(out/'initial.ppm').read_bytes();pbm=(out/'initial.pbm').read_bytes();header=len(b'P6\n640 350\n255\n')
        for x,y in ((100,100),(0,0)):
            altered=bytearray(raw);at=header+(y*640+x)*3;altered[at]^=255
            try:oracle.compare(pbm,bytes(altered),margin_white=True)
            except ValueError:continue
            raise ValueError('pixel comparison missed deliberate mutation')
        report={'status':'pass','toolchain':lock['release'],'boothush_sha256':h.digest(exe),
                'host_compiler_sha256':h.digest(cc),'runtime_library_sha256':h.digest(lib) if lib else None,
                'assembler_sha256':h.digest(assembler) if assembler else None,'runtime_modules':used,
                'source_sha256':{str(x.relative_to(ROOT)).replace('\\','/'):h.digest(x) for x in
                                 (*sources,guest,host,ROOT/'platform/pc286/ega-mono.h',
                                  ROOT/'platform/pc286/keyboard-pointer.h',ROOT/'scripts/pc286_loader.py',
                                  ROOT/'platform/pc286/keyboard-set2.h',ROOT/'platform/pc286/keyboard-config.h',
                                  ROOT/'scripts/Test-BootHushPC286BankPlan.py',
                                  ROOT/'scripts/Test-BootHushPC286SharedCompositor.py',Path(__file__),
                                  *((ROOT/'arch/i286/watcom-arithmetic.asm',) if assembler else ()))},
                'payload_bytes':binary.stat().st_size,'payload_sha256':h.digest(binary),'floppy_sha256':before,
                'bss_start':bss_start,'bss_bytes':bss_size,'bss_poisoned_before_load':True,
                'timing_markers':a.timing_markers,
                'natural_start':a.natural_start,
                'verify_payload':a.verify_payload,
                'raw_keyboard':a.raw_keyboard,
                'irq_unmask_control':a.irq_unmask_control,
                'config_control':a.config_control,
                'irq_mask_observation':irq_observation,
                'cases':observations,'floppy_unchanged':True,'pixel_mutations_rejected':2,
                'limits':'Same-source hosted/286 scripted input; experimental helpers require review; no Mac boot, E3/E4 or physical claim.'}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
        print(json.dumps(report,indent=2));return 0
    except (OSError,ValueError,KeyError) as exc:
        print('286 shared session test failed: '+str(exc),file=sys.stderr);return 1

if __name__=='__main__':sys.exit(main())
