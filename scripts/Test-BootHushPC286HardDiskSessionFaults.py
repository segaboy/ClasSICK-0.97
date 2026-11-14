#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Inject modeled BIOS failures into each read of a linked 286 session."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--emulator', required=True, type=Path)
    p.add_argument('--session', required=True, type=Path)
    p.add_argument('--output', required=True, type=Path)
    a = p.parse_args();exe = a.emulator.resolve(strict=True)
    session = a.session.resolve(strict=True);out = a.output.resolve()
    if out == ROOT or ROOT in out.parents:
        raise ValueError('output must be outside repository')
    report_path = session/'report.json';prior = json.loads(report_path.read_text(encoding='utf-8'))
    disk, payload = session/'session.img', session/'session.bin'
    if (prior.get('status') != 'pass' or prior.get('media') != 'hard-disk' or
        prior.get('payload_bytes') != 22374 or prior.get('media_sha256') != sha(disk) or
        prior.get('payload_sha256') != sha(payload)):
        raise ValueError('session identity or expected read layout mismatch')
    for name,digest in prior['source_sha256'].items():
        if sha(ROOT/name) != digest:
            raise ValueError('session source changed: '+name)
    disk_sha, payload_sha, exe_sha = sha(disk), sha(payload), sha(exe)
    data = payload.read_bytes();out.mkdir(parents=True, exist_ok=False);cases=[]
    for chs, offset in (('0:0:2',0), ('0:1:1',16*512), ('0:2:1',33*512)):
        offsets = ([offset-1] if offset else [])+[offset]
        cmd = [str(exe),'--machine','pc286','--hard-disk',str(disk),'--cycles','8000000',
               '--timeout-ms','20000','--until-post','0xee','--fail-hard-disk-read',chs]
        for address in offsets:cmd += ['--inspect',hex(0x10000+address)+':1']
        result = subprocess.run(cmd,capture_output=True,text=True,timeout=25,
                                creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        if result.returncode:raise RuntimeError(result.stdout+result.stderr)
        response = json.loads(result.stdout);fault=response['hard_disk_read_failure']
        rows = response['memory_ranges'] if len(offsets)>1 else [response['memory']]
        expected = ([data[offset-1]] if offset else [])+[0]
        if (response['status']!='condition_met' or response['post']!=0xee or
            fault['injected']!=1 or fault['bios_status']!=32 or
            [fault[x] for x in ('cylinder','head','sector')]!=[int(x) for x in chs.split(':')] or
            [row['address'] for row in rows]!=[0x10000+x for x in offsets] or
            [bytes.fromhex(row['bytes'])[0] for row in rows]!=expected):
            raise ValueError('unexpected loader failure behavior at '+chs)
        cases.append({'chs':chs,'command':cmd,'result':response})
    if sha(disk)!=disk_sha or sha(payload)!=payload_sha or sha(exe)!=exe_sha:
        raise ValueError('input changed')
    result={'status':'pass','disk_sha256':disk_sha,'payload_sha256':payload_sha,
            'baseline_report_sha256':sha(report_path),'emulator_sha256':exe_sha,
            'script_sha256':sha(Path(__file__)),'cases':cases,
            'limits':'Injected built-in BIOS failures on an original diagnostic, not physical disk or release qualification.'}
    (out/'report.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({'status':'pass','case_count':len(cases),'report':str(out/'report.json')}))


if __name__=='__main__':main()
