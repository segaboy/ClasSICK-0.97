#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Link only the original native workload/service/helpers; no interpreter or boot."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--tool-root',required=True,type=Path)
    p.add_argument('--rom-build',required=True,type=Path)
    p.add_argument('--output',required=True,type=Path)
    a=p.parse_args();repo=Path(__file__).resolve().parents[1]
    manifest=json.loads((a.rom_build/'manifest.json').read_text())
    if (manifest['specification'] not in ('SPEC-0038','SPEC-0047','SPEC-0049') or manifest['execution_path']!='direct-native-c'
        or manifest.get('native_drawing_context')!='SPEC-0041'):
        raise ValueError('Require the paired native ROM build')
    modules=list(manifest['inputs'])[:manifest['module_count']]
    selected=('apps/native-drawing/workload.c','personality/system1/quickdraw-view.c','personality/system1/quickdraw-rects.c','arch/m68k/runtime.c')
    if manifest['specification']=='SPEC-0047':
        selected=('apps/selected-drawing/workload.c','personality/system1/quickdraw-select.c',*selected[1:])
    if manifest['specification']=='SPEC-0049':
        selected=('apps/outline-drawing/workload.c','personality/system1/quickdraw-select.c','personality/system1/quickdraw-outline.c',*selected[1:])
    for name in selected:
        if hashlib.sha256((repo/name).read_bytes().replace(b'\r\n',b'\n')).hexdigest()!=manifest['inputs'][name]:
            raise ValueError('Native input changed: '+name)
    env=os.environ.copy();env['LD_LIBRARY_PATH']=str(a.tool_root/'usr/lib/x86_64-linux-gnu')
    bins=a.tool_root/'usr/bin';a.output.mkdir(parents=True,exist_ok=False)
    def run(argv):return subprocess.check_output([str(x) for x in argv],env=env,text=True)
    records=[]
    for twin in ('a','b'):
        objects=[a.rom_build/('Os-'+twin)/(str(modules.index(name))+'.o') for name in selected]
        dest=a.output/('native-'+twin+'.o')
        run([bins/'m68k-linux-gnu-gcc-13','-mcpu=68000','-nostdlib','-no-pie','-r',
             '-Wl,--build-id=none',*objects,'-o',dest])
        undefined=run([bins/'m68k-linux-gnu-nm','-u',dest]).strip()
        names=run([bins/'m68k-linux-gnu-nm','--defined-only','--format=posix',dest])
        if undefined or any(token in names for token in ('cs_image_','cs_m68k_','cs_drawing_app_','cs_trap_')):
            raise ValueError('Native closure contains imports or a compatibility dependency')
        required=('cs_qd_rect','cs_qd_view_init','cs_qd_view_offset','cs_qd_view_clip','cs_qd_view_rect')
        required+=('cs_selected_draw','cs_qd_select_get','cs_qd_select_set') if manifest['specification']=='SPEC-0047' else ('cs_native_draw',)
        if manifest['specification']=='SPEC-0049':required=tuple(x for x in required if x!='cs_native_draw')+('cs_outline_draw','cs_qd_view_outline','cs_qd_select_get','cs_qd_select_set')
        if not all(token in names for token in required):
            raise ValueError('Native workload/service absent')
        records.append(dict(bytes=dest.stat().st_size,sha256=hashlib.sha256(dest.read_bytes()).hexdigest()))
    if records[0]!=records[1]:raise ValueError('Native closure twins differ')
    report=dict(specification=manifest['specification'],native_drawing_context='SPEC-0041',selected_original_modules=list(selected),
                undefined_imports=0,interpreter_symbols=0,twins_equal=True,object=records[0],emulator_run=False)
    (a.output/'manifest.json').write_text(json.dumps(report,indent=2)+'\n')
    print('Native68000 workload/service closure PASS: no undefined imports or interpreter symbols; matching twins.')
if __name__=='__main__':main()
