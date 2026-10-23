#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Isolated RAM bootstrap compiler/layout audit; no hardware binding or run."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess

def main():
    p=argparse.ArgumentParser();p.add_argument('--tool-root',required=True,type=Path);p.add_argument('--output',required=True,type=Path)
    a=p.parse_args();repo=Path(__file__).resolve().parents[1];a.output.mkdir(parents=True,exist_ok=False)
    bins=a.tool_root/'usr/bin';cc=bins/'m68k-linux-gnu-gcc-13';nm=bins/'m68k-linux-gnu-nm'
    env=os.environ.copy();env['LD_LIBRARY_PATH']=str(a.tool_root/'usr/lib/x86_64-linux-gnu')
    def run(args):return subprocess.check_output([str(x) for x in args],env=env,text=True,stderr=subprocess.STDOUT,timeout=60)
    flags=['-mcpu=68000','-std=c11','-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-ident',
           '-Wall','-Wextra','-Wpedantic','-Werror','-Wconversion','-Wsign-conversion',
           '-ffile-prefix-map='+str(repo)+'=.','-ffile-prefix-map='+str(a.output)+'=.']
    layout=a.output/'layout.c';layout.write_text('#include "'+str(repo/'platform/mac128k/window-session.h')+'"\nconst unsigned char cs_mac_ram_layout[sizeof(cs_mac_window_session)]={0};\n')
    lo=a.output/'layout.o';run([cc,*flags,'-O2','-c',layout,'-o',lo])
    symbols=run([nm,'-S','--defined-only','--format=posix',lo]);m=re.search(r'^cs_mac_ram_layout R \S+ (\S+)$',symbols,re.M)
    if not m or not 0<int(m[1],16)<=1536:raise ValueError('Missing/excessive RAM descriptor')
    size=int(m[1],16);allowed={'cs_surface_init','cs_cursor_init','cs_cursor_bind','cs_window_input_begin','cs_foreground_begin'}
    rows=[]
    for opt in ('0','2','s'):
        hashes=[];frames=[]
        for twin in ('a','b'):
            folder=a.output/f'O{opt}-{twin}';folder.mkdir();obj=folder/'bootstrap.o'
            run([cc,*flags,'-O'+opt,'-fstack-usage','-Wframe-larger-than=512','-c',repo/'platform/mac128k/window-session.c','-o',obj])
            imports={v.split()[-1] for v in run([nm,'-u',obj]).splitlines()}
            defined=run([nm,'--defined-only','--format=posix',obj])
            if imports!=allowed or re.search(r'^\S+ [BbCcDdGgSsVv] ',defined,re.M):raise ValueError('Unexpected bootstrap imports/data')
            if not re.search(r'^cs_mac_window_begin T ',defined,re.M):raise ValueError('Missing bootstrap API')
            for line in obj.with_suffix('.su').read_text().splitlines():
                name,n,kind=line.rsplit('\t',2)
                if kind not in ('static','dynamic,bounded') or int(n)>512:raise ValueError('Unbounded/excessive frame')
                if twin=='a':frames.append({'function':name.split(':')[-1],'bytes':int(n)})
            hashes.append(hashlib.sha256(obj.read_bytes()).hexdigest())
        if hashes[0]!=hashes[1]:raise ValueError('Bootstrap object twins differ')
        rows.append({'optimization':'O'+opt,'twins_equal':True,'object_sha256':hashes[0],'frames':frames})
    files=['platform/mac128k/window-session.c','platform/mac128k/window-session.h',
           'apps/window-scene/foreground.h','apps/window-scene/inbox.h','apps/window-scene/capture.h','apps/window-scene/input.h','apps/window-scene/scene.h',
           'apps/product-scene/scene.h','core/input/pointer.h','core/input/mouse.h','core/ui/window.h','core/graphics/cursor.h','core/graphics/surface.h']
    report={'specification':'SPEC-0070','cpu':'68000','object_compilations':7,'descriptor_bytes':size,
            'records':rows,'imports':sorted(allowed),'mutable_data':0,'target_executed':False,'ROM_inputs_changed':False,
            'whole_stack_or_128K_certification':False,'inputs':{f:hashlib.sha256((repo/f).read_bytes().replace(b'\r\n',b'\n')).hexdigest() for f in files}}
    (a.output/'manifest.json').write_text(json.dumps(report,indent=2)+'\n')
    print(f'Mac RAM bootstrap68000 PASS: 7 objects, matched O0/O2/Os twins, descriptor{size}B; no hardware or run.')
if __name__=='__main__':main()
