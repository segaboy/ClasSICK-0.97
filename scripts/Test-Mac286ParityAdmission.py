#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Verify a paired run rejects changed Mac build identity before execution."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]


def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for option in ('emulator','rom','manifest','pc-results','passing-report','output'):
        p.add_argument('--'+option,required=True,type=Path)
    a=p.parse_args();exe=a.emulator.resolve(strict=True);rom=a.rom.resolve(strict=True)
    manifest=a.manifest.resolve(strict=True);pc=a.pc_results.resolve(strict=True)
    passing=a.passing_report.resolve(strict=True);out=a.output.resolve()
    if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside the repository')
    build=json.loads(manifest.read_text(encoding='utf-8'));prior=json.loads(passing.read_text(encoding='utf-8'))
    if (prior['status']!='pass' or prior['mac_rom_sha256']!=sha(rom) or
        prior['mac_manifest_sha256']!=sha(manifest) or prior['boothush_sha256']!=sha(exe)):
        raise ValueError('passing baseline identity mismatch')
    out.mkdir(parents=True,exist_ok=False);cases=[]
    expected={'image':'Mac manifest, profile or image mismatch',
              'source':'Mac source identity changed: core/graphics/surface.c',
              'profile':'Mac manifest, profile or image mismatch',
              'symbols':'Mac diagnostic symbols outside RAM'}
    for name in expected:
        bad=copy.deepcopy(build)
        if name=='image':bad['profiles'][0]['image_sha256']='0'*64
        elif name=='source':bad['inputs']['core/graphics/surface.c']='0'*64
        elif name=='profile':bad['dcd_invert']=0
        else:bad['profiles'][0]['symbols']['cs_mac_trace']=0x2100
        file=out/(name+'.json');file.write_text(json.dumps(bad,indent=2)+'\n',encoding='utf-8')
        dest=out/(name+'-run')
        cmd=[sys.executable,'-B',str(ROOT/'scripts/Test-BootHushMac286Parity.py'),
             '--emulator',str(exe),'--mac-rom',str(rom),'--mac-manifest',str(file),
             '--pc-results',str(pc),'--output',str(dest)]
        result=subprocess.run(cmd,capture_output=True,text=True,timeout=15,
                              creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        if result.returncode==0 or dest.exists() or expected[name] not in result.stderr:
            raise ValueError(name+': wrong rejection or execution began: '+result.stderr)
        cases.append({'name':name,'command':cmd,'exit_code':result.returncode,'stderr':result.stderr.strip()})
    report={'status':'pass','mac_rom_sha256':sha(rom),'manifest_sha256':sha(manifest),
            'passing_report_sha256':sha(passing),'cases':cases,
            'limits':'Input-admission controls only; not a release qualification.'}
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
