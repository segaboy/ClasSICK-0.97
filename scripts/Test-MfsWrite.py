#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Original existing-fork overwrites through native Mac/286 sector providers."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
COMMON=[ROOT/'core/storage'/name for name in ('block-read.c','mfs-read.c','mfs-volume.c')]
WRITE=ROOT/'core/storage/mfs-write.c'
SHARED=ROOT/'probes/storage/mfs-write.c'
FORKS=([16,17,22,23,18,19],[20,21],[24,25])
LENGTHS=(2437,17,513)


def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path)
    result=importlib.util.module_from_spec(spec);spec.loader.exec_module(result);return result
def checksum(data):
    acc=0
    for (word,) in struct.iter_unpack('>H',data):
        acc=(acc+word)&0xffffffff;acc=(acc>>1)|((acc&1)<<31)
    return acc
def dc42(data):
    tags=bytes((lba*11+i*7+3)&255 for lba in range(800) for i in range(12))
    header=bytearray(84);header[:9]=b'\x08Own MFSW';header[12:60]=bytes(range(48))
    struct.pack_into('>4I4B',header,64,len(data),len(tags),checksum(data),checksum(tags[12:]),0,2,1,0)
    return bytes(header)+data+tags
def container_result(source,logical):
    if len(source)==409600:return logical
    header=bytearray(source[:84]);tags=source[84+409600:]
    struct.pack_into('>II',header,72,checksum(logical),checksum(tags[12:]))
    return bytes(header)+logical+tags
