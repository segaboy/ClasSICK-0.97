# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Compile and execute original FIFO/deferred-motion contracts on the host."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

ROOT=Path(__file__).resolve().parents[1]
SOURCES=('tests/mac-buffered-capture.c','tests/mac-motion.c',
         'platform/mac128k/motion.c','platform/mac128k/motion.h',
         'platform/mac128k/capture-queue.h','apps/window-scene/inbox.c',
         'apps/window-scene/inbox.h')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--cc',type=Path,required=True);p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();cc=a.cc.resolve(strict=True);out=a.output.resolve()
    if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside repository')
    out.mkdir(parents=True,exist_ok=False);cases=[]
    for opt in ('-O0','-O2'):
        exe=out/(opt[1:]+'.exe')
        command=[str(cc),'-std=c11',opt,'-Wall','-Wextra','-Werror',
                 '-Wconversion','-Wsign-conversion','-DCLASSICK_MAC_FAST_COLLECT=1',
                 '-DCLASSICK_MAC_SHORT_COLLECT=1','-DCLASSICK_MAC_LOSS_COUNTS=1',
                 str(ROOT/SOURCES[0]),str(ROOT/SOURCES[2]),str(ROOT/SOURCES[5]),'-o',str(exe)]
        commands=[command,[str(exe)]];logs=[]
        for cmd in commands:
            r=subprocess.run(cmd,capture_output=True,text=True,timeout=30,
                             creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            logs.append(r.stdout+r.stderr)
            (out/(opt[1:]+'.log')).write_text('\n'.join(logs))
            if r.returncode:raise RuntimeError(r.stdout+r.stderr)
        cases.append({'optimization':opt,'commands':commands,'output':logs[-1], 'exe_sha256':sha(exe)})
    report={'status':'pass','cases':cases,'compiler_sha256':sha(cc),
            'source_sha256':{n:sha(ROOT/n) for n in SOURCES},'scope':'host contracts; no target timing claim'}
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({'status':'pass','profiles':len(cases),'report':str(out/'report.json')}))
if __name__=='__main__':main()
