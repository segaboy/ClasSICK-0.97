#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Hosted scan decoder regression with a deliberate repeat defect."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
SOURCE=ROOT/'platform/pc286/keyboard-set2.c'
TEST=ROOT/'tests/pc286-keyboard-set2.c'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--cc',required=True,type=Path);p.add_argument('--output',required=True,type=Path);a=p.parse_args()
    try:
        cc=a.cc.resolve(strict=True);out=a.output.resolve()
        if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside the repository')
        source=SOURCE.read_text(encoding='utf-8');needle='if(release!=s->held)return 0;'
        if source.count(needle)!=2:raise ValueError('mutation point changed')
        out.mkdir(parents=True,exist_ok=False);mutant=out/'bad-repeat.c'
        mutant.write_text(source.replace(needle,'/* intentional repeat defect */'),encoding='utf-8')
        observations=[]
        for name,src in (('baseline',SOURCE),('bad-repeat',mutant)):
            binary=out/(name+'.exe')
            cmd=[str(cc),'-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-Wsign-conversion',
                 '-I',str(ROOT/'platform/pc286'),str(TEST),str(src),'-o',str(binary)]
            built=subprocess.run(cmd,capture_output=True,text=True,timeout=30,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            if built.returncode:raise ValueError(name+' compile: '+built.stdout+built.stderr)
            run=subprocess.run([str(binary)],capture_output=True,text=True,timeout=10,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            if (run.returncode==0)!=(name=='baseline'):raise ValueError(name+' unexpected result')
            observations.append({'name':name,'command':cmd,'binary_sha256':sha(binary),'exit_code':run.returncode,'stdout':run.stdout})
        report={'status':'pass','cc_sha256':sha(cc),'source_sha256':sha(SOURCE),'test_sha256':sha(TEST),
                'mutant_sha256':sha(mutant),'cases':observations,
                'limits':'Hosted software test, not a physical keyboard or controller qualification.'}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(report,indent=2));return 0
    except (OSError,ValueError,subprocess.TimeoutExpired) as exc:
        print('keyboard scan regression failed: '+str(exc),file=sys.stderr);return 1

if __name__=='__main__':sys.exit(main())
