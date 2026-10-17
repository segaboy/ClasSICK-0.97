#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Compile original SPEC-0043 native selection; no target execution or ROM change."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--tool-root', required=True, type=Path)
    p.add_argument('--output', required=True, type=Path)
    a = p.parse_args()
    repo = Path(__file__).resolve().parents[1]
    a.output.mkdir(parents=True, exist_ok=False)
    env = os.environ.copy()
    env['LD_LIBRARY_PATH'] = str(a.tool_root / 'usr/lib/x86_64-linux-gnu')
    bins = a.tool_root / 'usr/bin'
    source = repo / 'personality/system1/quickdraw-select.c'
    records = []
    for opt in ('0', '2', 's'):
        twins = []
        for twin in ('a', 'b'):
            dest = a.output / ('selection-O' + opt + '-' + twin + '.o')
            subprocess.check_call([str(bins / 'm68k-linux-gnu-gcc-13'), '-mcpu=68000',
                '-O' + opt, '-std=c11', '-ffreestanding', '-fno-builtin',
                '-fno-stack-protector', '-fno-ident', '-Wall', '-Wextra',
                '-Wpedantic', '-Werror', '-Wconversion', '-Wsign-conversion',
                '-ffile-prefix-map=' + str(repo) + '=.', '-c', str(source),
                '-o', str(dest)], env=env)
            nm = str(bins / 'm68k-linux-gnu-nm')
            undef = subprocess.check_output([nm, '-u', str(dest)], env=env, text=True)
            symbols = subprocess.check_output([nm, '--defined-only', '--format=posix',
                                               str(dest)], env=env, text=True)
            data = re.findall(r'^\S+ [BbCcDdGgSsVv] .*$', symbols, re.M)
            if undef.strip() or data:
                raise ValueError('Selection imports/data: ' + repr(undef.strip())
                                 + ' / ' + repr(data))
            for name in ('cs_qd_select_get', 'cs_qd_select_set'):
                if not re.search(r'^' + name + r' T ', symbols, re.M):
                    raise ValueError('Missing selection entry: ' + name)
            twins.append(dict(bytes=dest.stat().st_size,
                              sha256=hashlib.sha256(dest.read_bytes()).hexdigest()))
        if twins[0] != twins[1]:
            raise ValueError('Selection object twins differ')
        records.append(dict(optimization='O' + opt, twins_equal=True, object=twins[0]))
    names = ('quickdraw-select.c', 'quickdraw-select.h', 'quickdraw-view.h',
             'quickdraw-rects.h')
    inputs = {'personality/system1/' + name: hashlib.sha256(
        (source.parent / name).read_bytes().replace(b'\r\n', b'\n')).hexdigest()
        for name in names}
    report = dict(specification='SPEC-0043', cpu='68000', inputs=inputs,
                  object_audits=6, undefined_imports=0, mutable_data=0,
                  records=records, target_executed=False, rom_inputs_changed=False)
    (a.output / 'manifest.json').write_text(json.dumps(report, indent=2) + '\n')
    print('SPEC-0043 68000 PASS: six objects, matching twins, no imports/data; no run.')


if __name__ == '__main__':
    main()
