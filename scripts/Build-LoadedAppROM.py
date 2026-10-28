# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Original first loaded-application ROM. Compiler only; never launches a guest."""
import argparse,hashlib,json,os,struct,sys
from pathlib import Path
from importlib.util import spec_from_file_location,module_from_spec

def audit_register_checkpoint(image,syms,enabled):
    """Exact original MWR1 leaf: mask, own-RAM loads, register data, spin."""
    entry=syms.get('cs_mac_window_idle_checkpoint')
    spin=syms.get('cs_mac_window_idle_spin')
    if not enabled:
        if entry is not None or spin is not None:raise ValueError('Checkpoint present in default ROM')
        return None
    trace=syms.get('cs_mac_trace',0);diag=syms.get('cs_mac_diag',0)
    if trace%4 or diag%4 or not 0x400<=trace<=0x2100-128 or not 0x400<=diag<=0x2100-28:
        raise ValueError('Checkpoint own-RAM operands')
    expected=(bytes.fromhex('007c07002c7c')+struct.pack('>I',trace)
        +bytes.fromhex('2a79')+struct.pack('>I',diag+24)+bytes.fromhex('203c4d575231'))
    for reg,offset in enumerate((28,36,48,100,104,120,124),1):
        expected+=struct.pack('>HH',0x202e+(reg<<9),offset)
    for reg,offset in enumerate((108,112,116,52,56)):
        expected+=struct.pack('>HH',0x206e+(reg<<9),offset)
    expected+=bytes.fromhex('60fe')
    if len(expected)!=72 or entry is None or spin!=entry+70 or entry%2 or not 260<=entry-0x400000<=65536-len(expected):
        raise ValueError('Checkpoint native span/spin')
    at=entry-0x400000
    if image[at:at+len(expected)]!=expected:raise ValueError('Checkpoint register load/mask/spin encoding')
    return {'format':'MWR1','entry':entry,'spin':spin,'bytes':72,'trace_address':trace,
        'diag_fault_address':diag+24,'exact_opcodes_verified':True,'guest_memory_writes':0,
        'MMIO_accesses':0,'additional_RAM_bytes':0,'semantic_session_shutdown':False}

def audit_activity_checkpoint(image,syms,enabled):
    spec=spec_from_file_location('mac_activity_audit',Path(__file__).with_name('Audit-MacActivityCheckpoint.py'))
    module=module_from_spec(spec);spec.loader.exec_module(module)
    return module.audit(image,syms,enabled)

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
 'personality/system1/quickdraw-rects.h','personality/system1/code-run.h','personality/system1/code-run-internal.h',
 'personality/system1/code-select.h','personality/system1/quickdraw-select.h','personality/system1/quickdraw-view.h')
WINDOW_SOURCES=('arch/m68k/loaded-reset.S','arch/m68k/runtime.c','arch/m68k/composition-runtime.c','arch/m68k/exclusion.c',
 'arch/m68k/window-irqs.S','platform/mac128k/loop.c','platform/mac128k/window-entry.c','platform/mac128k/window-session.c',
 'platform/mac128k/motion.c','core/graphics/surface.c','core/graphics/cursor.c','core/input/pointer.c','core/input/mouse.c',
 'core/ui/window.c','apps/product-scene/scene.c','apps/window-scene/scene.c','apps/window-scene/input.c',
 'apps/window-scene/capture.c','apps/window-scene/inbox.c','apps/window-scene/foreground.c')
WINDOW_HEADERS=('arch/m68k/loaded-rom.ld','arch/m68k/runtime.h','arch/m68k/exclusion.h','platform/mac128k/rom.h',
 'platform/mac128k/window-session.h','platform/mac128k/motion.h','platform/mac128k/input-probe.h','core/graphics/surface.h','core/graphics/cursor.h',
 'core/input/pointer.h','core/input/mouse.h','core/ui/window.h','core/services/traps.h','apps/product-scene/scene.h',
 'apps/window-scene/scene.h','apps/window-scene/input.h','apps/window-scene/capture.h','apps/window-scene/inbox.h',
 'apps/window-scene/foreground.h','scripts/Build-LoadedAppROM.py','scripts/Build-MacROM.py')
