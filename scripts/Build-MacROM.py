# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Build/audit our original 68000 ROM; never executes a guest or reads a reference ROM."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import zlib

BASE=0x400000
SEAL=256
SIZE=65536
SOURCES=('arch/m68k/mac-reset.S','arch/m68k/runtime.c','platform/mac128k/loop.c',
         'platform/mac128k/entry.c','apps/rom-scene/scene.c','core/graphics/surface.c',
         'core/services/traps.c','apps/service-probe/probe.c','core/cpu/m68k.c',
         'apps/aline-probe/probe.c','arch/m68k/aline.S')
HEADERS=('arch/m68k/runtime.h','platform/mac128k/rom.h','core/graphics/surface.h',
         'arch/m68k/mac-rom.ld','docs/specifications/SPEC-0018-mac128k-rom-startup.md',
         'core/services/traps.h','apps/service-probe/probe.h','docs/specifications/SPEC-0020-shared-trap-address-services.md',
         'core/cpu/m68k.h','apps/aline-probe/probe.h','apps/aline-probe/program.h','docs/specifications/SPEC-0021-aline-execution.md')

def run(args,env):
    p=subprocess.run([str(a) for a in args],env=env,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    if p.returncode:
        raise RuntimeError('Command failed: '+' '.join(str(a) for a in args)+'\n'+p.stdout)
    return p.stdout

def crc(image):
    b=bytearray(image); b[SEAL:SEAL+4]=bytes(4)
    return zlib.crc32(b)

def symbols(elf):
    b=elf.read_bytes()
    if len(b)<52 or b[:7]!=b'\x7fELF\x01\x02\x01': raise ValueError('Not big-endian ELF32')
    h=struct.unpack_from('>HHIIIIIHHHHHH',b,16)
    kind,machine,version,entry,phoff,shoff,flags,ehsize,phsize,phnum,shsize,shnum,shstr=h
    if kind!=2 or machine!=4 or version!=1 or ehsize!=52 or shsize!=40: raise ValueError('ELF identity')
    if shnum==0 or shoff+shnum*40>len(b) or shstr>=shnum: raise ValueError('ELF section bounds')
    sections=[struct.unpack_from('>IIIIIIIIII',b,shoff+i*40) for i in range(shnum)]
    result={}
    for s in sections:
        _,typ,_,_,off,length,link,_,_,entsize=s
        if typ!=2: continue
        if entsize!=16 or length%16 or off+length>len(b) or link>=shnum: raise ValueError('Symbol bounds')
        stringsec=sections[link]; st=b[stringsec[4]:stringsec[4]+stringsec[5]]
        for pos in range(off,off+length,16):
            name,value,_,info,_,index=struct.unpack_from('>IIIBBH',b,pos)
            if name>=len(st): raise ValueError('Symbol name bounds')
            end=st.find(b'\0',name)
            if end<0: raise ValueError('Symbol terminator')
            label=st[name:end].decode('ascii')
            if label:
                if index==0: raise ValueError('Undefined symbol '+label)
                result[label]=value
    if entry!=result.get('cs_mac_reset') or not BASE+260<=entry<result.get('__rom_code_end',0): raise ValueError('Entry')
    if not 0xB00<=result['__data_start']<=result['__data_end']<=result['__bss_start']<=result['__bss_end']<=0x2000: raise ValueError('RAM span')
    if not BASE+260<=result['__rom_end']<=BASE+SIZE: raise ValueError('ROM span')
    return result

def audit_image(b,syms):
    if len(b)!=SIZE: raise ValueError('Image size')
    if struct.unpack_from('>II',b)!= (0x4000,syms['cs_mac_reset']): raise ValueError('Reset vectors')
    for i in range(2,64):
        expected=syms['cs_mac_probe_handler'] if i==32 else syms['cs_mac_line_a_handler'] if i==10 else syms['cs_mac_fault']
        if struct.unpack_from('>I',b,i*4)[0]!=expected: raise ValueError('Exception vector')
    if struct.unpack_from('>I',b,SEAL)[0]!=crc(b): raise ValueError('Image seal')
    start=syms['cs_mac_aline_program']-BASE
    if syms['cs_mac_aline_program_end']-BASE!=start+12 or b[start:start+12]!=bytes.fromhex('7046a1467020a047a1464e75'):
        raise ValueError('Original A-line fixture')
    handler=syms['cs_mac_line_a_handler']-BASE
    bridge=bytes.fromhex('9efc004848d77fff41ef004e2f48003c2f6f004a00403f6f004800442f0f4eb9')
    bridge+=struct.pack('>I',syms['cs_mac_line_a'])
    bridge+=bytes.fromhex('588f3f6f004400482f6f0040004a4cd77fffdefc00484e73')
    if not 260<=handler<start or start-handler!=len(bridge) or b[handler:start]!=bridge:
        raise ValueError('Native A-line bridge')

def build(repo,tools,out,opt,env):
    out.mkdir()
    bins=tools/'usr/bin'
    cc=bins/'m68k-linux-gnu-gcc-13'
    flags=['-mcpu=68000','-Wa,-m68000',opt,'-std=c11','-ffreestanding','-fno-builtin',
           '-fno-pic','-fno-pie','-fno-common','-fno-stack-protector','-fno-unwind-tables',
           '-fno-asynchronous-unwind-tables','-fno-tree-loop-distribute-patterns','-fstack-usage',
           '-Wall','-Wextra','-Werror','-Wconversion','-Wsign-conversion',
           '--sysroot='+str(tools),'-ffile-prefix-map='+str(repo)+'=.',
           '-ffile-prefix-map='+str(out)+'=.']
    objects=[]
    for i,source in enumerate(SOURCES):
        obj=out/(str(i)+'.o'); objects.append(obj)
        run([cc,*flags,'-c',repo/source,'-o',obj],env)
    elf=out/'classick-rom.elf'
    ldflags=['-nostdlib','-no-pie','-Wl,--build-id=none','-Wl,-T,'+str(repo/'arch/m68k/mac-rom.ld')]
    run([cc,'-mcpu=68000',*ldflags,'-Wl,-Map,'+str(out/'classick-rom.map'),*objects,'-o',elf],env)
    syms=symbols(elf)
    raw=out/'classick-rom.raw'
    run([bins/'m68k-linux-gnu-objcopy','-O','binary',elf,raw],env)
    b=bytearray(raw.read_bytes())
    if len(b)>SIZE: raise ValueError('Raw image oversized')
    b.extend(bytes(SIZE-len(b))); struct.pack_into('>I',b,SEAL,crc(b))
    image=out/'classick-097.rom'; image.write_bytes(b)
    audit_image(b,syms)
    (out/'own-disassembly.txt').write_text(run([bins/'m68k-linux-gnu-objdump','-d',elf],env))
    for offset in (0,4,8,40,128,256,1024,SIZE-1):
        bad=bytearray(b); bad[offset]^=1
        try: audit_image(bad,syms)
        except ValueError: pass
        else: raise ValueError('Corruption not rejected')
    for offset in (40,syms['cs_mac_aline_program']-BASE,syms['cs_mac_aline_program']-BASE-1,syms['cs_mac_line_a_handler']-BASE):
        bad=bytearray(b);bad[offset]^=1;struct.pack_into('>I',bad,SEAL,crc(bad))
        try: audit_image(bad,syms)
        except ValueError: pass
        else: raise ValueError('Resealed A-line mutation not rejected')
    for removed in objects:
        p=subprocess.run([str(cc),'-mcpu=68000',*ldflags,*[str(x) for x in objects if x!=removed],
                          '-o',str(out/'omitted.elf')],env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
        if p.returncode==0: raise ValueError('Omitted object accepted: '+removed.name)
        (out/('omitted-'+removed.stem+'.txt')).write_text(p.stdout)
    stack=[]
    for f in out.glob('*.su'):
        for line in f.read_text().splitlines():
            fields=line.split('\t')
            if len(fields)!=3 or fields[2] not in ('static','dynamic,bounded') or int(fields[1])>1024: raise ValueError('Stack profile: '+line)
            stack.append(dict(function=fields[0].split(':')[-1],bytes=int(fields[1])))
    # Covers own assembly save/argument/return/exception frames as well as margin.
    if not stack or sum(x['bytes'] for x in stack)+192>8192: raise ValueError('Stack reserve')
    return dict(optimization=opt,image_size=len(b),image_sha256=hashlib.sha256(b).hexdigest(),
                image_crc32=f'{crc(b):08x}',rom_used=syms['__rom_end']-BASE,
                ram_data_bss=syms['__bss_end']-0xB00,symbols=syms,stack_frames=stack,
                stack_extra_bytes=192,corruption_controls=8,resealed_aline_controls=4,omitted_object_controls=len(objects))

def main():
    p=argparse.ArgumentParser(); p.add_argument('--tool-root',required=True); p.add_argument('--output',required=True)
    a=p.parse_args(); repo=Path(__file__).resolve().parents[1]; tools=Path(a.tool_root).resolve(); out=Path(a.output).resolve()
    out.mkdir(parents=True,exist_ok=False)
    env=os.environ.copy(); env['LD_LIBRARY_PATH']=str(tools/'usr/lib/x86_64-linux-gnu')
    env['PATH']=str(tools/'usr/bin')+os.pathsep+env.get('PATH','')
    manifests=[]
    for opt in ('-Os','-O0'):
        one=build(repo,tools,out/(opt[1:]+'-a'),opt,env); two=build(repo,tools,out/(opt[1:]+'-b'),opt,env)
        if one['image_sha256']!=two['image_sha256']: raise ValueError('Image twins differ')
        manifests.append(one)
    inputs={f:hashlib.sha256((repo/f).read_bytes().replace(b'\r\n',b'\n')).hexdigest() for f in (*SOURCES,*HEADERS)}
    result=dict(schema_version=1,specification='SPEC-0018+SPEC-0020+SPEC-0021',compiler=run([tools/'usr/bin/m68k-linux-gnu-gcc-13','--version'],env).splitlines()[0],
                tool_lock_sha256=hashlib.sha256((repo/'scripts/m68k-toolchain-lock.json').read_bytes().replace(b'\r\n',b'\n')).hexdigest(),
                inputs=inputs,profiles=manifests,emulator_run=False)
    (out/'manifest.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(result,indent=2))

if __name__=='__main__': main()
