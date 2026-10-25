# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Check original compile-only window diagnostic proofs; never starts a guest."""
import argparse,hashlib,json,subprocess,sys
from pathlib import Path

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--normal',required=True);p.add_argument('--probe',required=True)
    p.add_argument('--output',required=True)
    p.add_argument('--pending-defer',action='store_true',help='Verify the opt-in finite-defer experiment instead of the idle spin')
    a=p.parse_args();repo=Path(__file__).resolve().parents[1]
    roots=[Path(a.normal),Path(a.probe)]
    manifests=[json.loads((r/'manifest.json').read_text()) for r in roots]
    expected='673a2cca57d1ec9ffd842f4c858fa4aafbcb042ec6f86445ae5717d4816c4965'
    label='cs_mac_window_idle_checkpoint';profiles=[]
    for enabled,r,m in zip((False,True),roots,manifests):
        q=m['profiles'][0];b=(r/'Os-a/classick-loaded-app.rom').read_bytes()
        assert len(b)==65536 and hashlib.sha256(b).hexdigest()==q['image_sha256']
        assert m['window_drawing'] and m['window_idle_probe']==(enabled and not a.pending_defer) and m['twins_equal']
        assert m.get('window_pending_defer_probe',False)==(enabled and a.pending_defer)
        if enabled and a.pending_defer:assert m['profile']=='native-window-pending-defer-probe-v1'
        assert q['corruption_controls']==8 and q['resealed_native_corruption_controls']==4+int(enabled and not a.pending_defer)
        assert len(q['irq_veneer_audit'])==3 and all(x['exact_opcodes_verified'] for x in q['irq_veneer_audit'])
        assert q['native_data_bss']==304+8*int(enabled and a.pending_defer) and q['workspace_end']-q['workspace_begin']==964
        assert q['conservative_stack_bound']<=0x8000-0x2100 and q['individual_frame_max']<=1024
        for f,h in m['inputs'].items():
            assert hashlib.sha256((repo/f).read_bytes().replace(b'\r\n',b'\n')).hexdigest()==h,f
        if enabled and not a.pending_defer:
            at=q['symbols'][label]-0x400000
            assert m['window_idle_probe_threshold']==500 and b[at:at+2]==bytes.fromhex('60fe')
        elif not enabled:
            assert label not in q['symbols'] and q['image_sha256']==expected
        else:
            assert label not in q['symbols'] and q['image_sha256']!=expected
            assert q['rom_used']>manifests[0]['profiles'][0]['rom_used']
        profiles.append({'probe':enabled,'rom_sha256':q['image_sha256'],'rom_used':q['rom_used'],
            'native_data_bss':q['native_data_bss'],'stack_bound':q['conservative_stack_bound'],
            'resealed_controls':q['resealed_native_corruption_controls']})
    # Wrong modes must refuse before any output directory/compiler access.
    missing=Path(a.output).parent/('defer-refusal-must-not-exist' if a.pending_defer else 'idle-refusal-must-not-exist')
    assert not missing.exists()
    modes=[[],['--drawing'],['--native-drawing'],['--selected-drawing'],['--outline-drawing']]
    flag='--window-pending-defer-probe' if a.pending_defer else '--window-idle-probe'
    for mode in modes:
        result=subprocess.run([sys.executable,'-B',str(repo/'scripts/Build-LoadedAppROM.py'),
            '--tool-root',str(missing),'--output',str(missing),flag,*mode],
            capture_output=True,text=True)
        assert result.returncode==2 and flag+' requires --window-drawing' in result.stderr
        assert not missing.exists()
    if a.pending_defer:
        result=subprocess.run([sys.executable,'-B',str(repo/'scripts/Build-LoadedAppROM.py'),
            '--tool-root',str(missing),'--output',str(missing),'--window-drawing',
            '--window-idle-probe',flag],capture_output=True,text=True)
        assert result.returncode==2 and 'Choose one diagnostic profile' in result.stderr and not missing.exists()
    d={'schema_version':1,'profiles':profiles,'normal_byte_identical_to_b3cb3fe':True,
       'current_source_hashes_verified':True,'mode_refusals':len(modes)+int(a.pending_defer),
       'probe_kind':'pending-defer' if a.pending_defer else 'idle-checkpoint',
       'probe_spin_bytes':None if a.pending_defer else '60fe','additional_ram_bytes':8 if a.pending_defer else 0,'trace_layout_bytes':128,
       'emulator_run':False,'motion_or_release_qualification':False}
    Path(a.output).write_text(json.dumps(d,indent=2)+'\n')
    print('PASS: normal ROM unchanged, probe twins/'+('12' if a.pending_defer else '13')+
          ' controls/three IRQ veneers, '+str(d['additional_ram_bytes'])+' added RAM bytes, '+str(d['mode_refusals'])+' refusals; no boot.')

if __name__=='__main__':main()
