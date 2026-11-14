#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Measure instrumented scene and presentation boundaries for one drag."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--emulator',required=True,type=Path);p.add_argument('--pc-results',required=True,type=Path)
    p.add_argument('--output',required=True,type=Path);a=p.parse_args()
    try:
        exe=a.emulator.resolve(strict=True);pc=a.pc_results.resolve(strict=True);out=a.output.resolve()
        if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside the repository')
        report=json.loads((pc/'report.json').read_text(encoding='utf-8'));image=pc/'session.img'
        if (report['status']!='pass' or not report.get('timing_markers') or
            sha(exe)!=report['boothush_sha256'] or sha(image)!=report['floppy_sha256']):
            raise ValueError('instrumented input identity mismatch')
        for name,digest in report['source_sha256'].items():
            if sha(ROOT/name)!=digest:raise ValueError('source changed: '+name)
        prior=next(c for c in report['cases'] if c['name']=='pressed')['guest_result']['memory']
        out.mkdir(parents=True,exist_ok=False);samples=[]
        for marker in (0xc1,0xd1,0xe1):
            cmd=[str(exe),'--machine','pc286','--floppy',str(image),'--cycles','20000000','--timeout-ms','20000',
                 '--until-post',hex(marker),'--inspect','0x60000:17']
            for cycle,key in ((6000000,0x29),(11000000,0x74)):
                for when,code in ((cycle,key),(cycle+100000,0xf0),(cycle+101000,key)):
                    cmd+=['--scancode',f'{when}:{code}']
            run=subprocess.run(cmd,capture_output=True,text=True,timeout=25,
                               creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            if run.returncode:raise ValueError('marker failed: '+run.stdout+run.stderr)
            data=json.loads(run.stdout)
            if (data.get('status')!='condition_met' or data.get('post')!=marker or
                data.get('memory')!=prior or sha(image)!=report['floppy_sha256']):raise ValueError('wrong marker or state')
            samples.append({'marker':marker,'cycles':data['cycles'],'command':cmd})
        cycles=[s['cycles'] for s in samples]
        if not cycles[0]<cycles[1]<cycles[2]:raise ValueError('timing order incorrect')
        summary={'status':'pass','boothush_sha256':sha(exe),'floppy_sha256':sha(image),
                 'samples':samples,'shared_sample_cycles':cycles[1]-cycles[0],
                 'ega_publication_cycles':cycles[2]-cycles[1],
                 'limits':'Instrumented modeled CPU and devices; interval includes marker overhead; not physical performance.'}
        (out/'report.json').write_text(json.dumps(summary,indent=2)+'\n',encoding='utf-8');print(json.dumps(summary,indent=2));return 0
    except (OSError,ValueError,KeyError,subprocess.TimeoutExpired) as exc:
        print('286 timing failed: '+str(exc),file=sys.stderr);return 1

if __name__=='__main__':sys.exit(main())
