# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Original bounded guest-session standalone closure/encoding audit; no execution."""
import argparse,hashlib,json,os,re,sys
from pathlib import Path
from importlib.util import spec_from_file_location,module_from_spec

def main():
    p=argparse.ArgumentParser();p.add_argument('--tool-root',required=True);p.add_argument('--output',required=True)
    a=p.parse_args();repo=Path(__file__).resolve().parents[1];root=Path(a.tool_root).resolve();out=Path(a.output).resolve()
    out.mkdir(parents=True,exist_ok=False)
    spec=spec_from_file_location('guest_audit',repo/'scripts/Verify-GuestM68k.py');h=module_from_spec(spec)
    sys.dont_write_bytecode=True;spec.loader.exec_module(h)
    modules=dict(h.MODULES);modules.pop('apps/memory-probe/probe.c')
    modules['core/cpu/m68k-call.c']={'cs_guest_validate','cs_guest_window','cs_guest_read','cs_m68k_bus_step_bounded'}
    modules['core/cpu/m68k-session.c']={'cs_m68k_call_init','cs_m68k_call_using','cs_m68k_bus_step_bounded'}
    env=os.environ.copy();env['LD_LIBRARY_PATH']=str(root/'usr/lib/x86_64-linux-gnu')
    env['PATH']=str(root/'usr/bin')+os.pathsep+env.get('PATH','');bins=root/'usr/bin'
    cc=bins/'m68k-linux-gnu-gcc-13';nm=bins/'m68k-linux-gnu-nm'
    flags=['-mcpu=68000','-Wa,-m68000','-std=c11','-ffreestanding','-fno-builtin','-fno-pic','-fno-pie','-fno-common',
        '-fno-stack-protector','-fno-tree-loop-distribute-patterns','-fstack-usage','-Wall','-Wextra','-Werror',
        '-Wconversion','-Wsign-conversion','--sysroot='+str(root),'-ffile-prefix-map='+str(repo)+'=.']
    profiles=[]
    for opt in ['-O0','-Os']:
        folder=out/opt[1:];folder.mkdir();objects=[];frames=[]
        for i,(source,allowed) in enumerate(modules.items()):
            obj=folder/f'{i}.o';objects.append(obj)
            h.run([cc,*flags,opt,'-ffile-prefix-map='+str(folder)+'=.','-c',repo/source,'-o',obj],env)
            h.audit(obj,nm,allowed,env)
        combined=folder/'original-guest-sessions.o'
        h.run([bins/'m68k-linux-gnu-ld','-r',*objects,'-o',combined],env);h.audit(combined,nm,set(),env)
        for usage in sorted(folder.glob('*.su')):
            for line in usage.read_text().splitlines():
                fields=line.split('\t')
                if len(fields)!=3 or fields[2] not in ['static','dynamic,bounded'] or int(fields[1])>1024:
                    raise ValueError('Unbounded/oversized frame: '+line)
                frames.append({'function':fields[0].split(':')[-1],'bytes':int(fields[1])})
        profiles.append({'optimization':opt,'objects':len(objects),'combined_sha256':hashlib.sha256(combined.read_bytes()).hexdigest(),'stack_frames':frames})
    asm=out/'fixture.o';raw=out/'fixture.bin'
    h.run([cc,'-mcpu=68000','-Wa,-m68000','-c',repo/'apps/guest-session-probe/program.S','-o',asm],env)
    h.run([bins/'m68k-linux-gnu-objcopy','-O','binary','--only-section=.text',asm,raw],env)
    text=(repo/'apps/guest-session-probe/program.h').read_text(encoding='utf-8').split('#define CS_GUEST_SESSION_PROGRAM_BYTES',1)[1].split('#endif',1)[0]
    expected=bytes(int(x,0) for x in re.findall(r'0x[0-9A-Fa-f]+|\b[0-9]+\b',text))
    if len(expected)!=18 or raw.read_bytes()!=expected: raise ValueError('Original guest-session assembler fixture differs')
    footprint=out/'footprint.c';footprint.write_text('#include "core/cpu/m68k-session.h"\nconst uint32_t cs_session_storage[]={sizeof(cs_m68k_session),sizeof(cs_m68k_session_report),sizeof(cs_m68k_session_request)};\n',encoding='utf-8')
    h.run([cc,*flags,'-Os','-I'+str(repo),'-c',footprint,'-o',out/'footprint.o'],env)
    h.run([bins/'m68k-linux-gnu-objcopy','-O','binary','--only-section=.rodata',out/'footprint.o',out/'footprint.bin'],env)
    storage=(out/'footprint.bin').read_bytes();assert len(storage)==12
    inputs=[*modules,'core/cpu/m68k-session.h','core/cpu/m68k-call.h','core/cpu/m68k-bus.h','core/cpu/m68k-guard.h','core/cpu/m68k.h','core/memory/guest.h',
        'core/services/traps.h','apps/guest-session-probe/program.h','apps/guest-session-probe/program.S',
        'docs/specifications/SPEC-0028-bounded-guest-sessions.md','scripts/Verify-GuestSessionsM68k.py','scripts/Verify-GuestM68k.py']
    result={'schema_version':1,'specification':'SPEC-0028','compiler':h.run([cc,'--version'],env).splitlines()[0],
        'object_audits':len(modules)*2,'complete_relocatable_links':2,'assembler_fixture_bytes':len(expected),
        'assembler_fixture_sha256':hashlib.sha256(expected).hexdigest(),'storage_bytes_68000':dict(zip(['session','report','request'],[int.from_bytes(storage[i:i+4],'big') for i in range(0,12,4)])),
        'profiles':profiles,'inputs':{f:hashlib.sha256((repo/f).read_bytes().replace(b'\r\n',b'\n')).hexdigest() for f in inputs},
        'tool_lock_sha256':hashlib.sha256((repo/'scripts/m68k-toolchain-lock.json').read_bytes().replace(b'\r\n',b'\n')).hexdigest(),
        'native_executed':False,'rom_integration':False}
    (out/'manifest.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
    print('SPEC-0028 original 68000 guest-session linkage/encoding PASS; no execution claim.')
if __name__=='__main__': main()
