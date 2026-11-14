#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Verify all loaded bytes and third-track failure for the diagnostic loader."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
from pc286_loader import floppy,MAX_PAYLOAD

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--emulator',required=True,type=Path);p.add_argument('--output',required=True,type=Path);a=p.parse_args()
    try:
        exe=a.emulator.resolve(strict=True);out=a.output.resolve();root=Path(__file__).resolve().parents[1]
        if out==root or root in out.parents:raise ValueError('output must be outside the repository')
        for payload,entry in ((b'',0),(bytes(MAX_PAYLOAD+1),0),(b'abc',-1),(b'abc',3)):
            try:floppy(payload,entry)
            except ValueError:continue
            raise ValueError('invalid loader input accepted')
        out.mkdir(parents=True,exist_ok=False);cases=[]
        for size in (6,7168,7169,14848,14849,22016,22017,MAX_PAYLOAD):
            payload=bytes.fromhex('b0a4e680ebfe')+bytes((i*13+7)&255 for i in range(size-6))
            image=out/f'payload-{size}.img'
            with image.open('xb') as f:f.write(floppy(payload,0))
            before=sha(image)
            for fail in (False,True) if size==MAX_PAYLOAD else (False,):
                label=f'{size}'+('-third-fail' if fail else '')
                expected=payload[:14848]+bytes(size-14848) if fail else payload
                for run,start in enumerate(range(0,size,16384)):
                    block=expected[start:start+16384];cmd=[str(exe),'--machine','pc286','--floppy',str(image),
                        '--cycles','8000000','--timeout-ms','20000','--until-post','0xee' if fail else '0xa4']
                    if fail:cmd+=['--fail-floppy-read','1:0:1']
                    ranges=[]
                    for offset in range(0,len(block),4096):
                        n=min(4096,len(block)-offset);address=0x10000+start+offset
                        cmd+=['--inspect',f'{hex(address)}:{n}'];ranges.append((address,block[offset:offset+n]))
                    result=subprocess.run(cmd,capture_output=True,text=True,timeout=25,
                        creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
                    if result.returncode:raise ValueError(label+': '+result.stdout+result.stderr)
                    data=json.loads(result.stdout)
                    rows=data.get('memory_ranges',[]) if len(ranges)>1 else [data.get('memory',{})]
                    if (data.get('status')!='condition_met' or data.get('post')!=(0xee if fail else 0xa4) or
                        data.get('cs')!=(0 if fail else 0x1000) or len(rows)!=len(ranges) or sha(image)!=before):
                        raise ValueError(label+': bad stop or changed image')
                    if any(row.get('address')!=address or bytes.fromhex(row.get('bytes',''))!=content
                           for row,(address,content) in zip(rows,ranges)):
                        raise ValueError(label+': loaded bytes differ')
                    if fail and data['floppy_read_failure']['injected']!=1:raise ValueError('failure was not injected')
                    (out/f'{label}-{run}.json').write_text(json.dumps({'command':cmd,'result':data},indent=2)+'\n',encoding='utf-8')
                cases.append({'size':size,'third_track_failure':fail,'floppy_sha256':before,
                              'all_loaded_bytes_match':True,'floppy_unchanged':True})
        report={'status':'pass','boothush_sha256':sha(exe),'max_payload':MAX_PAYLOAD,
                'invalid_inputs_rejected':4,'cases':cases,
                'limits':'Diagnostic floppy; not a production bootloader, install, or physical test.'}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
        print(json.dumps(report,indent=2));return 0
    except (OSError,ValueError,KeyError,subprocess.TimeoutExpired) as exc:
        print('three-track loader test failed: '+str(exc),file=sys.stderr);return 1

if __name__=='__main__':sys.exit(main())
