#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Original Mac sector write/read/export/restart qualification; no external media."""
import argparse, hashlib, importlib.util, json, os, re, struct, subprocess, sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
LBAS=(0,191,192,367,368,527,528,671,672,799,2)
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path)
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value
def checksum(data):
    total=0
    for offset in range(0,len(data),2):
        total=(total+int.from_bytes(data[offset:offset+2],'big'))&0xffffffff
        total=(total>>1)|((total&1)<<31)
    return total
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('cc','m68k-bin','emulator','output'):parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--emulator-sha256',required=True)
    args=parser.parse_args();out=args.output.resolve();exe=args.emulator.resolve(strict=True)
    if out==ROOT or ROOT in out.parents:raise ValueError('use fresh external output directory')
    if sha(exe)!=args.emulator_sha256:raise ValueError('explicit emulator identity mismatch')
    out.mkdir(parents=True,exist_ok=False)
    sources=[ROOT/'platform/mac128k'/p for p in ('iwm-read.c','iwm-read.h','iwm-capture.S','iwm-write.c','iwm-write.h','iwm-send.S')]
    sources += [ROOT/p for p in ('core/storage/block-read.h','tests/mac-floppy-write.c','probes/mac128k/floppy-write.c','arch/m68k/mac-reset.S','arch/m68k/mac-rom.ld','scripts/m68k_windows_toolchain.py','scripts/Test-MacFloppyRead.py','docs/specifications/SPEC-0125-modeled-mac-sector-write.md','provenance/records/SRC-0619-modeled-mac-sector-write.md')]+[Path(__file__).resolve()]
    source_hashes={str(p.relative_to(ROOT)):sha(p) for p in sources}
    native=module('iwm_write_tools',ROOT/'scripts/m68k_windows_toolchain.py');tools,identity=native.inspect(args.m68k_bin)
    fixture=module('iwm_write_fixture',ROOT/'scripts/Test-MacFloppyRead.py')
    tool_hashes={'emulator':sha(exe),'host_cc':sha(args.cc),'python':sha(Path(sys.executable))}
    tool_hashes.update({name:sha(path) for name,path in tools.items()})
    env=os.environ.copy();env['PATH']=str(Path(tools['gcc']).parent)+os.pathsep+str(args.cc.parent)+os.pathsep+env.get('PATH','')
    commands=[];results=[];builds=[];host=[]
    def run(command,label,allow_failure=False):
        command=[str(x) for x in command];commands.append(command)
        p=subprocess.run(command,capture_output=True,env=env,timeout=60,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        (out/(label+'.stdout')).write_bytes(p.stdout);(out/(label+'.stderr')).write_bytes(p.stderr)
        if p.returncode and not allow_failure:raise ValueError(label+' failed; retained output')
        return p
    for name,flags in [('release',['-O2']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-sanitize-recover=all'])]:
        binary=out/('host-'+name+'.exe')
        run([args.cc,'-std=c11','-Wall','-Wextra','-Werror','-Wconversion','-Wsign-conversion',*flags,
             ROOT/'tests/mac-floppy-write.c',ROOT/'platform/mac128k/iwm-write.c',ROOT/'platform/mac128k/iwm-read.c','-o',binary],name+'-build')
        p=run([binary],name+'-host');host.append({'profile':name,'output':p.stdout.decode().strip(),'sha256':sha(binary)})
    def build(mode,tags):
        name=f'probe-{mode}-{int(tags)}';elf=out/(name+'.elf');raw=out/(name+'.bin');rom=out/(name+'.rom')
        run([tools['gcc'],'-mcpu=68000','-std=c99','-Os','-Wall','-Wextra','-Werror','-Wconversion','-Wsign-conversion','-ffreestanding','-fno-builtin','-nostdlib',f'-DPROBE_CASE={mode}',f'-DEXPECT_TAGS={int(tags)}','-Wl,--build-id=none','-Wl,-T,'+str(ROOT/'arch/m68k/mac-rom.ld'),ROOT/'arch/m68k/mac-reset.S',ROOT/'platform/mac128k/iwm-read.c',ROOT/'platform/mac128k/iwm-capture.S',ROOT/'platform/mac128k/iwm-write.c',ROOT/'platform/mac128k/iwm-send.S',ROOT/'probes/mac128k/floppy-write.c','-o',elf],name+'-build')
        if run([tools['nm'],'-u',elf],name+'-imports').stdout.strip():raise ValueError('runtime imports')
        syms=run([tools['nm'],'-n',elf],name+'-symbols').stdout.decode()
        address=int(re.search(r'^([0-9a-fA-F]+) [BbDd] cs_mac_diag\s*$',syms,re.M).group(1),16)
        run([tools['objcopy'],'-O','binary',elf,raw],name+'-objcopy');payload=raw.read_bytes()
        if not 260<=len(payload)<=65536:raise ValueError('ROM bounds')
        rom.write_bytes(payload+bytes([255])*(65536-len(payload)))
        builds.append({'case':name,'payload_bytes':len(payload),'rom_sha256':sha(rom),'symbols':syms})
        return rom,address
    def guest(rom,address,disk,label,destination=None,refused_export=False):
        command=[exe,'--machine','mac128k','--rom',rom,'--floppy',disk,'--cycles','800000000','--timeout-ms','30000','--until',f'{address}:4:0xb8','--inspect',f'{address}:32','--mac-floppy-state','1','--mac-cpu-state','1']
        if destination:command+=['--floppy-out',destination]
        before={str(p):sha(p) for p in (rom,disk)}
        p=run(command,label,allow_failure=refused_export);result=json.loads(p.stdout)
        words=list(struct.unpack('>8I',bytes.fromhex(result['memory']['bytes'])))
        if words[0]!=0xb8 or words[7]!=0:raise ValueError(label+' unexpected guest result')
        if refused_export:
            state=result.get('mac_floppy_write',{})
            if p.returncode!=2 or result.get('status')!='error' or destination.exists() or state.get('committed_sectors')!=0 or state.get('rejected_records')!=1:
                raise ValueError(label+' missing atomic refusal and export suppression')
        elif result.get('status')!='condition_met':raise ValueError(label+' unexpected run status')
        if before!={str(p):sha(Path(p)) for p in before}:raise ValueError('input changed')
        results.append({'case':label,'diag':words,'inputs':before,'result':result})
        return result
    raw,dc42=fixture.original_media()
    for form,content in [('raw',raw),('dc42',dc42)]:
        disk=out/('original.'+form);disk.write_bytes(content)
        rom,address=build(0,form=='dc42');dest=out/('saved.'+form)
        guest(rom,address,disk,form+'-write-read',dest)
        data=bytearray(raw)
        for lba in LBAS:data[lba*512:(lba+1)*512]=bytes((lba*19+i*13+(i>>2)+0x5b)&255 for i in range(512))
        if form=='raw':expected=bytes(data)
        else:
            expected=bytearray(content);expected[84:84+409600]=data
            struct.pack_into('>I',expected,72,checksum(data));expected=bytes(expected)
        if dest.read_bytes()!=expected:raise ValueError(form+' full export mismatch')
        readrom,readaddress=build(1,form=='dc42');guest(readrom,readaddress,dest,form+'-fresh-read')
        ro,roaddress=build(2,form=='dc42');guest(ro,roaddress,disk,form+'-protected-refusal')
        results[-1]['original_unchanged']=sha(disk)==hashlib.sha256(content).hexdigest()
        results.append({'case':form+'-full-export','bytes':len(expected),'sha256':sha(dest),'all_other_sectors_and_tags_preserved':True})
        bad,badaddress=build(3,form=='dc42')
        guest(bad,badaddress,disk,form+'-bad-checksum',out/('rejected.'+form),refused_export=True)
    _,after=native.inspect(args.m68k_bin)
    if identity!=after or source_hashes!={str(p.relative_to(ROOT)):sha(p) for p in sources}:raise ValueError('sources/tools changed')
    final_tools={'emulator':sha(exe),'host_cc':sha(args.cc),'python':sha(Path(sys.executable))}
    final_tools.update({name:sha(path) for name,path in tools.items()})
    if final_tools!=tool_hashes:raise ValueError('tool binaries changed')
    report={'status':'pass','source_sha256':source_hashes,'tool_sha256':tool_hashes,'tools':identity,'host':host,'builds':builds,'cases':results,'commands':commands,'inputs_unchanged':True,'limits':'Original modeled sector writes only; no MFS mutation, protected media, physical drive or System startup.'}
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({'status':'pass','native_runs':8,'report':str(out/'report.json')}))
if __name__=='__main__':
    try:main()
    except (OSError,ValueError,KeyError,subprocess.TimeoutExpired) as exc:
        print('Mac writer verification failed: '+str(exc),file=sys.stderr);sys.exit(1)
