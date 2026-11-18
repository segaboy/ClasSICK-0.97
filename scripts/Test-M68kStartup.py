#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Original guarded 68000 address/frame tests and native modeled probes."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import re
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
BUS=ROOT/'core/cpu/m68k-bus.c'
CORE=[BUS,ROOT/'core/cpu/m68k.c',ROOT/'core/memory/guest.c',ROOT/'core/services/traps.c']
PROBE=ROOT/'probes/cpu/m68k-startup.c'

def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path)
    result=importlib.util.module_from_spec(spec);spec.loader.exec_module(result);return result

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('cc','m68k-bin','tool-root','emulator','output'):parser.add_argument('--'+name,required=True,type=Path)
    args=parser.parse_args()
    try:
        helpers=module('startup_helpers',ROOT/'scripts/Test-BootHushPC286BankPlan.py')
        loader=module('startup_loader',ROOT/'scripts/pc286_loader.py')
        native=module('startup_m68k',ROOT/'scripts/m68k_windows_toolchain.py')
        cc=args.cc.resolve(strict=True);tool=args.tool_root.resolve(strict=True);emulator=args.emulator.resolve(strict=True)
        out=args.output.resolve()
        if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside repository')
        lock_path=ROOT/'scripts/pc286-toolchain-lock.json';lock=json.loads(lock_path.read_text(encoding='utf-8'))
        for row in lock['selected_members']:
            path=tool/row['path']
            if path.stat().st_size!=row['bytes'] or helpers.digest(path)!=row['sha256']:raise ValueError('pinned C16 tool/header mismatch: '+row['path'])
        m68k,identity=native.inspect(args.m68k_bin)
        inputs=[*CORE,PROBE,ROOT/'probes/cpu/m68k-startup.h',ROOT/'probes/mac128k/m68k-startup.c',
                ROOT/'probes/pc286/boot-m68k-startup.c',ROOT/'core/cpu/m68k-bus.h',ROOT/'core/cpu/m68k-guard.h',
                ROOT/'core/cpu/m68k-backend.h',ROOT/'core/cpu/m68k-transfer.h',
                ROOT/'core/cpu/m68k.h',ROOT/'core/memory/guest.h',ROOT/'core/services/traps.h',
                ROOT/'tests/m68k-startup.c',ROOT/'tests/m68k-bus.c',ROOT/'tests/m68k-guard.c',
                ROOT/'apps/memory-probe/probe.c',ROOT/'apps/memory-probe/probe.h',ROOT/'apps/memory-probe/program.h',ROOT/'apps/aline-probe/program.h',
                ROOT/'docs/specifications/SPEC-0118-m68k-address-stack.md',ROOT/'provenance/records/SRC-0616-m68k-address-stack-instructions.md',
                ROOT/'arch/i286/c16.h',ROOT/'arch/m68k/mac-reset.S',ROOT/'arch/m68k/mac-rom.ld',
                ROOT/'scripts/m68k_windows_toolchain.py',ROOT/'scripts/Test-BootHushPC286BankPlan.py',
                ROOT/'scripts/pc286_loader.py',lock_path,Path(__file__)]
        source_hashes={str(path.relative_to(ROOT)).replace('\\','/'):helpers.digest(path) for path in inputs}
        tool_paths={'host_cc':cc,'emulator':emulator,'pc286_lock':lock_path,'python':Path(sys.executable).resolve(strict=True)}
        tool_paths.update({'pc286:'+row['path']:tool/row['path'] for row in lock['selected_members']})
        tool_paths.update({'m68k:'+name:path for name,path in m68k.items()})
        tool_hashes={name:helpers.digest(path) for name,path in tool_paths.items()}
        out.mkdir(parents=True,exist_ok=False)
        env=os.environ.copy();env['PATH']=str(cc.parent)+os.pathsep+str(m68k['gcc'].parent)+os.pathsep+env.get('PATH','')
        for name in ('WCC','WATCOM','INCLUDE','LIB','WCL','WLINK'):env.pop(name,None)
        commands=[]
        def run(command,label):
            commands.append([str(item) for item in command]);return helpers.run(command,out,env,out/(label+'.log'))
        warnings=['-Wall','-Wextra','-Werror','-Wconversion','-Wsign-conversion']
        suites={'startup':[''],'bus':['registers','operands','branches','failures','services','probe'],
                'guard':['events','atomicity','control','services','aliases']}
        hosts=[]
        for profile,flags in [('release',['-O3','-DNDEBUG']),('sanitized',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer'])]:
            for name in ('startup','bus','guard'):
                binary=out/(profile+'-'+name+'.exe')
                extra=[ROOT/'apps/memory-probe/probe.c'] if name=='bus' else []
                run([cc,'-std=c11',*warnings,*flags,*CORE,*extra,ROOT/('tests/m68k-'+name+'.c'),'-o',binary],profile+'-'+name+'-build')
                for case in suites[name]:
                    result=run([binary,*([case] if case else [])],profile+'-'+name+'-'+case+'-test').decode().strip()
                    hosts.append({'profile':profile,'suite':name,'case':case,'result':result or 'exit 0','sha256':helpers.digest(binary)})
        # The C99 alignment fallback is also exercised by the full host suites.
        for name in ('startup','guard'):
            binary=out/('c99-'+name+'.exe')
            run([cc,'-std=c99',*warnings,'-O2',*CORE,ROOT/('tests/m68k-'+name+'.c'),'-o',binary],'c99-'+name+'-build')
            for case in suites[name]:
                result=run([binary,*([case] if case else [])],'c99-'+name+'-'+case+'-test').decode().strip()
                hosts.append({'profile':'c99','suite':name,'case':case,'result':result or 'exit 0','sha256':helpers.digest(binary)})
        source=BUS.read_text(encoding='utf-8')
        controls={
            'pea-old-sp':('        status=preflight_store(t,slot,4,at);','        status=preflight_store(t,slot,4,at-4u);',0xe2),
            'unlk-increment':('        status=add_offset(t,slot,4,&t->frame.a[7]);','        status=add_offset(t,slot,2,&t->frame.a[7]);',0xe3),
            'early-write':('        status=add_offset(t,slot,sign_extend(value,16),&t->frame.a[7]);',
                           '        for(unsigned i=0;i<t->store_bytes;++i)t->store[i]=t->data[i];\n        status=add_offset(t,slot,sign_extend(value,16),&t->frame.a[7]);',0xe4)}
        cases=[('baseline',BUS,0xb8)]
        for name,(old,new,marker) in controls.items():
            if source.count(old)!=1:raise ValueError('changed mutation anchor: '+name)
            path=out/(name+'.c');path.write_text(source.replace(old,new),encoding='utf-8');cases.append((name,path,marker))
        wcc,wlink=tool/'binnt64/wcc.exe',tool/'binnt64/wlink.exe'
        cflags=['-zq','-2','-zl','-s','-os','-zastd=c99','-we','-dCS_C16_REQUIRE_16BIT','-i='+str(tool/'h'),'-i='+str(BUS.parent)]
        c16=[];baseline_objects={}
        for model in ('ms','mc'):
            objects=[];model_rows=[]
            for index,path in enumerate([*CORE,PROBE]):
                obj=out/(model+'-'+str(index)+'.obj');objects.append(obj)
                run([wcc,*cflags,'-'+model,'-fo='+str(obj),path],model+'-'+str(index)+'-compile')
                metadata=run([tool/'binnt64/dmpobj.exe',obj],model+'-'+str(index)+'-metadata').decode('utf-8',errors='replace').replace('\r\n','\n')
                if '80286' not in metadata or 'USE16' not in metadata:raise ValueError('C16 object identity')
                def symbols(kind):
                    sections=re.findall(r'(?:static )?'+kind+r'\([^\n]*\n(.*?)(?=\n(?:static )?[A-Z][A-Z0-9]+\(|\Z)',metadata,re.S)
                    return {value for section in sections for value in re.findall(r"'([^']+)'",section)}
                imports=sorted(symbols('L?EXTDEF')-symbols('L?PUBDEF'))
                allowed=({'cs_guest_validate_','cs_guest_read_','cs_guest_window_','cs_m68k_aline_'},
                         {'cs_trap_call_'},set(),set(),{'cs_trap_init_','cs_m68k_bus_step_guarded_'})[index]
                if not set(imports)<=allowed:raise ValueError('unexpected C16 import: '+repr(imports))
                lengths=[int(value,16) for value in re.findall(r'USE16 Length ([0-9a-fA-F]+)h',metadata)]
                model_rows.append({'source':str(path.relative_to(ROOT)).replace('\\','/'),'text_bytes':lengths[0],
                                   'other_segment_bytes':sum(lengths[1:]),'imports':imports,'sha256':helpers.digest(obj)})
            c16.append({'model':model,'objects':model_rows});baseline_objects[model]=objects
        entry=out/'pc-entry.obj'
        run([wcc,*cflags,'-ms','-fo='+str(entry),ROOT/'probes/pc286/boot-m68k-startup.c'],'pc-entry-compile')
        pc=[]
        for name,path,marker in cases:
            objects=list(baseline_objects['ms'])
            if name!='baseline':
                obj=out/('pc-'+name+'.obj');run([wcc,*cflags,'-ms','-fo='+str(obj),path],'pc-'+name+'-compile');objects[0]=obj
            binary=out/('pc-'+name+'.bin');mapping=out/('pc-'+name+'.map')
            run([wlink,'format','raw','bin','option','start=start_','option','nodefaultlibs','option','map='+str(mapping),
                 'name',binary,'file',','.join(str(item) for item in [*objects,entry])],'pc-'+name+'-link')
            text=mapping.read_text(encoding='utf-8',errors='replace');start=re.search(r'Entry point address:\s*([0-9a-fA-F]+)',text)
            bss=re.search(r'^_BSS\s+BSS\s+DGROUP\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)',text,re.M)
            if start is None or bss is None or '.lib' in text.lower():raise ValueError('missing native map or unwanted runtime library')
            bss_start,bss_size=(int(value,16) for value in bss.groups())
            payload=binary.read_bytes();image=out/('pc-'+name+'.img')
            image.write_bytes(loader.floppy(payload,int(start.group(1),16),bss_end=bss_start+bss_size,poison_bss=True,verify_payload=True))
            before=helpers.digest(image);inspection=[]
            for at in range(0,len(payload),4096):inspection+=['--inspect',hex(65536+at)+':'+str(min(4096,len(payload)-at))]
            response=json.loads(run([emulator,'--machine','pc286','--floppy',image,'--cycles','30000000','--timeout-ms','20000',
                                     '--until-post',hex(marker),*inspection],'pc-'+name+'-boot'))
            ranges=response.pop('memory_ranges',None) or [response.pop('memory',{})]
            actual=b''.join(bytes.fromhex(row.get('bytes','')) for row in ranges)
            if (response.get('status')!='condition_met' or response.get('post')!=marker or response.get('cs')!=4096 or
                ranges[0].get('address')!=65536 or actual!=payload or helpers.digest(image)!=before):raise ValueError('286 target or input check failed: '+name)
            pc.append({'case':name,'marker':marker,'result':response,'payload_bytes':len(payload),'bss_bytes':bss_size,
                       'payload_sha256':helpers.digest(binary),'image_sha256':before,'complete_payload_matches':True,'input_unchanged':True})
        mac=[];frames={}
        for name,path,marker,opt in [('O0',BUS,0xb8,'-O0'),('Os',BUS,0xb8,'-Os'),*[(name,path,marker,'-Os') for name,path,marker in cases[1:]]]:
            sources=[ROOT/'arch/m68k/mac-reset.S',ROOT/'probes/mac128k/m68k-startup.c',PROBE,path,*CORE[1:]]
            objects=[];case_frames=[]
            for index,src in enumerate(sources):
                obj=out/('mac-'+name+'-'+str(index)+'.o');objects.append(obj)
                flags=['-mcpu=68000','-ffreestanding','-fno-builtin','-nostdlib',opt,'-I',BUS.parent]
                if src.suffix=='.c':flags+=['-std=c99',*warnings,'-fstack-usage']
                run([m68k['gcc'],*flags,'-c',src,'-o',obj],'mac-'+name+'-'+str(index)+'-compile')
                if src.suffix=='.c':case_frames.extend(obj.with_suffix('.su').read_text(encoding='utf-8').splitlines())
            elf=out/('mac-'+name+'.elf');raw=out/('mac-'+name+'.raw');rom=out/('mac-'+name+'.rom')
            run([m68k['gcc'],'-mcpu=68000','-nostdlib','-Wl,--build-id=none','-Wl,-T,'+str(ROOT/'arch/m68k/mac-rom.ld'),*objects,'-o',elf],'mac-'+name+'-link')
            if run([m68k['nm'],'-u',elf],'mac-'+name+'-imports').strip():raise ValueError('unexpected Mac import')
            symbols=run([m68k['nm'],'-n','-S',elf],'mac-'+name+'-symbols').decode().replace('\r\n','\n')
            match=re.search(r'^([0-9a-fA-F]+)\s+[0-9a-fA-F]+\s+[BbDd]\s+cs_mac_diag\s*$',symbols,re.M)
            if match is None:raise ValueError('missing diagnostic symbol')
            address=int(match.group(1),16)
            if not 0xb00<=address<=0x2000-28:raise ValueError('diagnostic outside reset RAM region')
            run([m68k['objcopy'],'-O','binary',elf,raw],'mac-'+name+'-binary');payload=raw.read_bytes()
            if not 260<=len(payload)<=65536:raise ValueError('Mac probe does not fit ROM')
            rom.write_bytes(payload+bytes([255])*(65536-len(payload)));before=helpers.digest(rom)
            response=json.loads(run([emulator,'--machine','mac128k','--rom',rom,'--cycles','50000000','--timeout-ms','20000',
                                     '--until',hex(address)+':4:'+hex(marker),'--inspect',hex(address)+':28'],'mac-'+name+'-boot'))
            memory=response.pop('memory',{});actual=bytes.fromhex(memory.get('bytes',''))
            if (response.get('status')!='condition_met' or memory.get('address')!=address or len(actual)!=28 or
                int.from_bytes(actual[:4],'big')!=marker or actual[24:]!=bytes(4) or helpers.digest(rom)!=before):raise ValueError('Mac target or input check failed: '+name)
            function_sizes={function:int(length,16) for _,length,function in re.findall(r'^([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+[Tt]\s+(address_instruction|frame_instruction|step|envelope)\s*$',symbols,re.M)}
            frames[name]=case_frames
            mac.append({'case':name,'marker':marker,'result':response,'rom_sha256':before,'raw_bytes':len(payload),
                        'bus_function_bytes':function_sizes,'diag_address':address,'diag_hex':actual.hex(),'imports':[],'input_unchanged':True})
        _,after=native.inspect(args.m68k_bin)
        if after!=identity:raise ValueError('m68k identity changed')
        if {str(path.relative_to(ROOT)).replace('\\','/'):helpers.digest(path) for path in inputs}!=source_hashes:raise ValueError('declared input changed during verification')
        if {name:helpers.digest(path) for name,path in tool_paths.items()}!=tool_hashes:raise ValueError('declared tool changed during verification')
        report={'status':'pass','hosted':hosts,'c16':c16,'pc286':pc,'mac128k':mac,'m68k_frames':frames,'m68k_identity':identity,
                'source_sha256':source_hashes,'tool_sha256':tool_hashes,'commands':commands,'inputs_unchanged':True,
                'limits':'Original guarded interpreter fixture; native modeled 68000 and small-model286, with compact-model import audit only. No general segmented backing, protected program, physical target, full CPU, timing or System startup claim. Frames are individual compiler reports, not cumulative stack proof.'}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
        print(json.dumps({'status':'pass','host_suites':len(hosts),'pc286_cases':len(pc),'mac128k_cases':len(mac),'report':str(out/'report.json')},indent=2));return 0
    except (OSError,ValueError,KeyError,subprocess.TimeoutExpired) as exc:
        print('M68K startup verification failed: '+str(exc),file=sys.stderr);return 1

if __name__=='__main__':sys.exit(main())
