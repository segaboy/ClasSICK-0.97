# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Original data-only eager CODE image closure/storage; no guest/ROM binding."""
import argparse,hashlib,json,os,struct,sys
from pathlib import Path
from importlib.util import spec_from_file_location,module_from_spec

def main():
    p=argparse.ArgumentParser();p.add_argument('--tool-root',required=True);p.add_argument('--output',required=True)
    a=p.parse_args();repo=Path(__file__).resolve().parents[1];root=Path(a.tool_root).resolve();out=Path(a.output).resolve()
    out.mkdir(parents=True,exist_ok=False)
    spec=spec_from_file_location('guest_audit',repo/'scripts/Verify-GuestM68k.py');h=module_from_spec(spec)
    sys.dont_write_bytecode=True;spec.loader.exec_module(h)
    modules={'core/memory/guest.c':set(),'core/memory/manager.c':{'cs_guest_validate','cs_guest_window'},
        'core/services/traps.c':set(),'core/memory/lifecycle.c':{'cs_guest_validate','cs_guest_window','cs_mm_execute','cs_trap_get'},
        'formats/resourcefork/reader.c':set(),
        'personality/system1/code-preflight.c':{'cs_guest_window','cs_life_admit','cs_rf_open','cs_rf_find'},
        'personality/system1/resources.c':{'cs_guest_window','cs_life_admit','cs_life_manager_call','cs_rf_open','cs_rf_find'},
        'personality/system1/code-image.c':{'cs_code_check_layout','cs_code_inspect','cs_guest_read','cs_guest_validate','cs_guest_window','cs_life_admit','cs_life_manager_call','cs_res_external_admit'}}
    env=os.environ.copy();env['LD_LIBRARY_PATH']=str(root/'usr/lib/x86_64-linux-gnu')
    env['PATH']=str(root/'usr/bin')+os.pathsep+env.get('PATH','');bins=root/'usr/bin'
    cc=bins/'m68k-linux-gnu-gcc-13';nm=bins/'m68k-linux-gnu-nm'
    flags=['-mcpu=68000','-Wa,-m68000','-std=c11','-ffreestanding','-fno-builtin','-fno-pic','-fno-pie','-fno-common',
        '-fno-stack-protector','-fno-tree-loop-distribute-patterns','-fstack-usage','-Wall','-Wextra','-Werror',
        '-Wconversion','-Wsign-conversion','--sysroot='+str(root),'-ffile-prefix-map='+str(repo)+'=.']
    profiles=[]
    for opt in ('-O0','-Os'):
        folder=out/opt[1:];folder.mkdir();objects=[];frames=[]
        for i,(source,allowed) in enumerate(modules.items()):
            obj=folder/f'{i}.o';objects.append(obj)
            h.run([cc,*flags,opt,'-ffile-prefix-map='+str(folder)+'=.','-c',repo/source,'-o',obj],env)
            h.audit(obj,nm,allowed,env)
            if 'cs_image_test_fault' in h.run([nm,obj],env):raise ValueError('Production exposes fault injection')
        combined=folder/'original-code-image.o'
        h.run([bins/'m68k-linux-gnu-ld','-r',*objects,'-o',combined],env);h.audit(combined,nm,set(),env)
        for usage in sorted(folder.glob('*.su')):
            for line in usage.read_text().splitlines():
                f=line.split('\t')
                if len(f)!=3 or f[2] not in ('static','dynamic,bounded') or int(f[1])>1024:
                    raise ValueError('Unbounded/oversized frame: '+line)
                frames.append({'function':f[0].split(':')[-1],'bytes':int(f[1])})
        profiles.append({'optimization':opt,'objects':len(objects),
            'combined_sha256':hashlib.sha256(combined.read_bytes()).hexdigest(),'stack_frames':frames})
    names=('service','profile','context','request','report','description','workspace','segment','manager','lifecycle_controller','resource_service')
    types=('cs_image_service','cs_image_profile','cs_image_context','cs_image_request','cs_image_report','cs_image_description',
        'cs_image_workspace','cs_image_segment','cs_mm_manager','cs_life_controller','cs_res_service')
    source=out/'footprint.c'
    source.write_text('#include "personality/system1/code-image.h"\nconst uint32_t cs_code_image_storage[]={'+','.join('sizeof('+t+')' for t in types)+'};\n')
    h.run([cc,*flags,'-Os','-I'+str(repo),'-c',source,'-o',out/'footprint.o'],env)
    h.run([bins/'m68k-linux-gnu-objcopy','-O','binary','--only-section=.rodata',out/'footprint.o',out/'footprint.bin'],env)
    sizes=struct.unpack('>11I',(out/'footprint.bin').read_bytes())
    inputs=[*modules,'personality/system1/code-image.h','personality/system1/code-run.h','personality/system1/code-run-internal.h','personality/system1/quickdraw-rects.h','core/cpu/m68k-guard.h','personality/system1/resources.h','personality/system1/code-preflight.h','core/memory/lifecycle.h','core/memory/manager.h','core/memory/guest.h',
        'core/services/traps.h','formats/resourcefork/reader.h','core/cpu/m68k-session.h','core/cpu/m68k-call.h',
        'core/cpu/m68k-bus.h','core/cpu/m68k.h','docs/specifications/SPEC-0033-owned-code-image-lifecycle.md',
        'scripts/Verify-CodeImageM68k.py','scripts/Verify-GuestM68k.py','tests/code-image.c']
    result={'schema_version':1,'specification':'SPEC-0033','compiler':h.run([cc,'--version'],env).splitlines()[0],
        'object_audits':16,'complete_relocatable_links':2,'storage_bytes_68000':dict(zip(names,sizes)),
        'profiles':profiles,'inputs':{f:hashlib.sha256((repo/f).read_bytes().replace(b'\r\n',b'\n')).hexdigest() for f in inputs},
        'tool_lock_sha256':hashlib.sha256((repo/'scripts/m68k-toolchain-lock.json').read_bytes().replace(b'\r\n',b'\n')).hexdigest(),
        'native_executed':False,'rom_integration':False}
    (out/'manifest.json').write_text(json.dumps(result,indent=2)+'\n')
    print('SPEC-0033 original68000 CODE image closure/storage PASS; no execution claim.',result['storage_bytes_68000'])
if __name__=='__main__':main()
