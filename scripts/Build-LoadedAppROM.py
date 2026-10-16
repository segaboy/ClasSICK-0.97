# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Original first loaded-application ROM. Compiler only; never launches a guest."""
import argparse,hashlib,json,os,struct,sys
from pathlib import Path
from importlib.util import spec_from_file_location,module_from_spec
SOURCES=('arch/m68k/loaded-reset.S','arch/m68k/runtime.c','platform/mac128k/loop.c',
 'platform/mac128k/loaded-entry.c','apps/rom-scene/scene.c','core/graphics/surface.c',
 'core/services/traps.c','apps/service-probe/probe.c','core/cpu/m68k.c','apps/aline-probe/probe.c',
 'arch/m68k/aline.S','core/memory/guest.c','core/memory/manager.c','core/memory/lifecycle.c',
 'formats/resourcefork/reader.c','personality/system1/code-preflight.c','personality/system1/resources.c',
 'personality/system1/code-image.c','personality/system1/code-run.c','core/cpu/m68k-bus.c',
 'apps/loaded-rom-app/probe.c','apps/loaded-rom-app/scene.c','personality/system1/quickdraw-rects.c')
HEADERS=('arch/m68k/loaded-rom.ld','arch/m68k/runtime.h','platform/mac128k/rom.h',
 'apps/loaded-rom-app/probe.h','apps/loaded-rom-app/scene.h','core/graphics/surface.h',
 'core/services/traps.h','core/cpu/m68k.h','core/cpu/m68k-bus.h','core/cpu/m68k-guard.h',
 'core/cpu/m68k-session.h','core/cpu/m68k-call.h','core/memory/guest.h','core/memory/manager.h',
 'core/memory/lifecycle.h','formats/resourcefork/reader.h','personality/system1/code-preflight.h',
 'personality/system1/resources.h','personality/system1/code-image.h','personality/system1/code-run.h',
 'personality/system1/code-run-internal.h','apps/service-probe/probe.h','apps/aline-probe/probe.h',
 'apps/aline-probe/program.h','apps/loaded-rom-app/program.S','scripts/Build-LoadedAppROM.py','scripts/Build-MacROM.py','docs/specifications/SPEC-0035-first-loaded-rom-application.md',
 'personality/system1/quickdraw-rects.h','personality/system1/code-run.h','personality/system1/code-run-internal.h')
