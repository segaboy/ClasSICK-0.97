#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Compare the untouched shared cursor position on Mac and 286."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
MAC_SHA='faf37c6f96f8eb2ee6cf7a1e58a8b1b9538055d2fc89493c7419eeea816f6c8c'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--emulator',required=True,type=Path);p.add_argument('--mac-rom',required=True,type=Path)
    p.add_argument('--pc-results',required=True,type=Path);p.add_argument('--output',required=True,type=Path);a=p.parse_args()
    try:
        exe=a.emulator.resolve(strict=True);rom=a.mac_rom.resolve(strict=True)
        pc=a.pc_results.resolve(strict=True);out=a.output.resolve()
        if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside the repository')
        report=json.loads((pc/'report.json').read_text(encoding='utf-8'));image=pc/'session.img'
        if (report['status']!='pass' or not report.get('natural_start') or report.get('timing_markers') or
            sha(rom)!=MAC_SHA or sha(exe)!=report['boothush_sha256'] or sha(image)!=report['floppy_sha256']):
            raise ValueError('input identity mismatch')
        for name,digest in report['source_sha256'].items():
            if sha(ROOT/name)!=digest:raise ValueError('source changed: '+name)
        spec=importlib.util.spec_from_file_location('oracle',ROOT/'scripts/Test-BootHushPC286SharedCompositor.py')
        oracle=importlib.util.module_from_spec(spec);spec.loader.exec_module(oracle)
        out.mkdir(parents=True,exist_ok=False);frame=out/'mac.pbm'
        cmd=[str(exe),'--machine','mac128k','--rom',str(rom),'--cycles','100000000','--timeout-ms','20000',
             '--inspect','0xc04:192','--frame',str(frame)]
        run=subprocess.run(cmd,capture_output=True,text=True,timeout=25,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        (out/'run.log').write_text(run.stdout+'\n--- stderr ---\n'+run.stderr,encoding='utf-8')
        if run.returncode:raise ValueError('Mac run failed')
        data=json.loads(run.stdout);memory=bytes.fromhex(data['memory']['bytes'])
        def word(address):return int.from_bytes(memory[address-0xc04:address-0xc04+4],'big')
        state={'pointer':[word(0xc5c),word(0xc60)],'window':[word(0xc64),word(0xc68)],
               'pattern':word(0xc6c),'held':word(0xc80),'presses':word(0xcb4),
               'losses':[word(0xc04+i*4) for i in range(1,5)],'omissions':word(0xc1c),
               'stage':word(0xcac),'fault':word(0xcc0)}
        expected={'pointer':[256,171],'window':[80,70],'pattern':0,'held':0,'presses':0,
                  'losses':[0,0,0,0],'omissions':0,'stage':16,'fault':0}
        if data['status']!='cycle_limit' or state!=expected:raise ValueError('unexpected Mac startup: '+str(state))
        if frame.read_bytes()!=(pc/'initial.pbm').read_bytes():raise ValueError('Mac and hosted frames differ')
        oracle.compare(frame.read_bytes(),(pc/'initial.ppm').read_bytes(),margin_white=True)
        summary={'status':'pass','boothush_sha256':sha(exe),'mac_rom_sha256':sha(rom),
                 'pc_payload_sha256':report['payload_sha256'],'pc_floppy_sha256':sha(image),
                 'mac_cycles':data['cycles'],'mac_state':state,'mac_frame_sha256':sha(frame),
                 'pc_frame_sha256':sha(pc/'initial.ppm'),'canvas_pixels_match':512*342,
                 'margin_pixels_match':640*350-512*342,'command':cmd,
                 'limits':'No input, distinct source revisions and modeled devices; no physical or release claim.'}
        (out/'report.json').write_text(json.dumps(summary,indent=2)+'\n',encoding='utf-8');print(json.dumps(summary,indent=2));return 0
    except (OSError,ValueError,KeyError,subprocess.TimeoutExpired) as exc:
        print('Mac/286 natural start failed: '+str(exc),file=sys.stderr);return 1

if __name__=='__main__':sys.exit(main())
