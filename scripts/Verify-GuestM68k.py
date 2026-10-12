# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Standalone original 68000 objects/encoding check; no emulator or ROM input."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess

MODULES={
    'core/memory/guest.c':set(),
    'core/cpu/m68k-bus.c':{'cs_guest_validate','cs_guest_read','cs_guest_window','cs_m68k_aline'},
    'apps/memory-probe/probe.c':{'cs_m68k_bus_step'},
    'core/cpu/m68k.c':{'cs_trap_call'},
    'core/services/traps.c':set(),
}

def run(args,env):
    p=subprocess.run([str(a) for a in args],env=env,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    if p.returncode: raise RuntimeError('Command failed: '+' '.join(str(a) for a in args)+'\n'+p.stdout)
    return p.stdout

def audit(obj,nm,allowed,env):
    for line in run([nm,'--undefined-only','--format=posix',obj],env).splitlines():
        fields=line.split()
        if len(fields)<2 or fields[1]!='U' or fields[0] not in allowed:
            raise ValueError('Unexpected import: '+line)
    for line in run([nm,'--defined-only','--format=posix',obj],env).splitlines():
        fields=line.split()
        if len(fields)<2 or fields[1] in 'BbCcDdGgSsVv': raise ValueError('Mutable global: '+line)

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--tool-root',required=True);parser.add_argument('--output',required=True)
    args=parser.parse_args();repo=Path(__file__).resolve().parents[1];root=Path(args.tool_root).resolve()
    out=Path(args.output).resolve();out.mkdir(parents=True,exist_ok=False)
    env=os.environ.copy();env['LD_LIBRARY_PATH']=str(root/'usr/lib/x86_64-linux-gnu')
    env['PATH']=str(root/'usr/bin')+os.pathsep+env.get('PATH','')
    bins=root/'usr/bin';cc=bins/'m68k-linux-gnu-gcc-13';nm=bins/'m68k-linux-gnu-nm'
    profiles=[]
    for opt in ('-O0','-Os'):
        folder=out/opt[1:];folder.mkdir();objects=[];frames=[]
        for index,(source,allowed) in enumerate(MODULES.items()):
            obj=folder/(str(index)+'.o');objects.append(obj)
            run([cc,'-mcpu=68000','-Wa,-m68000',opt,'-std=c11','-ffreestanding','-fno-builtin','-fno-pic','-fno-pie',
                 '-fno-common','-fno-stack-protector','-fno-tree-loop-distribute-patterns','-fstack-usage',
                 '-Wall','-Wextra','-Werror','-Wconversion','-Wsign-conversion','--sysroot='+str(root),
                 '-ffile-prefix-map='+str(repo)+'=.','-ffile-prefix-map='+str(folder)+'=.',
                 '-c',repo/source,'-o',obj],env)
            audit(obj,nm,allowed,env)
        combined=folder/'original-core.o'
        run([bins/'m68k-linux-gnu-ld','-r',*objects,'-o',combined],env);audit(combined,nm,set(),env)
        for usage in folder.glob('*.su'):
            for line in usage.read_text().splitlines():
                fields=line.split('\t')
                if len(fields)!=3 or fields[2] not in ('static','dynamic,bounded') or int(fields[1])>1024:
                    raise ValueError('Unbounded/oversized function frame: '+line)
                frames.append({'function':fields[0].split(':')[-1],'bytes':int(fields[1])})
        profiles.append({'optimization':opt,'objects':len(objects),'combined_sha256':hashlib.sha256(combined.read_bytes()).hexdigest(),
                         'stack_frames':frames})
    asm=out/'fixture.o';raw=out/'fixture.bin'
    run([cc,'-mcpu=68000','-Wa,-m68000','-c',repo/'apps/memory-probe/program.S','-o',asm],env)
    run([bins/'m68k-linux-gnu-objcopy','-O','binary','--only-section=.text',asm,raw],env)
    text=(repo/'apps/memory-probe/program.h').read_text().split('#define CS_MEMORY_PROGRAM_BYTES',1)[1].split('#endif',1)[0]
    expected=bytes(int(token,0) for token in re.findall(r'0x[0-9A-Fa-f]+|\b[0-9]+\b',text))
    if len(expected)!=36 or raw.read_bytes()!=expected: raise ValueError('Assembler and C fixture differ')
    inputs=(*MODULES,'core/memory/guest.h','core/cpu/m68k-bus.h','core/cpu/m68k.h','core/services/traps.h','apps/memory-probe/probe.h',
            'apps/memory-probe/program.h','apps/memory-probe/program.S','docs/specifications/SPEC-0022-guest-memory-execution.md')
    result={'schema_version':1,'specification':'SPEC-0022','compiler':run([cc,'--version'],env).splitlines()[0],
            'object_audits':10,'complete_relocatable_links':2,'assembler_fixture_bytes':36,
            'assembler_fixture_sha256':hashlib.sha256(expected).hexdigest(),'profiles':profiles,
            'inputs':{f:hashlib.sha256((repo/f).read_bytes().replace(b'\r\n',b'\n')).hexdigest() for f in inputs},
            'tool_lock_sha256':hashlib.sha256((repo/'scripts/m68k-toolchain-lock.json').read_bytes().replace(b'\r\n',b'\n')).hexdigest(),
            'native_executed':False,'rom_integration':False}
    (out/'manifest.json').write_text(json.dumps(result,indent=2)+'\n')
    print('SPEC-0022 original 68000 object/encoding audit PASS; no CPU execution claim.')

if __name__=='__main__':main()
