# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Check original compile-only window diagnostic proofs; never starts a guest."""
import argparse,hashlib,json,subprocess,sys
from importlib.util import spec_from_file_location,module_from_spec
from pathlib import Path

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--normal',required=True);p.add_argument('--probe',required=True)
    p.add_argument('--output',required=True)
    p.add_argument('--pending-defer',action='store_true',help='Verify the opt-in finite-defer experiment instead of the idle spin')
    p.add_argument('--first-input',action='store_true',help='Verify the first-input/loss/5000-service checkpoint, optionally over the defer baseline')
    a=p.parse_args();repo=Path(__file__).resolve().parents[1]
    spec=spec_from_file_location('checkpoint_build',repo/'scripts/Build-LoadedAppROM.py')
    build=module_from_spec(spec);spec.loader.exec_module(build)
    roots=[Path(a.normal),Path(a.probe)]
    manifests=[json.loads((r/'manifest.json').read_text()) for r in roots]
    baseline_defer=bool(a.first_input and a.pending_defer)
    expected=('cfa16dc2e816cc833fb6c2d51ca9088fdf26215c6aa989999783dbd08dcb4235' if baseline_defer else
              '673a2cca57d1ec9ffd842f4c858fa4aafbcb042ec6f86445ae5717d4816c4965')
    label='cs_mac_window_idle_checkpoint';profiles=[]
    for enabled,r,m in zip((False,True),roots,manifests):
        q=m['profiles'][0];b=(r/'Os-a/classick-loaded-app.rom').read_bytes()
        assert len(b)==65536 and hashlib.sha256(b).hexdigest()==q['image_sha256']
        pending=baseline_defer if a.first_input else (enabled and a.pending_defer)
        first=bool(enabled and a.first_input)
        assert m['window_drawing'] and m['window_idle_probe']==(enabled and not a.pending_defer and not a.first_input) and m['twins_equal']
        assert m.get('window_pending_defer_probe',False)==pending
        assert m.get('window_first_input_probe',False)==first
        assert ('cs_mac_first_input_probe_gate' in q['symbols'])==first
        if first:assert m['profile']=='native-window-'+('pending-defer-' if pending else '')+'first-input-probe-v1'
        elif enabled and a.pending_defer:assert m['profile']=='native-window-pending-defer-probe-v1'
        checkpoint=bool(enabled and (a.first_input or not a.pending_defer))
        assert q['corruption_controls']==8 and q['resealed_native_corruption_controls']==4+17*int(checkpoint)
        native_export=build.audit_register_checkpoint(b,q['symbols'],checkpoint)
        assert q.get('register_checkpoint')==native_export
        assert len(q['irq_veneer_audit'])==3 and all(x['exact_opcodes_verified'] for x in q['irq_veneer_audit'])
        assert q['native_data_bss']==304+8*int(pending) and q['workspace_end']-q['workspace_begin']==964
        assert q['conservative_stack_bound']<=0x8000-0x2100 and q['individual_frame_max']<=1024
        for f,h in m['inputs'].items():
            assert hashlib.sha256((repo/f).read_bytes().replace(b'\r\n',b'\n')).hexdigest()==h,f
        if enabled:
            assert q['image_sha256']!=expected and q['rom_used']>manifests[0]['profiles'][0]['rom_used']
        if enabled and (a.first_input or not a.pending_defer):
            at=q['symbols']['cs_mac_window_idle_spin']-0x400000
            if a.first_input:assert m['window_first_input_probe_limit']==5000
            else:assert m['window_idle_probe_threshold']==500
            assert b[at:at+2]==bytes.fromhex('60fe')
        elif not enabled:
            assert label not in q['symbols'] and q['image_sha256']==expected
        else:
            assert label not in q['symbols'] and q['image_sha256']!=expected
            assert q['rom_used']>manifests[0]['profiles'][0]['rom_used']
        profiles.append({'probe':enabled,'rom_sha256':q['image_sha256'],'rom_used':q['rom_used'],
            'native_data_bss':q['native_data_bss'],'stack_bound':q['conservative_stack_bound'],
            'resealed_controls':q['resealed_native_corruption_controls']})
    # Wrong modes must refuse before any output directory/compiler access.
    missing=Path(a.output).parent/('first-input-refusal-'+str(int(a.pending_defer)) if a.first_input else
                                 ('defer-refusal-must-not-exist' if a.pending_defer else 'idle-refusal-must-not-exist'))
    assert not missing.exists()
    modes=[[],['--drawing'],['--native-drawing'],['--selected-drawing'],['--outline-drawing']]
    flags=['--window-first-input-probe'] if a.first_input else []
    if a.pending_defer:flags.append('--window-pending-defer-probe')
    if not flags:flags=['--window-idle-probe']
    required_flag='--window-pending-defer-probe' if a.pending_defer else flags[0]
    for mode in modes:
        result=subprocess.run([sys.executable,'-B',str(repo/'scripts/Build-LoadedAppROM.py'),
            '--tool-root',str(missing),'--output',str(missing),*flags,*mode],
            capture_output=True,text=True)
        assert result.returncode==2 and required_flag+' requires --window-drawing' in result.stderr
        assert not missing.exists()
    if a.pending_defer or a.first_input:
        result=subprocess.run([sys.executable,'-B',str(repo/'scripts/Build-LoadedAppROM.py'),
            '--tool-root',str(missing),'--output',str(missing),'--window-drawing',
            '--window-idle-probe',*flags],capture_output=True,text=True)
        assert result.returncode==2 and 'Choose one diagnostic profile' in result.stderr and not missing.exists()
    d={'schema_version':1,'profiles':profiles,'normal_byte_identical_to_b3cb3fe':None if baseline_defer else True,
       'baseline_byte_identical_to_83f7b6f':True,'baseline_rom_sha256':expected,
       'current_source_hashes_verified':True,'mode_refusals':len(modes)+int(a.pending_defer or a.first_input),
       'probe_kind':('pending-defer-first-input' if a.pending_defer else 'first-input') if a.first_input else ('pending-defer' if a.pending_defer else 'idle-checkpoint'),
       'probe_spin_bytes':'60fe' if a.first_input or not a.pending_defer else None,'register_export_format':'MWR1' if a.first_input or not a.pending_defer else None,'additional_ram_bytes':8 if a.pending_defer and not a.first_input else 0,'trace_layout_bytes':128,
       'emulator_run':False,'motion_or_release_qualification':False}
    Path(a.output).write_text(json.dumps(d,indent=2)+'\n')
    print('PASS: baseline ROM unchanged, probe twins/'+str(q['corruption_controls']+q['resealed_native_corruption_controls'])+
          ' controls/three IRQ veneers, '+str(d['additional_ram_bytes'])+' added RAM bytes, '+str(d['mode_refusals'])+' refusals; no boot.')

if __name__=='__main__':main()