def main():
    p=argparse.ArgumentParser();p.add_argument('--tool-root',required=True);p.add_argument('--output',required=True)
    mode=p.add_mutually_exclusive_group()
    mode.add_argument('--drawing',action='store_true',help='Build the SPEC-0036 interpreted drawing candidate')
    mode.add_argument('--native-drawing',action='store_true',help='Build the SPEC-0038 direct native drawing candidate')
    mode.add_argument('--selected-drawing',action='store_true',help='Build the SPEC-0047 direct native two-port candidate')
    mode.add_argument('--outline-drawing',action='store_true',help='Build the SPEC-0049 paired native outline preview')
    mode.add_argument('--window-drawing',action='store_true',help='Build the SPEC-0072 native Mac window preview')
    p.add_argument('--window-idle-probe',action='store_true',help='Diagnostic: halt in a self-branch after >=500 services; window profile only')
    p.add_argument('--window-pending-defer-probe',action='store_true',help='Experimental one-service pending-edge defer; window profile only')
    p.add_argument('--window-first-input-probe',action='store_true',help='Freeze copied trace on delivered input/loss or >=5000 services; window only')
    p.add_argument('--window-timer-poll-probe',action='store_true',help='Separate timer-driven live-level polling experiment;window only')
    p.add_argument('--window-timer-refresh-probe',action='store_true',help='Opt-in SCC status-refresh endpoint experiment;requires timer polling')
    p.add_argument('--window-timer-activity-probe',action='store_true',help='Passive MWA1 activity checkpoint;requires timer polling and idle checkpoint')
    p.add_argument('--window-timer-recovery-probe',action='store_true',help='Observe512 services after first activity;requires activity checkpoint')
    p.add_argument('--window-timer-divisor',type=int,choices=(1,4,16),default=1,help='Experimental programmed reload divisor;nondefault requires recovery and refresh')
    p.add_argument('--window-phase-idle-probe',action='store_true',help='Experimental phase-only no-decision pass; window profile only')
    p.add_argument('--dcd-invert',type=int,choices=(0,1,2,3),default=0,help='Unverified raw RR0 DCD sense mask; window profile only')
    a=p.parse_args()
    if a.window_timer_poll_probe and not a.window_drawing:p.error('--window-timer-poll-probe requires --window-drawing')
    if a.window_timer_poll_probe and (a.window_pending_defer_probe or a.window_phase_idle_probe or a.window_first_input_probe):p.error('Timer experiment cannot combine with SCC defer/phase/first-input experiments')
    if a.window_timer_refresh_probe and not a.window_timer_poll_probe:p.error('--window-timer-refresh-probe requires --window-timer-poll-probe')
    if a.window_timer_activity_probe and not (a.window_timer_poll_probe and a.window_idle_probe):p.error('--window-timer-activity-probe requires timer polling and --window-idle-probe')
    if a.window_timer_recovery_probe and not a.window_timer_activity_probe:p.error('--window-timer-recovery-probe requires --window-timer-activity-probe')
    if a.window_timer_divisor!=1 and not (a.window_timer_recovery_probe and a.window_timer_refresh_probe):p.error('Nondefault timer divisor requires recovery and refresh probes')
    native=a.native_drawing or a.selected_drawing or a.outline_drawing or a.window_drawing
    if a.dcd_invert and not a.window_drawing:p.error('--dcd-invert requires --window-drawing')
    if a.window_idle_probe and not a.window_drawing:p.error('--window-idle-probe requires --window-drawing')
    if a.window_pending_defer_probe and not a.window_drawing:p.error('--window-pending-defer-probe requires --window-drawing')
    if a.window_pending_defer_probe and a.window_idle_probe:p.error('Choose one diagnostic profile')
    if a.window_first_input_probe and not a.window_drawing:p.error('--window-first-input-probe requires --window-drawing')
    if a.window_first_input_probe and a.window_idle_probe:p.error('Choose one diagnostic profile')
    if a.window_phase_idle_probe and not a.window_drawing:p.error('--window-phase-idle-probe requires --window-drawing')
    if a.window_phase_idle_probe and (a.window_idle_probe or a.window_first_input_probe):p.error('Phase-idle probe is separate from checkpoint profiles')
    repo=Path(__file__).resolve().parents[1];tools=Path(a.tool_root).resolve();out=Path(a.output).resolve()
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
    if native:
        sources=tuple('platform/mac128k/native-drawing-entry.c' if s=='platform/mac128k/loaded-entry.c' else s for s in sources)
        sources+=('apps/native-drawing/workload.c','apps/product-scene/scene.c','core/input/mouse.c','personality/system1/quickdraw-view.c')
        headers+=('apps/native-drawing/workload.h','apps/product-scene/scene.h','core/input/mouse.h',
                  'personality/system1/quickdraw-view.h','docs/specifications/SPEC-0041-native-drawing-context.md',
                  'docs/specifications/SPEC-0039-native-mouse-foundations.md',
                  'docs/specifications/SPEC-0038-paired-native-drawing-workload.md',
                  'docs/specifications/SPEC-0037-shared-product-scene.md')
        program_source=None;program_hex='';specification='SPEC-0038';profile='native-rectangles-v1'
    if a.selected_drawing:
        sources+=('apps/selected-drawing/workload.c','personality/system1/quickdraw-select.c')
        headers+=('apps/selected-drawing/workload.h','personality/system1/quickdraw-select.h',
                  'docs/specifications/SPEC-0047-paired-selected-drawing.md')
        specification='SPEC-0047';profile='native-selected-ports-v1'
    if a.outline_drawing:
        sources+=('apps/outline-drawing/workload.c','personality/system1/quickdraw-select.c','personality/system1/quickdraw-outline.c')
        headers+=('apps/outline-drawing/workload.h','apps/selected-drawing/workload.h','personality/system1/quickdraw-select.h','personality/system1/quickdraw-outline.h','docs/specifications/SPEC-0048-native-rectangle-outline.md','docs/specifications/SPEC-0049-paired-native-outline.md')
        specification='SPEC-0049';profile='native-outline-preview-v1'
    if a.window_drawing:
        specs=[]
        numbers=((('0070','0072','0074','0075','0077') if a.window_timer_refresh_probe else ('0070','0072','0074','0075')) if a.window_timer_poll_probe else ('0070','0072'))
        if a.window_timer_activity_probe:numbers+=('0078',)
        if a.window_timer_recovery_probe:numbers+=('0080',)
        for number in numbers:
            found=sorted(repo.glob('docs/specifications/SPEC-'+number+'-*.md'))
            if len(found)!=1:raise ValueError('Specification lookup SPEC-'+number)
            specs.append(found[0].relative_to(repo).as_posix())
        sources=WINDOW_SOURCES;headers=WINDOW_HEADERS+tuple(specs)
        if a.window_timer_poll_probe:
            sources=tuple(x for x in sources if x!='platform/mac128k/motion.c')+('platform/mac128k/timer-poll.c','platform/mac128k/polled-phase.c')
            headers+=('platform/mac128k/timer-poll.h','platform/mac128k/polled-phase.h')
        if a.window_timer_activity_probe:
            sources+=('platform/mac128k/activity.c','arch/m68k/activity-checkpoint.S')
            headers+=('platform/mac128k/activity.h',)
        if a.window_timer_recovery_probe:
            sources+=('platform/mac128k/activity-window.c',)
            headers+=('platform/mac128k/activity-window.h',)
        program_source=None;program_hex='';specification='SPEC-0072';profile='native-window-preview-v1'
        if a.window_pending_defer_probe:profile='native-window-pending-defer-probe-v1'
        if a.window_first_input_probe:profile='native-window-'+('pending-defer-' if a.window_pending_defer_probe else '')+'first-input-probe-v1'
        if a.window_timer_poll_probe:profile='native-window-timer-poll-'+('idle-' if a.window_idle_probe else '')+'experiment-v1'
        if a.window_timer_refresh_probe:profile='native-window-timer-refresh-'+('idle-' if a.window_idle_probe else '')+'experiment-v1'
        if a.window_timer_activity_probe:profile='native-window-timer-activity-'+('refresh-' if a.window_timer_refresh_probe else '')+'checkpoint-v1';specification='SPEC-0078'
        if a.window_timer_recovery_probe:profile='native-window-timer-recovery-'+('refresh-' if a.window_timer_refresh_probe else '')+'checkpoint-v1';specification='SPEC-0080'
        if a.window_timer_divisor!=1:profile=profile.replace('checkpoint-v1','divisor'+str(a.window_timer_divisor)+'-checkpoint-v1')
        if a.window_phase_idle_probe:profile='native-window-phase-idle-'+('pending-defer-' if a.window_pending_defer_probe else '')+'probe-v1'
    profiles=[]
    for twin in ('a','b'):
        folder=out/('Os-'+twin);folder.mkdir();objects=[]
        flags=['-mcpu=68000','-Wa,-m68000','-Os','-std=c11','-ffreestanding','-fno-builtin',
         '-fno-pic','-fno-pie','-fno-common','-fno-stack-protector','-fno-unwind-tables',
         '-fno-asynchronous-unwind-tables','-fno-tree-loop-distribute-patterns','-fstack-usage',
         '-ffunction-sections','-fdata-sections','-Wall','-Wextra','-Werror','-Wconversion','-Wsign-conversion',
         '--sysroot='+str(tools),'-ffile-prefix-map='+str(repo)+'=.','-ffile-prefix-map='+str(folder)+'=.']
        if a.selected_drawing or a.outline_drawing:flags+=['-DCLASSICK_SELECTED_DRAWING=1']
        if a.outline_drawing:flags+=['-DCLASSICK_OUTLINE_DRAWING=1']
        if a.window_drawing:flags+=['-DCLASSICK_MAC_DCD_INVERT='+str(a.dcd_invert)]
        if a.window_idle_probe or a.window_first_input_probe:flags+=['-DCLASSICK_MAC_IDLE_PROBE=1']
        if a.window_timer_poll_probe:flags+=['-DCLASSICK_MAC_TIMER_POLL_PROBE=1']
        if a.window_timer_refresh_probe:flags+=['-DCLASSICK_MAC_TIMER_REFRESH_PROBE=1']
        if a.window_timer_activity_probe:flags+=['-DCLASSICK_MAC_TIMER_ACTIVITY_PROBE=1']
        if a.window_timer_recovery_probe:flags+=['-DCLASSICK_MAC_TIMER_RECOVERY_PROBE=1']
        if a.window_timer_divisor!=1:flags+=['-DCLASSICK_MAC_TIMER_DIVISOR='+str(a.window_timer_divisor)]
        if a.window_first_input_probe:flags+=['-DCLASSICK_MAC_FIRST_INPUT_PROBE=1']
        if a.window_pending_defer_probe:flags+=['-DCLASSICK_MAC_PENDING_DEFER_PROBE=1']
        if a.window_phase_idle_probe:flags+=['-DCLASSICK_MAC_PHASE_IDLE_PROBE=1']
        for i,source in enumerate(sources):
            obj=folder/f'{i}.o';objects.append(obj);h.run([cc,*flags,'-c',repo/source,'-o',obj],env)
            if source not in ('platform/mac128k/loaded-entry.c','platform/mac128k/drawing-entry.c','platform/mac128k/native-drawing-entry.c','platform/mac128k/window-entry.c'):
                for line in h.run([bins/'m68k-linux-gnu-nm','--defined-only','--format=posix',obj],env).splitlines():
                    fields=line.split()
                    if len(fields)>1 and fields[1] in 'BbCcDdGgSsVv':raise ValueError('Mutable core global: '+source+' '+line)
        elf=folder/'classick-loaded-app.elf'
        link=[cc,'-mcpu=68000','-nostdlib','-no-pie','-Wl,--build-id=none','-Wl,--gc-sections',
         '-Wl,-T,'+str(repo/'arch/m68k/loaded-rom.ld'),'-Wl,-Map,'+str(folder/'classick-loaded-app.map')]
        h.run([*link,*objects,'-o',elf],env);syms=h.symbols(elf)
        if not 0x8000<=syms['__loaded_app_start']<syms['__loaded_app_end']<=0x1A000:raise ValueError('Workspace bounds')
        required=('cs_native_draw','cs_qd_rect','cs_qd_view_init','cs_qd_view_offset','cs_qd_view_clip','cs_qd_view_rect','cs_product_scene') if native else (('cs_drawing_app_probe','cs_image_run_rectangles') if a.drawing else ('cs_loaded_app_probe','cs_image_run'))
        if not native:required=(*required,'cs_m68k_bus_step_admitted')
        if a.selected_drawing:required=tuple(x for x in required if x!='cs_native_draw')+('cs_selected_draw','cs_qd_select_get','cs_qd_select_set')
        if a.outline_drawing:required=tuple(x for x in required if x not in ('cs_native_draw','cs_qd_rect','cs_qd_view_rect'))+('cs_outline_draw','cs_qd_view_outline','cs_qd_select_get','cs_qd_select_set')
        if a.window_drawing:required=('cs_mac_main','cs_mac_window_begin','cs_mac_motion_service','cs_foreground_step','cs_mac_window_via_irq','cs_mac_window_scc_irq','cs_mac_window_combined_irq','cs_mac_window_interrupt','cs_product_scene','cs_window_apply','cs_m68k_foreground_enter','cs_m68k_foreground_leave')
        if a.window_timer_poll_probe:required=tuple(x for x in required if x!='cs_mac_motion_service')+('cs_mac_timer_poll_service','cs_mac_polled_step')
        if a.window_timer_activity_probe:required+=('cs_mac_activity_pair','cs_mac_activity_trace','cs_mac_activity_checkpoint','cs_mac_activity_spin')
        elif any(name.startswith('cs_mac_activity_') for name in syms):raise ValueError('Unexpected activity observer in default profile')
        if a.window_timer_recovery_probe:required+=('cs_mac_activity_window_begin','cs_mac_activity_window_step','cs_mac_activity_gate')
        elif any(name.startswith('cs_mac_activity_window_') or name=='cs_mac_activity_gate' for name in syms):raise ValueError('Recovery gate in another profile')
        if (a.window_idle_probe and not a.window_timer_activity_probe) or a.window_first_input_probe:required+=('cs_mac_window_idle_checkpoint','cs_mac_window_idle_spin')
        if a.window_first_input_probe:required+=('cs_mac_first_input_probe_gate',)
        if a.window_drawing and not any(name.startswith('cs_cursor_') for name in syms):raise ValueError('Disconnected cursor helper')
        if any(name not in syms for name in required):raise ValueError('Disconnected demonstration')
        cpu_callbacks=('cs_m68k_foreground_enter','cs_m68k_foreground_leave') if a.window_drawing else ()
        if any(name not in syms for name in cpu_callbacks):raise ValueError('Missing CPU exclusion callbacks')
        forbidden=('cs_image_','cs_m68k_','cs_drawing_app_','cs_aline_probe')+(('cs_guest_','cs_memory_','cs_resource','cs_rsrc_','cs_code_','cs_system1_','cs_trap_','cs_service_','cs_qd_','cs_mac_loop_') if a.window_drawing else ())
        if native and any(name.startswith(forbidden) and name not in cpu_callbacks for name in syms):
            raise ValueError('Unexpected interpreted application in native drawing profile')
        raw=folder/'loaded.raw';h.run([bins/'m68k-linux-gnu-objcopy','-O','binary',elf,raw],env)
        b=bytearray(raw.read_bytes());b.extend(bytes(65536-len(b)));struct.pack_into('>I',b,256,h.crc(b))
        def audit_window_code(image):
            rows=[]
            for source,name in enumerate(('cs_mac_window_via_irq','cs_mac_window_scc_irq','cs_mac_window_combined_irq'),1):
                expected=(bytes.fromhex('007c070048e7fffe2f3c')+struct.pack('>I',source)
                    +bytes.fromhex('4eb9')+struct.pack('>I',syms['cs_mac_window_interrupt'])
                    +bytes.fromhex('588f4cdf7fff4e73'))
                at=syms[name]-0x400000
                if not 260<=at<=65536-len(expected) or image[at:at+len(expected)]!=expected:
                    raise ValueError('IRQ preservation/mask/RTE: '+name)
                rows.append({'name':name,'source':source,'bytes':len(expected),'exact_opcodes_verified':True})
            audit_register_checkpoint(image,syms,(a.window_idle_probe and not a.window_timer_activity_probe) or a.window_first_input_probe)
            audit_activity_checkpoint(image,syms,a.window_timer_activity_probe)
            if a.window_timer_activity_probe:
                activity=syms['cs_mac_activity_trace']
                if activity%4 or not 0x400<=activity<=0x2100-80:raise ValueError('Activity record own-RAM bounds')
                for other,size in ((syms['cs_mac_trace'],128),(syms['cs_mac_diag'],64)):
                    if activity<other+size and other<activity+80:raise ValueError('Activity record overlaps prior diagnostics')
            if a.window_timer_recovery_probe:
                gate=syms['cs_mac_activity_gate']
                if gate%4 or not 0x400<=gate<=0x2100-16:raise ValueError('Recovery gate own-RAM bounds')
                for other,size in ((syms['cs_mac_trace'],128),(syms['cs_mac_diag'],64),(syms['cs_mac_activity_trace'],80)):
                    if gate<other+size and other<gate+16:raise ValueError('Recovery gate overlaps diagnostics')
            if ('cs_mac_first_input_probe_gate' in syms)!=bool(a.window_first_input_probe):raise ValueError('First-input gate/profile mismatch')
            at=syms['cs_mac_window_install_vectors']-0x400000
            for vector,name in zip((0x64,0x68,0x6C),('cs_mac_window_via_irq','cs_mac_window_scc_irq','cs_mac_window_combined_irq')):
                if not 260<=at<=65526:raise ValueError('Vector installer span')
                opcode=struct.unpack_from('>H',image,at)[0]
                if opcode==0x21FC:destination=struct.unpack_from('>H',image,at+6)[0];length=8
                elif opcode==0x23FC:destination=struct.unpack_from('>I',image,at+6)[0];length=10
                else:raise ValueError('Vector installer instruction')
                if struct.unpack_from('>I',image,at+2)[0]!=syms[name] or destination!=vector:
                    raise ValueError('Vector installer target/destination')
                at+=length
            if image[at:at+2]!=bytes.fromhex('4e75'):raise ValueError('Vector installer return')
            return rows
        def audit(image):
            if len(image)!=65536 or struct.unpack_from('>II',image)!=(0x8000,syms['cs_mac_reset']):raise ValueError('Loaded reset vectors')
            if struct.unpack_from('>I',image,256)[0]!=h.crc(image):raise ValueError('Loaded seal')
            if a.window_drawing:
                for vector in range(2,64):
                    target=syms['cs_mac_probe_handler'] if vector==32 else syms['cs_mac_line_a_handler'] if vector==10 else syms['cs_mac_fault']
                    if struct.unpack_from('>I',image,vector*4)[0]!=target:raise ValueError('Native window exception vector')
                at=syms['cs_mac_line_a_handler']-0x400000
                fatal=bytes.fromhex('4ef9')+struct.pack('>I',syms['cs_mac_fault'])
                if image[at:at+len(fatal)]!=fatal:raise ValueError('Native window Line-A fatal entry')
                audit_window_code(image)
            else:
                legacy=bytearray(image);struct.pack_into('>I',legacy,0,0x4000);struct.pack_into('>I',legacy,256,h.crc(legacy))
                h.audit_image(legacy,syms)
        audit(b)
        irq_audit=[]
        if a.window_drawing:
            irq_audit=audit_window_code(b)
            listing=h.run([bins/'m68k-linux-gnu-objdump','-d',elf],env)
            (folder/'native-window-disassembly.txt').write_text(listing)
        for offset in (0,4,8,40,128,256,1024,65535):
            bad=bytearray(b);bad[offset]^=1
            try:audit(bad)
            except ValueError:pass
            else:raise ValueError('Corruption accepted')
        if a.window_drawing:
            controls=[40,syms['cs_mac_line_a_handler']-0x400000,
                    syms['cs_mac_window_via_irq']-0x400000+3,syms['cs_mac_window_install_vectors']-0x400000+7]
            if (a.window_idle_probe and not a.window_timer_activity_probe) or a.window_first_input_probe:
                start=syms['cs_mac_window_idle_checkpoint']-0x400000
                controls.extend(start+x for x in (3,8,14,21,25,29,33,37,41,45,49,53,57,61,65,69,71))
            if a.window_timer_activity_probe:
                controls.extend(syms['cs_mac_activity_checkpoint']-0x400000+x for x in range(74))
            for offset in controls:
                bad=bytearray(b);bad[offset]^=1;struct.pack_into('>I',bad,256,h.crc(bad))
                try:audit(bad)
                except ValueError:pass
                else:raise ValueError('Resealed native corruption accepted')
        frames=[]
        for f in sorted(folder.glob('*.su')):
            for line in f.read_text().splitlines():
                fields=line.split('\t')
                if len(fields)!=3 or fields[2] not in ('static','dynamic,bounded') or int(fields[1])>1024:raise ValueError('Frame: '+line)
                frames.append({'function':fields[0].split(':')[-1],'bytes':int(fields[1])})
        # Sum even discarded frames. Legacy synchronous profiles stay mask7.
        # Window profile permits one serialized producer over unmasked foreground;
        # 256B covers register save/argument/return and pre-mask exception margin.
        # Calls are nonrecursive; shared inbox operations cannot overlap.
        stack_margin=256 if a.window_drawing else 192
        stack_bound=sum(x['bytes'] for x in frames)+stack_margin
        if stack_bound>0x8000-0x2100:raise ValueError('Conservative native stack reserve exceeded: '+str(stack_bound))
        image=folder/'classick-loaded-app.rom';image.write_bytes(b)
        profiles.append({'optimization':'-Os','image_sha256':hashlib.sha256(b).hexdigest(),'image_size':len(b),
         'rom_used':syms['__rom_end']-0x400000,'native_data_bss':syms['__bss_end']-0xB00,
         'workspace_begin':syms['__loaded_app_start'],'workspace_end':syms['__loaded_app_end'],
         'native_stack_base':0x2100,'native_stack_top':0x8000,'conservative_stack_bound':stack_bound,
         'individual_frame_max':max(x['bytes'] for x in frames),'stack_frames':frames,'symbols':syms,'corruption_controls':8})
        if a.window_drawing:profiles[-1].update(irq_veneer_audit=irq_audit,resealed_native_corruption_controls=len(controls))
        if a.window_timer_activity_probe:
            profiles[-1]['activity_register_checkpoint']=audit_activity_checkpoint(b,syms,True)
        if (a.window_idle_probe and not a.window_timer_activity_probe) or a.window_first_input_probe:
            profiles[-1]['register_checkpoint']=audit_register_checkpoint(b,syms,True)
    if profiles[0]!=profiles[1]:raise ValueError('ROM twins differ')
    if program_source is not None:
        fixture=out/'program.o';binary=out/'program.bin'
        h.run([cc,'-mcpu=68000','-Wa,-m68000','-c',repo/program_source,'-o',fixture],env)
        h.run([bins/'m68k-linux-gnu-objcopy','-O','binary','--only-section=.text',fixture,binary],env)
        if binary.read_bytes()!=bytes.fromhex(program_hex):raise ValueError('Original program encoding mismatch')
    manifest={'schema_version':1,'specification':specification,'profile':profile,
     'compiler':h.run([cc,'--version'],env).splitlines()[0],'profiles':[profiles[0]],'twins_equal':True,
     'original_program_bytes':len(bytes.fromhex(program_hex)),'execution_path':'direct-native-c' if native else 'guarded-68k-fixture',
     'module_count':len(sources),'selected_ports':bool(a.selected_drawing or a.outline_drawing),'outline_drawing':bool(a.outline_drawing),'native_drawing_context':('SPEC-0041' if native else None),'emulator_run':False,'system1_compatibility_proven':False,
     'inputs':{f:hashlib.sha256((repo/f).read_bytes().replace(b'\r\n',b'\n')).hexdigest() for f in (*sources,*headers)}}
    if a.window_timer_poll_probe:manifest.update({'timer_poll_experiment':True,'timer_reload_count':4096//a.window_timer_divisor,'timer_frequency_qualified':False,'physical_edge_completeness_qualified':False,'SCC_master_interrupts':False})
    if a.window_timer_refresh_probe:manifest['timer_refresh_experiment']=True
    if a.window_drawing:manifest['window_idle_probe']=bool(a.window_idle_probe)
    if a.window_drawing:manifest['window_pending_defer_probe']=bool(a.window_pending_defer_probe)
    if a.window_drawing:manifest['window_phase_idle_probe']=bool(a.window_phase_idle_probe)
    if a.window_drawing:manifest['window_first_input_probe']=bool(a.window_first_input_probe)
    if a.window_first_input_probe:manifest['window_first_input_probe_limit']=5000
    if a.window_idle_probe and not a.window_timer_activity_probe:manifest['window_idle_probe_threshold']=500
    if a.window_timer_activity_probe:manifest['activity_checkpoint']={'tag':'MWA1','version':1,'bytes':80,'address':syms['cs_mac_activity_trace'],'minimum_services':500,'fallback_services':50000,'wall_time_qualified':False,'extra_MMIO_reads':0,'extra_MMIO_writes':0,'old_diagnostics_unchanged':True}
    if a.window_timer_recovery_probe:manifest['recovery_window']={'specification':'SPEC-0080','delay_services':512*a.window_timer_divisor,'minimum_services':500*a.window_timer_divisor,'fallback_services':50000*a.window_timer_divisor,'programmed_reload_divisor':a.window_timer_divisor,'state_bytes':16,'state_address':syms['cs_mac_activity_gate'],'timer_rate_qualified':False,'MAR1_exposes_completion_reason':False,'extra_MMIO_reads':0,'extra_MMIO_writes':0}
    if a.window_drawing:manifest.update({'window_drawing':True,'native_drawing_context':None,
     'dcd_invert':a.dcd_invert,'dcd_invert_verified':False,'cpu_callbacks':list(cpu_callbacks),
     'board_profile':{'status':'engineering-upper-alias-hypothesis-unverified','scc_read_b':0x9FFFF8,'scc_read_a':0x9FFFFA,
      'scc_write_b':0xBFFFF9,'scc_write_a':0xBFFFFB,'ab_select_bit':1,'dc_select_bit':2,'scc_byte_guard_nops':16},
     'interrupt_stack_margin':stack_margin,
     'proof_limits':['compile/link/seal only','no boot or hardware trial','SCC aliases, DCD sense, pending coherence and physical fit unverified','stack bound is a nonrecursive frame sum plus margin, not measured']})
    (out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    print('First loaded-app ROM twins PASS:',profiles[0]['rom_used'],'ROM bytes,',profiles[0]['workspace_end']-0x8000,'workspace bytes; no boot trial.')
if __name__=='__main__':main()
