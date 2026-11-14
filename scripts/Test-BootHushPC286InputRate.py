#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Characterize queued keyboard sequences against the 286 window diagnostic."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
ROOT=Path(__file__).resolve().parents[1]
KEYS=(0x29,0x74,0x74,0x75,0x29,0x74,0x72,0x72,0x72,0x29,0x29)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--emulator',required=True,type=Path);p.add_argument('--pc-results',required=True,type=Path)
    p.add_argument('--output',required=True,type=Path);a=p.parse_args()
    try:
        exe=a.emulator.resolve(strict=True);pc=a.pc_results.resolve(strict=True);out=a.output.resolve()
        if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside the repository')
        report=json.loads((pc/'report.json').read_text(encoding='utf-8'));image=pc/'session.img'
        if report['status']!='pass' or sha(exe)!=report['boothush_sha256'] or sha(image)!=report['floppy_sha256']:
            raise ValueError('input identity mismatch')
        for name,digest in report['source_sha256'].items():
            if sha(ROOT/name)!=digest:raise ValueError('source changed: '+name)
        expected=next(c for c in report['cases'] if c['name']=='content-release')
        out.mkdir(parents=True,exist_ok=False);cases=[]
        for period in (5000000,1000000,500000,250000):
            frame=out/f'{period}.ppm';cmd=[str(exe),'--machine','pc286','--floppy',str(image),
                '--cycles','100000000','--timeout-ms','20000','--until-post','0xbb',
                '--inspect','0x60000:17','--frame',str(frame)]
            for i,key in enumerate(KEYS):
                cycle=6000000+i*period
                for when,code in ((cycle,key),(cycle+100000,0xf0),(cycle+101000,key)):
                    cmd+=['--scancode',f'{when}:{code}']
            result=subprocess.run(cmd,capture_output=True,text=True,timeout=25,
                                  creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            (out/f'{period}.log').write_text(result.stdout+'\n--- stderr ---\n'+result.stderr,encoding='utf-8')
            data=json.loads(result.stdout);frame_hash=sha(frame) if frame.exists() else None
            match=(result.returncode==0 and data.get('status')=='condition_met' and data.get('post')==0xbb and
                   data.get('memory')==expected['guest_result']['memory'] and frame_hash==expected['ega_ppm_sha256'])
            if sha(image)!=report['floppy_sha256']:raise ValueError('input image changed')
            if period==5000000 and not match:raise ValueError('baseline control failed')
            cases.append({'period_cycles':period,'exit_code':result.returncode,'status':data.get('status'),
                          'stop_cycle':data.get('cycles'),'post':data.get('post'),'memory':data.get('memory'),
                          'frame_sha256':frame_hash,'expected_final_state_and_frame':match,'command':cmd})
        summary={'status':'characterization','boothush_sha256':sha(exe),'floppy_sha256':sha(image),
                 'input_count':len(KEYS),'cases':cases,
                 'limits':'Injected emulator keyboard events; no human, physical timing or release qualification.'}
        (out/'report.json').write_text(json.dumps(summary,indent=2)+'\n',encoding='utf-8');print(json.dumps(summary,indent=2));return 0
    except (OSError,ValueError,KeyError,subprocess.TimeoutExpired) as exc:
        print('286 input-rate characterization failed: '+str(exc),file=sys.stderr);return 1

if __name__=='__main__':sys.exit(main())
