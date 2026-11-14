#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Exercise the 286 controller query and three test-sensitivity controls."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

ROOT=Path(__file__).resolve().parents[1]


def digest(data):return hashlib.sha256(data).hexdigest()


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--cc',required=True,type=Path);p.add_argument('--output',required=True,type=Path)
    a=p.parse_args();cc=a.cc.resolve(strict=True);out=a.output.resolve()
    if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside the repository')
    source=ROOT/'platform/pc286/keyboard-config.c';header=source.with_suffix('.h')
    test=ROOT/'tests/pc286-keyboard-config.c';original=source.read_text(encoding='utf-8')
    out.mkdir(parents=True,exist_ok=False)
    mutations=(('baseline',None,None,0),('pending','if(status&1u)', 'if(0)',5),
               ('timeout','s->polls>=s->limit','s->polls>s->limit',6),
               ('disabled-keyboard','(value&0x50u)==0x40u','(value&0x40u)!=0u',4))
    cases=[]
    for name,old,new,expected in mutations:
        text=original
        if old is not None:
            if text.count(old)!=1:raise ValueError('mutation site is not unique: '+name)
            text=text.replace(old,new,1)
        candidate=out/(name+'.c');candidate.write_text(text,encoding='utf-8')
        binary=out/(name+'.exe')
        command=[str(cc),'-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion',
                 '-Wsign-conversion','-I',str(header.parent),str(test),str(candidate),'-o',str(binary)]
        compile_result=subprocess.run(command,capture_output=True,text=True,timeout=30,
                                      creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        if compile_result.returncode:raise RuntimeError(compile_result.stdout+compile_result.stderr)
        run=subprocess.run([str(binary)],capture_output=True,text=True,timeout=10,
                           creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        if run.returncode!=expected:
            raise AssertionError((name,run.returncode,run.stdout,run.stderr))
        cases.append({'name':name,'source_sha256':digest(candidate.read_bytes()),
                      'compile_command':command,'exit_code':run.returncode,'stdout':run.stdout,'stderr':run.stderr})
    report={'status':'pass','compiler_sha256':digest(cc.read_bytes()),
            'runner_sha256':digest(Path(__file__).read_bytes()),'source_sha256':digest(source.read_bytes()),
            'test_sha256':digest(test.read_bytes()),'header_sha256':digest(header.read_bytes()),
            'cases':cases,'limits':'Hosted software checks only; no physical controller claim.'}
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
