#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Measure touched stack bytes for one bounded 286 window sequence."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
from pc286_loader import floppy

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
        report=json.loads((pc/'report.json').read_text(encoding='utf-8'));payload=pc/'session.bin'
        if report['status']!='pass' or sha(exe)!=report['boothush_sha256'] or sha(payload)!=report['payload_sha256']:
            raise ValueError('input identities do not match')
        for name,digest in report['source_sha256'].items():
            if sha(ROOT/name)!=digest:raise ValueError('source changed: '+name)
        mapping=(pc/'session.map').read_text(encoding='utf-8',errors='replace')
        entry=re.search(r'Entry point address:\s*([0-9a-fA-F]+)',mapping)
        if not entry:raise ValueError('missing entry')
        bss_end=report['bss_start']+report['bss_bytes'];reference=next(c for c in report['cases'] if c['name']=='content-release')
        out.mkdir(parents=True,exist_ok=False);results=[]
        for value in (0x5a,0xa5):
            image=out/f'stack-{value:02x}.img'
            with image.open('xb') as f:f.write(floppy(payload.read_bytes(),int(entry.group(1),16),
                                                       bss_end=bss_end,poison_bss=True,poison_stack=value))
            before=sha(image);stack=bytearray();cycles=[]
            for run,start in enumerate(range(bss_end,0xfffe,16384)):
                n=min(16384,0xfffe-start);frame=out/f'frame-{value:02x}-{run}.ppm'
                cmd=[str(exe),'--machine','pc286','--floppy',str(image),'--cycles','100000000',
                     '--timeout-ms','20000','--until-post','0xbb','--frame',str(frame)]
                ranges=[]
                for off in range(0,n,4096):
                    size=min(4096,n-off);address=0x10000+start+off
                    cmd+=['--inspect',f'{hex(address)}:{size}'];ranges.append((address,size))
                for i,key in enumerate(KEYS):
                    cycle=6000000+i*5000000
                    for when,code in ((cycle,key),(cycle+100000,0xf0),(cycle+101000,key)):
                        cmd+=['--scancode',f'{when}:{code}']
                result=subprocess.run(cmd,capture_output=True,text=True,timeout=25,
                                      creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
                if result.returncode:raise ValueError('stack run failed: '+result.stdout+result.stderr)
                data=json.loads(result.stdout);rows=data.get('memory_ranges',[]) if len(ranges)>1 else [data.get('memory',{})]
                if (data.get('status')!='condition_met' or data.get('post')!=0xbb or sha(image)!=before or
                    sha(frame)!=reference['ega_ppm_sha256'] or len(rows)!=len(ranges)):
                    raise ValueError('stack run image, stop or frame mismatch')
                for row,(address,size) in zip(rows,ranges):
                    content=bytes.fromhex(row.get('bytes',''))
                    if row.get('address')!=address or len(content)!=size:raise ValueError('stack inspection mismatch')
                    stack.extend(content)
                cycles.append(data['cycles'])
                (out/f'run-{value:02x}-{run}.json').write_text(json.dumps({'command':cmd,'result':data},indent=2)+'\n',encoding='utf-8')
            changed=[i for i,byte in enumerate(stack) if byte!=value]
            if not changed:raise ValueError('stack instrumentation saw no writes')
            lowest=bss_end+min(changed)
            if lowest<=bss_end:raise ValueError('observed stack reached BSS boundary')
            results.append({'poison':value,'floppy_sha256':before,'lowest_changed_offset':lowest,
                            'observed_span_bytes':0xfffe-lowest,'untouched_prefix_bytes':lowest-bss_end,'cycles':cycles})
        summary={'status':'pass','boothush_sha256':sha(exe),'payload_sha256':sha(payload),
                 'bss_end':bss_end,'initial_stack_pointer':0xfffe,'runs':results,
                 'limits':'Only an observed lower bound for this modeled sequence; same-value writes can evade canaries; no hardware, worst-case or release claim.'}
        (out/'report.json').write_text(json.dumps(summary,indent=2)+'\n',encoding='utf-8');print(json.dumps(summary,indent=2));return 0
    except (OSError,ValueError,KeyError,subprocess.TimeoutExpired) as exc:
        print('286 stack test failed: '+str(exc),file=sys.stderr);return 1

if __name__=='__main__':sys.exit(main())
