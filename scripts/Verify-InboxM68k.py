#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Compile/link original native helpers only; no ROM edit or target execution."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess

SOURCES = ('apps/window-scene/inbox.c','tests/window-inbox-layout.c')
HEADERS = ('apps/window-scene/inbox.h','core/input/pointer.h')
APIS = ('cs_inbox_begin','cs_inbox_restart','cs_inbox_baseline','cs_inbox_push',
        'cs_inbox_uncertain','cs_inbox_claim','cs_inbox_complete',
        'cs_inbox_discard','cs_inbox_resume','cs_inbox_fault','cs_inbox_stop')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--tool-root', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    repo = Path(__file__).resolve().parents[1]
    args.output.mkdir(parents=True, exist_ok=False)
    env = os.environ.copy()
    env['LD_LIBRARY_PATH'] = str(args.tool_root / 'usr/lib/x86_64-linux-gnu')
    bins = args.tool_root / 'usr/bin'
    gcc, nm = bins / 'm68k-linux-gnu-gcc-13', bins / 'm68k-linux-gnu-nm'
    def run(argv):
        return subprocess.check_output([str(v) for v in argv], env=env, text=True)
    records = []
    for opt in ('0', '2', 's'):
        twins, frames = [], []
        for twin in ('a', 'b'):
            folder = args.output / ('O' + opt + '-' + twin)
            folder.mkdir()
            objects = []
            for index, source in enumerate(SOURCES):
                obj = folder / (str(index) + '.o')
                run([gcc, '-mcpu=68000', '-O' + opt, '-std=c11', '-ffreestanding',
                     '-fno-builtin', '-fno-stack-protector', '-fno-ident',
                     '-fstack-usage', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
                     '-Wconversion', '-Wsign-conversion',
                     '-ffile-prefix-map=' + str(repo) + '=.',
                     '-c', repo / source, '-o', obj])
                objects.append(obj)
                if source == 'apps/window-scene/inbox.c':
                    own_imports=run([nm,'-u',obj]).splitlines()
                    if own_imports:
                        raise ValueError('Capture helper unexpected imports')
            closure = folder / 'native-helpers.o'
            run([gcc, '-mcpu=68000', '-nostdlib', '-no-pie', '-r',
                 '-Wl,--build-id=none', *objects, '-o', closure])
            undefined = run([nm, '-u', closure]).strip()
            defined = run([nm, '--defined-only', '--format=posix', closure])
            if undefined or re.search(r'^\S+ [BbCcDdGgSsVv] ', defined, re.M):
                for source, obj in zip(SOURCES, objects):
                    print('MODULE', 'O' + opt, source, run([nm, '-u', obj]).strip(), flush=True)
                raise ValueError('Native helper closure imports or mutable data: ' + undefined)
            if not all(re.search(r'^' + api + r' T ', defined, re.M) for api in APIS):
                raise ValueError('Missing native window API')
            layout = re.search(r'^cs_inbox_descriptor_layout R \S+ (\S+)$',
                               run([nm, '-S', '--defined-only', '--format=posix', closure]), re.M)
            if not layout or int(layout.group(1), 16) > 256:
                raise ValueError('Missing or excessive capture descriptor layout')
            descriptor_bytes = int(layout.group(1), 16)
            twins.append(dict(bytes=closure.stat().st_size,
                              sha256=hashlib.sha256(closure.read_bytes()).hexdigest()))
            if twin == 'a':
                for usage in folder.glob('*.su'):
                    for line in usage.read_text().splitlines():
                        label, size, kind = line.rsplit('\t', 2)
                        if kind not in ('static', 'dynamic,bounded'):
                            raise ValueError('Unbounded stack usage: ' + line)
                        frames.append(dict(function=label.split(':')[-1], bytes=int(size), kind=kind))
        if twins[0] != twins[1]:
            raise ValueError('Native helper closure twins differ')
        records.append(dict(optimization='O' + opt, twins_equal=True,
                            descriptor_bytes=descriptor_bytes,
                            closure=twins[0], static_frames=frames,
                            largest_frame=max(v['bytes'] for v in frames)))
    inputs = {name: hashlib.sha256((repo / name).read_bytes().replace(b'\r\n', b'\n')).hexdigest()
              for name in SOURCES + HEADERS}
    helper_frames=[v['bytes'] for row in records for v in row['static_frames']
                   if v['function'].startswith(('inbox_','cs_inbox_'))]
    if not helper_frames or max(helper_frames)>512:
        raise ValueError('Capture helper native frame bound')
    report = dict(helper_largest_frame=max(helper_frames),descriptor_bound_bytes=256,
                  descriptor_bytes=records[0]['descriptor_bytes'],specifications=['SPEC-0065'], cpu='68000',
                  inputs=inputs, object_compilations=12, closure_audits=6,
                  undefined_imports=0, mutable_data=0, records=records,
                  stack_note='Per-function compiler frames include ABI costs; not whole-system stack certification.',
                  target_executed=False, rom_inputs_changed=False)
    (args.output / 'manifest.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print('Window inbox68000 PASS:12 objects,6 closures,matching twins,no imports/data;no run.')


if __name__ == '__main__':
    main()