def logical_result(original,style):
    output=bytearray(original)
    operations=[(0,500,524 if style==2 else 1036 if style==3 else 1100,0x31)]
    if style==1:operations += [(1,3,11,0x53),(2,500,13,0x75),(0,2400,37,0x97),(0,0,512,0xb9)]
    for fork,offset,count,salt in operations:
        for i in range(count):
            logical=offset+i
            output[FORKS[fork][logical//512]*512+logical%512]=(i*17+(i>>3)+salt)&255
    return bytes(output)
def fork_digest(logical):
    digest=0
    for sectors,length in zip(FORKS,LENGTHS):
        data=b''.join(logical[lba*512:(lba+1)*512] for lba in sectors)[:length]
        for byte in data:digest=(((digest<<5)|(digest>>27))&0xffffffff)^byte
    return digest


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('cc','m68k-bin','tool-root','emulator','model-contract','output'):
        p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--emulator-sha256',required=True)
    a=p.parse_args()
    cc,exe,tool,contract=(x.resolve(strict=True) for x in (a.cc,a.emulator,a.tool_root,a.model_contract))
    out=a.output.resolve()
    if out==ROOT or ROOT in out.parents:raise ValueError('new output directory must remain outside repository')
    if sha(exe)!=a.emulator_sha256.lower():raise ValueError('unadmitted executable identity')
    native=module('mfs_write_native',ROOT/'scripts/m68k_windows_toolchain.py')
    m68k,identity=native.inspect(a.m68k_bin)
    loader=module('mfs_write_loader',ROOT/'scripts/pc286_loader.py')
    fixture=module('mfs_write_original',ROOT/'scripts/Test-MfsVolumeMedia.py')
    original,_=fixture.original_media()
    expected={style:logical_result(original,style) for style in (1,2,3)}
    lock_path=ROOT/'scripts/pc286-toolchain-lock.json';lock=json.loads(lock_path.read_text(encoding='utf-8'))
    tools={'host_cc':cc,'emulator':exe,'python':Path(sys.executable).resolve(),'model_contract':contract}
    tools.update({'m68k:'+name:path for name,path in m68k.items()})
    for row in lock['selected_members']:
        path=tool/row['path']
        if path.stat().st_size!=row['bytes'] or sha(path)!=row['sha256']:raise ValueError('C16 tool/header mismatch')
        tools['pc286:'+row['path']]=path
    sources=[*COMMON,*(x.with_suffix('.h') for x in COMMON),WRITE,WRITE.with_suffix('.h'),
        ROOT/'core/storage/block-write.h',SHARED,SHARED.with_suffix('.h'),ROOT/'tests/mfs-write.c',
        ROOT/'probes/mac128k/mfs-write.c',ROOT/'probes/mac128k/mfs-write.ld',ROOT/'probes/pc286/boot-mfs-write.c',
        ROOT/'platform/mac128k/iwm-read.c',ROOT/'platform/mac128k/iwm-read.h',ROOT/'platform/mac128k/iwm-capture.S',
        ROOT/'platform/mac128k/iwm-write.c',ROOT/'platform/mac128k/iwm-write.h',ROOT/'platform/mac128k/iwm-send.S',
        ROOT/'platform/pc286/boothush-floppy-read.c',ROOT/'platform/pc286/boothush-floppy-read.h',
        ROOT/'platform/pc286/boothush-floppy-write.c',ROOT/'platform/pc286/boothush-floppy-write.h',
        ROOT/'arch/i286/c16.h',ROOT/'arch/m68k/mac-reset.S',lock_path,Path(__file__).resolve(),
        ROOT/'scripts/m68k_windows_toolchain.py',ROOT/'scripts/pc286_loader.py',ROOT/'scripts/Test-MfsVolumeMedia.py',
        ROOT/'docs/specifications/SPEC-0128-mfs-fork-overwrite.md',ROOT/'docs/specifications/SPEC-0115-mfs-volume-reader.md',
        ROOT/'docs/specifications/SPEC-0125-modeled-mac-sector-write.md',ROOT/'docs/specifications/SPEC-0126-pc286-floppy-sector-write.md']
    before_sources={x.relative_to(ROOT).as_posix():sha(x) for x in sources}
    before_tools={name:sha(path) for name,path in tools.items()}
    out.mkdir(parents=True,exist_ok=False)
    env=os.environ.copy();env['PATH']=str(cc.parent)+os.pathsep+str(m68k['gcc'].parent)+os.pathsep+env.get('PATH','')
    for name in ('WCC','WATCOM','INCLUDE','LIB','WCL','WLINK'):env.pop(name,None)
    commands=[];hosts=[];builds=[];observations=[]
    def run(command,label,allow_failure=False):
        command=[str(x) for x in command];commands.append(command)
        result=subprocess.run(command,cwd=out,env=env,capture_output=True,timeout=60,
                              creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        (out/(label+'.stdout')).write_bytes(result.stdout);(out/(label+'.stderr')).write_bytes(result.stderr)
        if result.returncode and not allow_failure:raise ValueError(label+' failed; see retained output')
        return result
    warnings=['-Wall','-Wextra','-Werror','-Wconversion','-Wsign-conversion']
    for name,flags in (('release',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-sanitize-recover=all'])):
        binary=out/('host-'+name+'.exe')
        run([cc,'-std=c11',*warnings,*flags,*COMMON,WRITE,ROOT/'tests/mfs-write.c','-o',binary],'host-'+name+'-build')
        result=run([binary],'host-'+name+'-test').stdout.decode().strip()
        if not re.fullmatch(r'MFS write: [0-9]+ checks passed',result):raise ValueError('missing host result')
        hosts.append({'profile':name,'result':result,'sha256':sha(binary)})
    source=WRITE.read_text(encoding='utf-8')
    definitions={
        'skip-chain':('result=cs_mfsv_fork_validate(volume,entry,fork);','result=CS_MFSV_OK;',0,0xd1),
        'skip-volume-lock':('volume->info.attributes & UINT16_C(0x8080)',
                            'volume->info.attributes & UINT16_C(0)',0,0xd5),
        'wrong-fragment':('return (uint32_t)volume->info.alloc_first+delta;',
                          'return (uint32_t)volume->info.alloc_first+(delta&1u);',0,0xd4),
        'overcount-prefix':('publish(progress,completed,lba,CS_MFSW_WRITE,1);return result;',
                            'publish(progress,completed+take,lba,CS_MFSW_WRITE,1);return result;',2,0xd2)}
    mutants={}
    for name,(old,new,mode,marker) in definitions.items():
        if source.count(old)!=1:raise ValueError('mutation anchor changed: '+name)
        path=out/(name+'.c');path.write_text(source.replace(old,new),encoding='utf-8')
        mutants[name]=(path,mode,marker)

    def build_mac(label,mode,core):
        elf,raw,rom=(out/(label+suffix) for suffix in ('.elf','.bin','.rom'))
        run([m68k['gcc'],'-mcpu=68000','-std=c99','-Os',*warnings,'-ffreestanding','-fno-builtin','-nostdlib',
             '-I'+str(WRITE.parent),'-DPROBE_CASE='+str(mode),'-Wl,--build-id=none',
             '-Wl,-T,'+str(ROOT/'probes/mac128k/mfs-write.ld'),ROOT/'arch/m68k/mac-reset.S',
             *COMMON,core,SHARED,ROOT/'platform/mac128k/iwm-read.c',ROOT/'platform/mac128k/iwm-capture.S',
             ROOT/'platform/mac128k/iwm-write.c',ROOT/'platform/mac128k/iwm-send.S',ROOT/'probes/mac128k/mfs-write.c',
             '-o',elf],label+'-build')
        if run([m68k['nm'],'-u',elf],label+'-imports').stdout.strip():raise ValueError('Mac runtime import')
        symbols=run([m68k['nm'],'-n',elf],label+'-symbols').stdout.decode()
        def symbol(name):
            match=re.search(r'^([0-9a-fA-F]+) [A-Za-z] '+name+r'\s*$',symbols,re.M)
            if not match:raise ValueError('missing Mac symbol '+name)
            return int(match.group(1),16)
        run([m68k['objcopy'],'-O','binary',elf,raw],label+'-binary')
        payload=raw.read_bytes()
        if not 260<=len(payload)<=65536 or symbol('__bss_end')>0x3000:raise ValueError('Mac probe budget')
        rom.write_bytes(payload+bytes([255])*(65536-len(payload)))
        build={'lane':'mac128k','label':label,'mode':mode,'payload_bytes':len(payload),'rom_sha256':sha(rom),
               'bss_end':symbol('__bss_end'),'diag':symbol('cs_mac_diag'),'report':symbol('cs_mfs_write_report'),
               'runtime_imports':[]}
        builds.append(build);return rom,build

    def build_pc(label,mode,core):
        objects=[]
        flags=['-zq','-2','-ms','-zl','-s','-os','-zastd=c99','-we','-dCS_C16_REQUIRE_16BIT',
               '-dPROBE_CASE='+str(mode),'-i='+str(tool/'h'),'-i='+str(WRITE.parent)]
        for i,path in enumerate([*COMMON,core,SHARED,ROOT/'platform/pc286/boothush-floppy-read.c',
                                 ROOT/'platform/pc286/boothush-floppy-write.c',ROOT/'probes/pc286/boot-mfs-write.c']):
            obj=out/(label+'-'+str(i)+'.obj')
            run([tool/'binnt64/wcc.exe',*flags,'-fo='+str(obj),path],label+'-compile-'+str(i))
            meta=run([tool/'binnt64/dmpobj.exe',obj],label+'-object-'+str(i)).stdout
            if b'80286' not in meta or b'USE16' not in meta:raise ValueError('non-native C16 object')
            objects.append(obj)
        binary,mapping=out/(label+'.bin'),out/(label+'.map')
        run([tool/'binnt64/wlink.exe','format','raw','bin','option','start=start_','option','nodefaultlibs',
             'option','map='+str(mapping),'name',binary,'file',','.join(str(x) for x in objects)],label+'-link')
        text=mapping.read_text(encoding='utf-8',errors='replace')
        entry=re.search(r'Entry point address:\s*([0-9a-fA-F]+)',text)
        bss=re.search(r'^_BSS\s+BSS\s+DGROUP\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)',text,re.M)
        report=re.search(r'^\s*([0-9a-fA-F]+)[+*]?\s+_cs_mfs_write_report\s*$',text,re.M)
        if not entry or not bss or not report or '.lib' in text.lower():raise ValueError('missing native map or runtime library')
        payload=binary.read_bytes();bss_start,bss_bytes=(int(v,16) for v in bss.groups())
        if len(payload)+64>16384:raise ValueError('full C16 payload/report inspection exceeds budget')
        base=loader.floppy(payload,int(entry.group(1),16),bss_end=bss_start+bss_bytes,poison_bss=True,verify_payload=True)
        build={'lane':'pc286','label':label,'mode':mode,'payload_bytes':len(payload),'payload_sha256':sha(binary),
               'bss_start':bss_start,'bss_bytes':bss_bytes,'report':0x10000+int(report.group(1),16),'runtime_imports':[]}
        builds.append(build);return base,payload,build

    def guest(lane,label,artifact,disk,mode,marker=0xba,destination=None,expected_bytes=None):
        build=artifact[-1]
        cmd=[exe,'--machine',lane,'--floppy',disk,'--cycles','800000000','--timeout-ms','30000']
        if lane=='mac128k':
            cmd+=['--rom',artifact[0],'--until',str(build['diag'])+':4:'+str(marker),
                  '--inspect',str(build['diag'])+':32','--inspect',str(build['report'])+':64','--mac-floppy-state','1']
            inputs=[disk,artifact[0]]
        else:
            cmd+=['--until-post',str(marker)]
            for offset in range(0,len(artifact[1]),4096):cmd+=['--inspect',str(0x10000+offset)+':'+str(len(artifact[1][offset:offset+4096]))]
            cmd+=['--inspect',str(build['report'])+':64'];inputs=[disk]
        if destination:cmd+=['--floppy-out',destination]
        before={str(path):sha(path) for path in inputs}
        result=json.loads(run(cmd,label).stdout)
        memories=result.pop('memory_ranges',None)
        if result.get('status')!='condition_met' or memories is None:raise ValueError(label+' did not reach exact stop')
        if lane=='mac128k':
            if len(memories)!=2 or memories[0]['address']!=build['diag'] or memories[1]['address']!=build['report']:
                raise ValueError('Mac diagnostic/report addresses differ')
            diag=list(struct.unpack('>8I',bytes.fromhex(memories[0]['bytes'])))
            if diag[0]!=marker or any(diag[1:]):raise ValueError('Mac fault or diagnostic mismatch')
            words=list(struct.unpack('>16I',bytes.fromhex(memories[1]['bytes'])))
        else:
            if result.get('post')!=marker or result.get('cs')!=0x1000:raise ValueError('286 wrong stop/segment')
            if len(memories)!=(len(artifact[1])+4095)//4096+1 or memories[-1]['address']!=build['report']:
                raise ValueError('286 incomplete payload/report observations')
            for memory,offset in zip(memories[:-1],range(0,len(artifact[1]),4096)):
                if memory['address']!=0x10000+offset or bytes.fromhex(memory['bytes'])!=artifact[1][offset:offset+4096]:
                    raise ValueError('286 full loaded payload differs')
            words=list(struct.unpack('<16I',bytes.fromhex(memories[-1]['bytes'])))
        if words[0]!=0x4d465357 or words[1]!=mode or words[15]!=marker:raise ValueError('probe report differs')
        if marker==0xba:
            style=2 if mode in (2,4) else 3 if mode in (3,5) else 1
            if words[2]!=4 or words[10]!=2967 or words[11]!=fork_digest(expected[style]) or words[13]:
                raise ValueError('full fork oracle differs')
            if mode==0 and words[3:10]!=[0,512,0,0,0,9,6]:raise ValueError('successful write accounting differs')
            if mode==2 and words[3:10]!=[8,12,17,2,1,2,1]:raise ValueError('uncertain failed-sector prefix differs')
            if mode==3 and words[3:10]!=[8,1036,23,1,0,3,2]:raise ValueError('late read-failure prefix differs')
        if before!={str(path):sha(path) for path in inputs}:raise ValueError('source input changed')
        observation={'lane':lane,'case':label,'mode':mode,'marker':marker,'report':words,'result':result,
                     'input_sha256':before,'inputs_unchanged':True}
        if destination:
            if expected_bytes is not None and destination.read_bytes()!=expected_bytes:raise ValueError('whole snapshot differs')
            observation['snapshot_sha256']=sha(destination);observation['whole_snapshot_matches']=expected_bytes is not None
        observations.append(observation);print(lane+' '+label+' passed',flush=True)

    # Each fresh 286 boot has its own original loader outside the logical MFS
    # region. The exported logical400KiB region is copied without transformation.
    for lane in ('pc286','mac128k'):
        cache={}
        for mode in range(6):
            label=lane+'-mode-'+str(mode)
            cache[mode]=build_pc(label,mode,WRITE) if lane=='pc286' else build_mac(label,mode,WRITE)
        for mode,style,reboot in ((0,1,1),(2,2,4),(3,3,5)):
            formats=('raw','dc42') if lane=='mac128k' and mode==0 else ('raw',)
            for format in formats:
                label=lane+'-'+format+'-write-'+str(mode);disk=out/(label+'.img');snapshot=out/(label+'-out.img')
                if lane=='pc286':
                    image=bytearray(cache[mode][0]);image[60*512:860*512]=original
                    desired=bytearray(image);desired[60*512:860*512]=expected[style]
                else:
                    image=original if format=='raw' else dc42(original)
                    desired=container_result(image,expected[style])
                disk.write_bytes(image)
                guest(lane,label,cache[mode],disk,mode,destination=snapshot,expected_bytes=desired)
                if lane=='pc286':
                    fresh=bytearray(cache[reboot][0]);fresh[60*512:860*512]=snapshot.read_bytes()[60*512:860*512]
                    read_disk=out/(label+'-fresh.img');read_disk.write_bytes(fresh)
                else:read_disk=snapshot
                guest(lane,label+'-fresh',cache[reboot],read_disk,reboot)
        for name,(core,mode,marker) in mutants.items():
            label=lane+'-'+name
            artifact=build_pc(label,mode,core) if lane=='pc286' else build_mac(label,mode,core)
            disk=out/(label+'.img');snapshot=out/(label+'-out.img')
            if lane=='pc286':
                image=bytearray(artifact[0]);image[60*512:860*512]=original
            else:image=original
            disk.write_bytes(image)
            guest(lane,label,artifact,disk,mode,marker,destination=snapshot)
    if before_sources!={x.relative_to(ROOT).as_posix():sha(x) for x in sources}:raise ValueError('source changed during qualification')
    if before_tools!={name:sha(path) for name,path in tools.items()}:raise ValueError('tool/model contract changed')
    if native.inspect(a.m68k_bin)[1]!=identity:raise ValueError('m68k identity changed')
    report={'status':'pass','source_sha256':before_sources,'tool_sha256':before_tools,'m68k_identity':identity,
            'host_tests':hosts,'builds':builds,'observed':observations,'commands':commands,'inputs_unchanged':True,
            'original_logical_sha256':hashlib.sha256(original).hexdigest(),
            'expected_logical_sha256':{str(k):hashlib.sha256(v).hexdigest() for k,v in expected.items()},
            'limits':'Original disposable fixtures only; existing logical fork overwrite, no allocation or metadata mutation. '
                     'Per-sector commits, explicit uncertain failed sector, no whole-fork rollback/durable-flush promise. '
                     '286 fresh boot replaces only original test loader outside preserved logical MFS region. '
                     'Modeled native providers, no owner image, File Manager/startup or physical hardware claim.'}
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({'status':'pass','native_runs':len(observations),'report':str(out/'report.json')}))


if __name__=='__main__':
    try:main()
    except (OSError,ValueError,KeyError,struct.error,subprocess.TimeoutExpired) as error:
        print('MFS write verification failed: '+str(error),file=sys.stderr);sys.exit(1)
