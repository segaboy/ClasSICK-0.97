# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Own manager standalone 68000 linkage/size audit; no ROM/reference execution."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import sys
from importlib.util import spec_from_file_location,module_from_spec

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--tool-root',required=True);parser.add_argument('--output',required=True)
    args=parser.parse_args();repo=Path(__file__).resolve().parents[1];root=Path(args.tool_root).resolve()
    out=Path(args.output).resolve();out.mkdir(parents=True,exist_ok=False)
    spec=spec_from_file_location('guest_audit',repo/'scripts/Verify-GuestM68k.py')
    helper=module_from_spec(spec);sys.dont_write_bytecode=True;spec.loader.exec_module(helper)
    env=os.environ.copy();env['LD_LIBRARY_PATH']=str(root/'usr/lib/x86_64-linux-gnu')
    env['PATH']=str(root/'usr/bin')+os.pathsep+env.get('PATH','')
    bins=root/'usr/bin';cc=bins/'m68k-linux-gnu-gcc-13';nm=bins/'m68k-linux-gnu-nm'
    modules={'core/memory/guest.c':set(),'core/memory/manager.c':{'cs_guest_validate','cs_guest_window'},
             'personality/system1/memory-registers.c':{'cs_trap_decode','cs_mm_execute'},
             'apps/heap-probe/probe.c':{'cs_mm_register_call','cs_guest_read','cs_guest_write'},
             'core/services/traps.c':set()}
    flags=['-mcpu=68000','-Wa,-m68000','-std=c11','-ffreestanding','-fno-builtin','-fno-pic','-fno-pie',
           '-fno-common','-fno-stack-protector','-fno-tree-loop-distribute-patterns','-fstack-usage',
           '-Wall','-Wextra','-Werror','-Wconversion','-Wsign-conversion','--sysroot='+str(root),
           '-ffile-prefix-map='+str(repo)+'=.']
    profiles=[]
    for opt in ('-O0','-Os'):
        folder=out/opt[1:];folder.mkdir();objects=[];frames=[]
        for index,(source,allowed) in enumerate(modules.items()):
            obj=folder/(str(index)+'.o');objects.append(obj)
            helper.run([cc,*flags,opt,'-ffile-prefix-map='+str(folder)+'=.','-c',repo/source,'-o',obj],env)
            helper.audit(obj,nm,allowed,env)
        combined=folder/'original-manager.o'
        helper.run([bins/'m68k-linux-gnu-ld','-r',*objects,'-o',combined],env);helper.audit(combined,nm,set(),env)
        for usage in sorted(folder.glob('*.su')):
            for line in usage.read_text().splitlines():
                parts=line.split('\t')
                if len(parts)!=3 or parts[2] not in ('static','dynamic,bounded') or int(parts[1])>1024:
                    raise ValueError('Unbounded/oversized manager frame: '+line)
                frames.append({'function':parts[0].split(':')[-1],'bytes':int(parts[1])})
        profiles.append({'optimization':opt,'objects':5,'combined_sha256':hashlib.sha256(combined.read_bytes()).hexdigest(),'stack_frames':frames})
    footprint=out/'footprint.c';obj=out/'footprint.o';raw=out/'footprint.bin'
    footprint.write_text('#include "core/memory/manager.h"\nconst uint32_t cs_mm_footprint[]={sizeof(cs_mm_manager),sizeof(cs_mm_block),CS_MM_CELL_BYTES*2u};\n')
    helper.run([cc,*flags,'-Os','-I'+str(repo),'-c',footprint,'-o',obj],env)
    helper.run([bins/'m68k-linux-gnu-objcopy','-O','binary','--only-section=.rodata',obj,raw],env)
    budget=struct.unpack('>III',raw.read_bytes())
    if budget!=(672,20,256):raise ValueError('Original metadata/cell budget differs')
    inputs=(*modules,'core/memory/guest.h','core/memory/manager.h','core/services/traps.h',
            'personality/system1/memory-registers.h','apps/heap-probe/probe.h','docs/specifications/SPEC-0023-bounded-memory-manager.md',
            'docs/interfaces/system1-routines.json','scripts/Verify-MMM68k.py','scripts/Verify-GuestM68k.py')
    result={'schema_version':1,'specification':'SPEC-0023','compiler':helper.run([cc,'--version'],env).splitlines()[0],
            'object_audits':10,'complete_relocatable_links':2,'metadata_bytes':budget[0],'record_bytes':budget[1],
            'guest_cell_reserve_bytes':budget[2],'profiles':profiles,
            'inputs':{f:hashlib.sha256((repo/f).read_bytes().replace(b'\r\n',b'\n')).hexdigest() for f in inputs},
            'tool_lock_sha256':hashlib.sha256((repo/'scripts/m68k-toolchain-lock.json').read_bytes().replace(b'\r\n',b'\n')).hexdigest(),
            'native_executed':False,'rom_integration':False}
    (out/'manifest.json').write_text(json.dumps(result,indent=2)+'\n')
    print('SPEC-0023 original manager 68000 audit PASS: 672 metadata bytes + 256 guest cell bytes; no execution claim.')

if __name__=='__main__':main()
