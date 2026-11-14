#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Observe source-matched Mac startup stages at bounded cycle checkpoints."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

ROOT=Path(__file__).resolve().parents[1]


def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for opt in ('emulator','rom','manifest','output'):p.add_argument('--'+opt,required=True,type=Path)
    a=p.parse_args();exe=a.emulator.resolve(strict=True);rom=a.rom.resolve(strict=True)
    manifest=a.manifest.resolve(strict=True);out=a.output.resolve()
    if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside the repository')
    build=json.loads(manifest.read_text(encoding='utf-8'));profile=build['profiles'][0]
    if not build.get('twins_equal') or not build.get('window_drawing') or profile['image_sha256']!=sha(rom):
        raise ValueError('Mac image or manifest mismatch')
    for name,expected in build['inputs'].items():
        path=(ROOT/name).resolve(strict=True)
        if ROOT not in path.parents or hashlib.sha256(path.read_bytes().replace(b'\r\n',b'\n')).hexdigest()!=expected:
            raise ValueError('Mac source identity mismatch: '+name)
    diag=profile['symbols']['cs_mac_diag']
    if not 0x400<=diag<=0x2000-28:raise ValueError('diagnostics outside expected RAM')
    out.mkdir(parents=True,exist_ok=False);series=out/'frames.pbmseq'
    cmd=[str(exe),'--machine','mac128k','--rom',str(rom),'--cycles','100000000','--timeout-ms','20000',
         '--capture-series',str(series),'--capture-memory',hex(diag)+':28']
    for cycle in range(5000000,100000001,5000000):cmd+=['--capture-cycle',str(cycle)]
    result=subprocess.run(cmd,capture_output=True,text=True,timeout=25,
                          creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
    (out/'run.log').write_text(result.stdout+'\n--- stderr ---\n'+result.stderr,encoding='utf-8')
    if result.returncode:raise RuntimeError('startup run failed; see run.log')
    data=json.loads(result.stdout);capture=data['frame_series']
    if (data['status']!='cycle_limit' or capture['rom_sha256']!=sha(rom) or
        (capture['requested'],capture['processed'],capture['pending'],capture['dropped'])!=(20,20,0,0)):
        raise ValueError('incomplete startup observation')
    stages=[]
    for s in capture['samples']:
        b=bytes.fromhex(s['memory']['bytes'])
        if len(b)!=28 or s['memory']['address']!=diag:raise ValueError('diagnostic capture mismatch')
        magic=int.from_bytes(b[:4],'big');stage=int.from_bytes(b[4:8],'big');fault=int.from_bytes(b[24:28],'big')
        if magic not in (0,0x43533039) or fault:raise ValueError('unexpected guest diagnostic')
        stages.append({'requested':s['requested'],'cycle':s['cycle'],'stage':stage,'fault':fault})
    if stages[-1]['stage']!=16:raise ValueError('guest not ready by final checkpoint')
    ready_cmd=[str(exe),'--machine','mac128k','--rom',str(rom),'--cycles','100000000','--timeout-ms','20000',
               '--until',hex(diag+4)+':4:16','--inspect',hex(diag)+':28']
    ready_run=subprocess.run(ready_cmd,capture_output=True,text=True,timeout=25,
                             creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
    (out/'ready.log').write_text(ready_run.stdout+'\n--- stderr ---\n'+ready_run.stderr,encoding='utf-8')
    if ready_run.returncode:raise RuntimeError('ready stop failed; see ready.log')
    ready=json.loads(ready_run.stdout);b=bytes.fromhex(ready['memory']['bytes'])
    if (ready['status']!='condition_met' or len(b)!=28 or b[:4]!=b'CS09' or
        int.from_bytes(b[4:8],'big')!=16 or int.from_bytes(b[24:28],'big')!=0):
        raise ValueError('unexpected ready-stage stop')
    first=next(i for i,s in enumerate(stages) if s['stage']==16)
    if first and not stages[first-1]['cycle']<ready['cycles']<=stages[first]['cycle']:
        raise ValueError('ready stop outside sampled bracket')
    report={'status':'pass','rom_sha256':sha(rom),'manifest_sha256':sha(manifest),'emulator_sha256':sha(exe),
            'script_sha256':sha(Path(__file__)),'optimization':profile['optimization'],
            'command':cmd,'stages':stages,'ready_observation':{'command':ready_cmd,'result':ready},
            'limits':'Instruction-boundary modeled cycles; no physical performance claim.'}
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(report,indent=2))


if __name__=='__main__':main()
