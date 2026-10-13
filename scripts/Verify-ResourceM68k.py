# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Original resource reader 68000 closure/storage audit; no execution."""
import argparse,hashlib,json,os,sys
from pathlib import Path
from importlib.util import spec_from_file_location,module_from_spec
def main():
    p=argparse.ArgumentParser();p.add_argument('--tool-root',required=True);p.add_argument('--output',required=True);a=p.parse_args()
    repo=Path(__file__).resolve().parents[1];root=Path(a.tool_root).resolve();out=Path(a.output).resolve();out.mkdir(parents=True,exist_ok=False)
    spec=spec_from_file_location('owned_audit',repo/'scripts/Verify-GuestM68k.py');h=module_from_spec(spec)
    sys.dont_write_bytecode=True;spec.loader.exec_module(h)
    env=os.environ.copy();env['LD_LIBRARY_PATH']=str(root/'usr/lib/x86_64-linux-gnu');bins=root/'usr/bin'
    cc=bins/'m68k-linux-gnu-gcc-13';nm=bins/'m68k-linux-gnu-nm'
    flags=['-mcpu=68000','-Wa,-m68000','-std=c11','-ffreestanding','-fno-builtin','-fno-pic','-fno-pie',
        '-fno-common','-fno-stack-protector','-fno-tree-loop-distribute-patterns','-fstack-usage',
        '-Wall','-Wextra','-Werror','-Wconversion','-Wsign-conversion','--sysroot='+str(root),'-ffile-prefix-map='+str(repo)+'=.']
    profiles=[]
    for opt in ['-O0','-Os']:
        folder=out/opt[1:];folder.mkdir();obj=folder/'reader.o';combined=folder/'original-resource-reader.o'
        h.run([cc,*flags,opt,'-ffile-prefix-map='+str(folder)+'=.','-c',repo/'formats/resourcefork/reader.c','-o',obj],env)
        h.audit(obj,nm,set(),env);h.run([bins/'m68k-linux-gnu-ld','-r',obj,'-o',combined],env);h.audit(combined,nm,set(),env)
        frames=[]
        for line in (folder/'reader.su').read_text(encoding='utf-8').splitlines():
            fields=line.split('\t')
            if len(fields)!=3 or fields[2] not in ['static','dynamic,bounded'] or int(fields[1])>512:raise ValueError('Unbounded/oversized frame: '+line)
            frames.append({'function':fields[0].split(':')[-1],'bytes':int(fields[1])})
        profiles.append({'optimization':opt,'objects':1,'object_sha256':hashlib.sha256(obj.read_bytes()).hexdigest(),
            'combined_sha256':hashlib.sha256(combined.read_bytes()).hexdigest(),'stack_frames':frames})
    source=out/'footprint.c';source.write_text('#include "formats/resourcefork/reader.h"\nconst uint32_t cs_rf_storage[]={sizeof(cs_rf_view),sizeof(cs_rf_entry),sizeof(cs_rf_report)};\n',encoding='utf-8')
    h.run([cc,*flags,'-Os','-I'+str(repo),'-c',source,'-o',out/'footprint.o'],env)
    h.run([bins/'m68k-linux-gnu-objcopy','-O','binary','--only-section=.rodata',out/'footprint.o',out/'footprint.bin'],env)
    storage=(out/'footprint.bin').read_bytes();assert len(storage)==12
    inputs=['formats/resourcefork/reader.c','formats/resourcefork/reader.h','docs/specifications/SPEC-0024-resource-fork-reader.md',
        'scripts/Verify-ResourceM68k.py','scripts/Verify-GuestM68k.py']
    result={'schema_version':1,'specification':'SPEC-0024','compiler':h.run([cc,'--version'],env).splitlines()[0],
        'object_audits':2,'complete_relocatable_links':2,'storage_bytes_68000':dict(zip(['view','entry','report'],[int.from_bytes(storage[i:i+4],'big') for i in range(0,12,4)])),
        'profiles':profiles,'inputs':{f:hashlib.sha256((repo/f).read_bytes().replace(b'\r\n',b'\n')).hexdigest() for f in inputs},
        'tool_lock_sha256':hashlib.sha256((repo/'scripts/m68k-toolchain-lock.json').read_bytes().replace(b'\r\n',b'\n')).hexdigest(),
        'native_executed':False,'rom_integration':False}
    (out/'manifest.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
    print('SPEC-0024 original68000 reader closure/storage PASS; no execution claim.')
if __name__=='__main__':main()