def main():
    p=argparse.ArgumentParser();p.add_argument('--tool-root',required=True);p.add_argument('--output',required=True)
    p.add_argument('--drawing',action='store_true',help='Build the SPEC-0036 fixed-port drawing candidate')
    a=p.parse_args();repo=Path(__file__).resolve().parents[1];tools=Path(a.tool_root).resolve();out=Path(a.output).resolve()
    out.mkdir(parents=True,exist_ok=False);sys.dont_write_bytecode=True
    spec=spec_from_file_location('base_rom',repo/'scripts/Build-MacROM.py');h=module_from_spec(spec);spec.loader.exec_module(h)
    env=os.environ.copy();env['LD_LIBRARY_PATH']=str(tools/'usr/lib/x86_64-linux-gnu')
    env['PATH']=str(tools/'usr/bin')+os.pathsep+env.get('PATH','');bins=tools/'usr/bin';cc=bins/'m68k-linux-gnu-gcc-13'
    sources=SOURCES;headers=HEADERS;program_source='apps/loaded-rom-app/program.S'
    program_hex='204d213c5a5a5a5a70614e75';specification='SPEC-0035';profile='first-loaded-rom-app-v1'
    if a.drawing:
        sources=tuple('platform/mac128k/drawing-entry.c' if s=='platform/mac128k/loaded-entry.c' else s for s in sources)
        sources+=('apps/drawing-rom-app/probe.c','apps/drawing-rom-app/scene.c','apps/product-scene/scene.c')
        headers+=('apps/product-scene/scene.h','docs/specifications/SPEC-0037-shared-product-scene.md','apps/drawing-rom-app/probe.h','apps/drawing-rom-app/program.S','docs/specifications/SPEC-0036-loaded-rectangle-calls.md')
        program_source='apps/drawing-rom-app/program.S'
        program_hex='204d213c00380078213c000800082f08a8a2204d213c00300070213c001000102f08a8a370614e75'
        specification='SPEC-0036';profile='loaded-rectangles-v1'
    profiles=[]
    for twin in ('a','b'):
        folder=out/('Os-'+twin);folder.mkdir();objects=[]
        flags=['-mcpu=68000','-Wa,-m68000','-Os','-std=c11','-ffreestanding','-fno-builtin',
         '-fno-pic','-fno-pie','-fno-common','-fno-stack-protector','-fno-unwind-tables',
         '-fno-asynchronous-unwind-tables','-fno-tree-loop-distribute-patterns','-fstack-usage',
         '-ffunction-sections','-fdata-sections','-Wall','-Wextra','-Werror','-Wconversion','-Wsign-conversion',
         '--sysroot='+str(tools),'-ffile-prefix-map='+str(repo)+'=.','-ffile-prefix-map='+str(folder)+'=.']
        for i,source in enumerate(sources):
            obj=folder/f'{i}.o';objects.append(obj);h.run([cc,*flags,'-c',repo/source,'-o',obj],env)
            if source not in ('platform/mac128k/loaded-entry.c','platform/mac128k/drawing-entry.c'):
                for line in h.run([bins/'m68k-linux-gnu-nm','--defined-only','--format=posix',obj],env).splitlines():
                    fields=line.split()
                    if len(fields)>1 and fields[1] in 'BbCcDdGgSsVv':raise ValueError('Mutable core global: '+source+' '+line)
        elf=folder/'classick-loaded-app.elf'
        link=[cc,'-mcpu=68000','-nostdlib','-no-pie','-Wl,--build-id=none','-Wl,--gc-sections',
         '-Wl,-T,'+str(repo/'arch/m68k/loaded-rom.ld'),'-Wl,-Map,'+str(folder/'classick-loaded-app.map')]
        h.run([*link,*objects,'-o',elf],env);syms=h.symbols(elf)
        if not 0x8000<=syms['__loaded_app_start']<syms['__loaded_app_end']<=0x1A000:raise ValueError('Workspace bounds')
        required=('cs_drawing_app_probe','cs_image_run_rectangles') if a.drawing else ('cs_loaded_app_probe','cs_image_run')
        if any(name not in syms for name in (*required,'cs_m68k_bus_step_admitted')):raise ValueError('Disconnected demonstration')
        raw=folder/'loaded.raw';h.run([bins/'m68k-linux-gnu-objcopy','-O','binary',elf,raw],env)
        b=bytearray(raw.read_bytes());b.extend(bytes(65536-len(b)));struct.pack_into('>I',b,256,h.crc(b))
        def audit(image):
            if len(image)!=65536 or struct.unpack_from('>II',image)!=(0x8000,syms['cs_mac_reset']):raise ValueError('Loaded reset vectors')
            if struct.unpack_from('>I',image,256)[0]!=h.crc(image):raise ValueError('Loaded seal')
            legacy=bytearray(image);struct.pack_into('>I',legacy,0,0x4000);struct.pack_into('>I',legacy,256,h.crc(legacy))
            h.audit_image(legacy,syms)
        audit(b)
        for offset in (0,4,8,40,128,256,1024,65535):
            bad=bytearray(b);bad[offset]^=1
            try:audit(bad)
            except ValueError:pass
            else:raise ValueError('Corruption accepted')
        frames=[]
        for f in sorted(folder.glob('*.su')):
            for line in f.read_text().splitlines():
                fields=line.split('\t')
                if len(fields)!=3 or fields[2] not in ('static','dynamic,bounded') or int(fields[1])>1024:raise ValueError('Frame: '+line)
                frames.append({'function':fields[0].split(':')[-1],'bytes':int(fields[1])})
        # This synchronous, nonrecursive profile runs with mask7 and no callbacks.
        # Sum even discarded function frames, plus original assembly/exception margin.
        stack_bound=sum(x['bytes'] for x in frames)+192
        if stack_bound>0x8000-0x2100:raise ValueError('Conservative native stack reserve exceeded: '+str(stack_bound))
        image=folder/'classick-loaded-app.rom';image.write_bytes(b)
        profiles.append({'optimization':'-Os','image_sha256':hashlib.sha256(b).hexdigest(),'image_size':len(b),
         'rom_used':syms['__rom_end']-0x400000,'native_data_bss':syms['__bss_end']-0xB00,
         'workspace_begin':syms['__loaded_app_start'],'workspace_end':syms['__loaded_app_end'],
         'native_stack_base':0x2100,'native_stack_top':0x8000,'conservative_stack_bound':stack_bound,
         'individual_frame_max':max(x['bytes'] for x in frames),'stack_frames':frames,'symbols':syms,'corruption_controls':8})
    if profiles[0]!=profiles[1]:raise ValueError('ROM twins differ')
    fixture=out/'program.o';binary=out/'program.bin'
    h.run([cc,'-mcpu=68000','-Wa,-m68000','-c',repo/program_source,'-o',fixture],env)
    h.run([bins/'m68k-linux-gnu-objcopy','-O','binary','--only-section=.text',fixture,binary],env)
    if binary.read_bytes()!=bytes.fromhex(program_hex):raise ValueError('Original program encoding mismatch')
    manifest={'schema_version':1,'specification':specification,'profile':profile,
     'compiler':h.run([cc,'--version'],env).splitlines()[0],'profiles':[profiles[0]],'twins_equal':True,
     'original_program_bytes':len(bytes.fromhex(program_hex)),'module_count':len(sources),'emulator_run':False,'system1_compatibility_proven':False,
     'inputs':{f:hashlib.sha256((repo/f).read_bytes().replace(b'\r\n',b'\n')).hexdigest() for f in (*sources,*headers)}}
    (out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    print('First loaded-app ROM twins PASS:',profiles[0]['rom_used'],'ROM bytes,',profiles[0]['workspace_end']-0x8000,'workspace bytes; no boot trial.')
if __name__=='__main__':main()
