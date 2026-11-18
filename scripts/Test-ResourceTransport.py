#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Original streamed resource-fork fixtures on Windows, native 68000 and native 286."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import re
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
CORE=ROOT/'formats/resourcefork/transport.c'
ADAPTER=ROOT/'formats/resourcefork/mfs-transport.c'
VOLUME=ROOT/'core/storage/mfs-volume.c'
READER=ROOT/'formats/resourcefork/reader.c'
META=ROOT/'core/storage/mfs-read.c'
BLOCK=ROOT/'core/storage/block-read.c'
SHARED=ROOT/'probes/storage/resource-transport.c'
TEST=ROOT/'tests/resource-transport.c'


def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path)
    result=importlib.util.module_from_spec(spec);spec.loader.exec_module(result);return result


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--cc',required=True,type=Path)
    p.add_argument('--m68k-bin',required=True,type=Path)
    p.add_argument('--tool-root',required=True,type=Path)
    p.add_argument('--emulator',required=True,type=Path)
    p.add_argument('--output',required=True,type=Path)
    a=p.parse_args()
    try:
        h=module('mfs_helpers',ROOT/'scripts/Test-BootHushPC286BankPlan.py')
        loader=module('volume_loader',ROOT/'scripts/pc286_loader.py')
        native=module('mfs_m68k',ROOT/'scripts/m68k_windows_toolchain.py')
        cc=a.cc.resolve(strict=True);tool=a.tool_root.resolve(strict=True);exe=a.emulator.resolve(strict=True)
        out=a.output.resolve()
        if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside repository')
        lock_path=ROOT/'scripts/pc286-toolchain-lock.json'
        lock=json.loads(lock_path.read_text(encoding='utf-8'))
        for row in lock['selected_members']:
            path=tool/row['path']
            if path.stat().st_size!=row['bytes'] or h.digest(path)!=row['sha256']:
                raise ValueError('pinned C16 tool/header mismatch: '+row['path'])
        m68k,identity=native.inspect(a.m68k_bin)
        paths=(CORE,CORE.with_suffix('.h'),ADAPTER,ADAPTER.with_suffix('.h'),VOLUME,VOLUME.with_suffix('.h'),
               READER,READER.with_suffix('.h'),META,META.with_suffix('.h'),BLOCK,BLOCK.with_suffix('.h'),TEST,
               SHARED,SHARED.with_suffix('.h'),ROOT/'probes/mac128k/resource-transport.c',
               ROOT/'probes/pc286/boot-resource-transport.c',ROOT/'arch/i286/c16.h',
               ROOT/'arch/m68k/mac-reset.S',ROOT/'arch/m68k/mac-rom.ld',Path(__file__),
               ROOT/'docs/specifications/SPEC-0116-streamed-resource-fork.md',
               ROOT/'docs/specifications/SPEC-0024-resource-fork-reader.md',
               ROOT/'docs/specifications/SPEC-0115-mfs-volume-reader.md',
               ROOT/'scripts/m68k_windows_toolchain.py',ROOT/'scripts/Test-BootHushPC286BankPlan.py',
               ROOT/'scripts/pc286_loader.py',lock_path)
        # Admit exact inputs before the first build, then refuse a report if
        # any declared source/tool changes while the suite is executing.
        admitted_sources={str(path.relative_to(ROOT)).replace('\\','/'):h.digest(path) for path in paths}
        tool_paths={'host_cc':cc,'emulator':exe,'pc286_lock':lock_path}
        tool_paths.update({'pc286:'+row['path']:tool/row['path'] for row in lock['selected_members']})
        tool_paths.update({'m68k:'+name:path for name,path in m68k.items()})
        admitted_tools={name:h.digest(path) for name,path in tool_paths.items()}
        out.mkdir(parents=True,exist_ok=False)
        env=os.environ.copy();env['PATH']=str(cc.parent)+os.pathsep+str(m68k['gcc'].parent)+os.pathsep+env.get('PATH','')
        for name in ('WCC','WATCOM','INCLUDE','LIB','WCL','WLINK'):env.pop(name,None)
        commands=[]
        def run(command,label):
            commands.append([str(x) for x in command]);return h.run(command,out,env,out/(label+'.log'))
        warning=['-Wall','-Wextra','-Werror','-Wconversion','-Wsign-conversion']
        hosts=[]
        for name,flags in (('release',['-O3','-DNDEBUG']),('sanitized',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer'])):
            binary=out/(name+'.exe')
            run([cc,'-std=c11',*warning,*flags,BLOCK,META,VOLUME,READER,CORE,ADAPTER,TEST,'-o',binary],name+'-build')
            result=run([binary],name+'-test').decode().strip()
            if not re.fullmatch(r'Resource transport: [0-9]+ checks passed',result):raise ValueError('missing host result')
            hosts.append({'profile':name,'result':result,'sha256':h.digest(binary)})

        source=CORE.read_text(encoding='utf-8')
        controls={
            'duplicate-id':('    if(matches>1)return done(r,CS_RFS_DUPLICATE_ID,CS_RF_FIELD_ID,duplicate);',
                             '    if(matches>65535u)return done(r,CS_RFS_DUPLICATE_ID,CS_RF_FIELD_ID,duplicate);',0xc7),
            'early-publication':('        written+=amount;',
                                  '        copy(output+written,staging+written,amount);written+=amount;',0xc5)}
        cases=[('baseline',CORE,0xb7)]
        for name,(old,new,marker) in controls.items():
            if source.count(old)!=1:raise ValueError('changed mutation anchor: '+name)
            path=out/(name+'.c');path.write_text(source.replace(old,new),encoding='utf-8');cases.append((name,path,marker))

        wcc,wlink=tool/'binnt64/wcc.exe',tool/'binnt64/wlink.exe'
        flags=['-zq','-2','-zl','-s','-os','-zastd=c99','-we','-dCS_C16_REQUIRE_16BIT',
               '-i='+str(tool/'h'),'-i='+str(CORE.parent),'-i='+str(VOLUME.parent)]
        c16=[];model_objects={}
        for model in ('ms','mc'):
            for label,src in (('transport',CORE),('adapter',ADAPTER)):
                obj=out/(label+'-'+model+'.obj')
                run([wcc,*flags,'-'+model,'-fo='+str(obj),src],label+'-'+model+'-compile');model_objects[(model,label)]=obj
                metadata=run([tool/'binnt64/dmpobj.exe',obj],label+'-'+model+'-metadata').decode('utf-8',errors='replace').replace('\r\n','\n')
                if '80286' not in metadata or 'USE16' not in metadata:raise ValueError('C16 object identity')
                def symbols(kind):
                    sections=re.findall(r'(?:static )?'+kind+r'\([^\n]*\n(.*?)(?=\n(?:static )?[A-Z][A-Z0-9]+\(|\Z)',metadata,re.S)
                    return {value for section in sections for value in re.findall(r"'([^']+)'",section)}
                imports=sorted(symbols('L?EXTDEF')-symbols('L?PUBDEF'))
                if any(value not in ('cs_mfsv_fork_validate_','cs_mfsv_fork_read_') for value in imports):raise ValueError('unexpected C16 import: '+repr(imports))
                lengths=[int(x,16) for x in re.findall(r'USE16 Length ([0-9a-fA-F]+)h',metadata)]
                c16.append({'module':label,'model':model,'text_bytes':lengths[0],'other_segment_bytes':sum(lengths[1:]),'imports':imports,'sha256':h.digest(obj)})
        extra=[]
        for label,path in (('block',BLOCK),('metadata',META),('volume',VOLUME),('shared',SHARED),('entry',ROOT/'probes/pc286/boot-resource-transport.c')):
            obj=out/('pc-'+label+'.obj');run([wcc,*flags,'-ms','-fo='+str(obj),path],'pc-'+label+'-compile');extra.append(obj)
        pc=[]
        for name,path,marker in cases:
            obj=model_objects[('ms','transport')]
            if name!='baseline':
                obj=out/('pc-'+name+'.obj');run([wcc,*flags,'-ms','-fo='+str(obj),path],'pc-'+name+'-compile')
            binary=out/('pc-'+name+'.bin');mapping=out/('pc-'+name+'.map')
            run([wlink,'format','raw','bin','option','start=start_','option','nodefaultlibs','option','map='+str(mapping),
                 'name',binary,'file',','.join(str(x) for x in [obj,model_objects[('ms','adapter')],*extra])],'pc-'+name+'-link')
            text=mapping.read_text(encoding='utf-8',errors='replace');entry=re.search(r'Entry point address:\s*([0-9a-fA-F]+)',text)
            if entry is None or '.lib' in text.lower():raise ValueError('missing entry or unwanted runtime library')
            payload=binary.read_bytes();image=out/('pc-'+name+'.img')
            bss=re.search(r'^_BSS\s+BSS\s+DGROUP\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)',text,re.M)
            if not bss:raise ValueError('missing native BSS map')
            bss_start,bss_size=(int(v,16) for v in bss.groups())
            image.write_bytes(loader.floppy(payload,int(entry.group(1),16),bss_end=bss_start+bss_size,
                                            poison_bss=True,verify_payload=True));before=h.digest(image)
            inspections=[]
            for at in range(0,len(payload),4096):
                inspections+=['--inspect',hex(65536+at)+':'+str(min(4096,len(payload)-at))]
            response=json.loads(run([exe,'--machine','pc286','--floppy',image,'--cycles','100000000','--timeout-ms','20000',
                                     '--until-post',hex(marker),*inspections],'pc-'+name+'-boot'))
            ranges=response.pop('memory_ranges',None) or [response.pop('memory',{})]
            memory=b''.join(bytes.fromhex(r.get('bytes','')) for r in ranges)
            if (response.get('status')!='condition_met' or response.get('post')!=marker or response.get('cs')!=4096 or
                ranges[0].get('address')!=65536 or memory!=payload or h.digest(image)!=before):
                raise ValueError('286 target, loaded bytes or unchanged input check failed: '+name)
            pc.append({'case':name,'marker':marker,'result':response,'payload_bytes':len(payload),
                       'payload_sha256':h.digest(binary),'image_sha256':before,'complete_payload_matches':True,'input_unchanged':True})

        mac=[];frames={}
        for name,path,marker,opt in [('O0',CORE,0xb7,'-O0'),('Os',CORE,0xb7,'-Os'),*[(name,path,marker,'-Os') for name,path,marker in cases[1:]]]:
            sources=[('reset',ROOT/'arch/m68k/mac-reset.S'),('entry',ROOT/'probes/mac128k/resource-transport.c'),
                     ('shared',SHARED),('block',BLOCK),('metadata',META),('volume',VOLUME),('adapter',ADAPTER),('transport',path)]
            objects=[];case_frames=[]
            for label,src in sources:
                obj=out/('mac-'+name+'-'+label+'.o');objects.append(obj)
                compile_flags=['-mcpu=68000','-ffreestanding','-fno-builtin','-nostdlib',opt,'-I',CORE.parent,'-I',VOLUME.parent]
                if src.suffix=='.c':compile_flags+=['-std=c99',*warning,'-fstack-usage']
                run([m68k['gcc'],*compile_flags,'-c',src,'-o',obj],'mac-'+name+'-'+label+'-compile')
                if src.suffix=='.c':case_frames.extend(obj.with_suffix('.su').read_text(encoding='utf-8').splitlines())
            elf=out/('mac-'+name+'.elf');raw=out/('mac-'+name+'.raw');rom=out/('mac-'+name+'.rom')
            run([m68k['gcc'],'-mcpu=68000','-nostdlib','-Wl,--build-id=none','-Wl,-T,'+str(ROOT/'arch/m68k/mac-rom.ld'),
                 *objects,'-o',elf],'mac-'+name+'-link')
            if run([m68k['nm'],'-u',elf],'mac-'+name+'-imports').strip():raise ValueError('unexpected native Mac import')
            symbols=run([m68k['nm'],'-n','-S',elf],'mac-'+name+'-symbols').decode().replace('\r\n','\n')
            match=re.search(r'^([0-9a-fA-F]+)\s+[0-9a-fA-F]+\s+[BbDd]\s+cs_mac_diag\s*$',symbols,re.M)
            if match is None:raise ValueError('missing diagnostic symbol')
            address=int(match.group(1),16)
            if not 0xb00<=address<=0x2000-28:raise ValueError('diagnostic outside reset RAM region')
            run([m68k['objcopy'],'-O','binary',elf,raw],'mac-'+name+'-binary')
            payload=raw.read_bytes()
            if not 260<=len(payload)<=65536:raise ValueError('Mac diagnostic does not fit ROM')
            rom.write_bytes(payload+bytes([255])*(65536-len(payload)));before=h.digest(rom)
            response=json.loads(run([exe,'--machine','mac128k','--rom',rom,'--cycles','100000000','--timeout-ms','20000',
                                     '--until',hex(address)+':4:'+hex(marker),'--inspect',hex(address)+':28'],'mac-'+name+'-boot'))
            memory=response.pop('memory',{});actual=bytes.fromhex(memory.get('bytes',''))
            if (response.get('status')!='condition_met' or memory.get('address')!=address or len(actual)!=28 or
                int.from_bytes(actual[:4],'big')!=marker or actual[24:]!=bytes(4) or h.digest(rom)!=before):
                raise ValueError('Mac target, fault or unchanged input check failed: '+name)
            function_sizes={row[2]:int(row[1],16) for row in re.findall(r'^([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+[Tt]\s+(cs_rfs_\w+)\s*$',symbols,re.M)}
            frames[name]=case_frames
            mac.append({'case':name,'marker':marker,'result':response,'rom_sha256':before,'raw_bytes':len(payload),
                        'resource_public_function_bytes':function_sizes,'diag_address':address,'diag_hex':actual.hex(),'imports':[],'input_unchanged':True})
        _,after=native.inspect(a.m68k_bin)
        if after!=identity:raise ValueError('m68k frontend identity changed')
        if {str(path.relative_to(ROOT)).replace('\\','/'):h.digest(path) for path in paths}!=admitted_sources:
            raise ValueError('source/spec/helper input changed during verification')
        if {name:h.digest(path) for name,path in tool_paths.items()}!=admitted_tools:
            raise ValueError('declared tool identity changed during verification')
        report={'status':'pass','hosted':hosts,'c16':c16,'pc286':pc,'mac128k':mac,'m68k_frames':frames,
                'm68k_identity':identity,'cc_sha256':admitted_tools['host_cc'],'emulator_sha256':admitted_tools['emulator'],
                'pc286_lock_sha256':admitted_tools['pc286_lock'],'commands':commands,
                'source_sha256':admitted_sources,'tool_sha256':admitted_tools,'inputs_unchanged':True,
                'limits':'Original sparse MFS-backed resource-fork fixtures, maps beyond64KiB, native modeled targets; no Resource Manager handle/loading/trap semantics, physical storage, writable media or System boot. Frames are local compiler reports, not cumulative stack proof.'}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
        print(json.dumps({'status':'pass','hosted':hosts,'pc286_cases':len(pc),'mac128k_cases':len(mac),'report':str(out/'report.json')},indent=2))
        return 0
    except (OSError,ValueError,KeyError,subprocess.TimeoutExpired) as exc:
        print('Resource transport verification failed: '+str(exc),file=sys.stderr);return 1


if __name__=='__main__':sys.exit(main())
