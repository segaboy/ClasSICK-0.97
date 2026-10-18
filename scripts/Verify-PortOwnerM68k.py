#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Compile/link original selection owner on 68000; never execute a target."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess

MODULES = ('core/memory/guest.c', 'core/memory/manager.c',
    'core/memory/lifecycle.c', 'core/services/traps.c',
    'formats/resourcefork/reader.c', 'personality/system1/code-preflight.c',
    'personality/system1/resources.c', 'personality/system1/code-image.c',
    'personality/system1/code-run.c', 'personality/system1/code-select.c',
    'personality/system1/quickdraw-select.c', 'personality/system1/quickdraw-rects.c',
    'core/cpu/m68k.c', 'core/cpu/m68k-bus.c', 'arch/m68k/runtime.c')


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--tool-root', required=True, type=Path)
    p.add_argument('--output', required=True, type=Path)
    a = p.parse_args()
    repo = Path(__file__).resolve().parents[1]
    a.output.mkdir(parents=True, exist_ok=False)
    bins = a.tool_root / 'usr/bin'
    env = os.environ.copy()
    env['LD_LIBRARY_PATH'] = str(a.tool_root / 'usr/lib/x86_64-linux-gnu')
    def run(args):
        return subprocess.check_output([str(x) for x in args], env=env, text=True)
    cc = bins / 'm68k-linux-gnu-gcc-13'
    nm = bins / 'm68k-linux-gnu-nm'
    flags = ['-mcpu=68000', '-Wa,-m68000', '-std=c11', '-ffreestanding',
        '-fno-builtin', '-fno-pic', '-fno-pie', '-fno-common',
        '-fno-stack-protector', '-fno-tree-loop-distribute-patterns',
        '-fno-ident', '-fstack-usage', '-Wall', '-Wextra', '-Werror',
        '-Wconversion', '-Wsign-conversion', '--sysroot=' + str(a.tool_root),
        '-ffile-prefix-map=' + str(repo) + '=.']
    profiles = []
    for opt in ('0', 's'):
        twins = []
        for twin in ('a', 'b'):
            folder = a.output / ('O' + opt + '-' + twin)
            folder.mkdir()
            objects = []
            frames = []
            for i, name in enumerate(MODULES):
                obj = folder / (str(i) + '.o')
                run([cc, *flags, '-O' + opt, '-ffile-prefix-map=' + str(folder) + '=.',
                     '-c', repo / name, '-o', obj])
                symbols = run([nm, '--defined-only', '--format=posix', obj])
                if re.search(r'^\S+ [BbCcDdGgSsVv] ', symbols, re.M):
                    raise ValueError('Mutable global in ' + name)
                if 'cs_image_selection_test_fault' in symbols:
                    raise ValueError('Production exposes test fault schedule')
                objects.append(obj)
            combined = folder / 'port-owner.o'
            run([bins / 'm68k-linux-gnu-ld', '-r', *objects, '-o', combined])
            undefined = run([nm, '-u', combined]).strip()
            if undefined:
                raise ValueError('Unresolved owner closure: ' + undefined)
            symbols = run([nm, '--defined-only', '--format=posix', combined])
            for entry in ('cs_image_run_selection', 'cs_image_run_selection_retire',
                          'cs_qd_select_get', 'cs_qd_select_set'):
                if not re.search(r'^' + entry + r' T ', symbols, re.M):
                    raise ValueError('Disconnected owner entry ' + entry)
            for usage in sorted(folder.glob('*.su')):
                for line in usage.read_text().splitlines():
                    f = line.split('\t')
                    if len(f) != 3 or f[2] not in ('static', 'dynamic,bounded') or int(f[1]) > 1024:
                        raise ValueError('Unbounded/oversized frame: ' + line)
                    frames.append(dict(function=f[0].split(':')[-1], bytes=int(f[1])))
            twins.append(dict(sha256=hashlib.sha256(combined.read_bytes()).hexdigest(),
                              bytes=combined.stat().st_size, frames=frames))
        if twins[0] != twins[1]:
            raise ValueError('Owner closure twins differ')
        profiles.append(dict(optimization='O' + opt, twins_equal=True, **twins[0]))
    types = ('cs_image_service', 'cs_image_select_state', 'cs_image_select_workspace',
             'cs_image_select_request', 'cs_image_select_report')
    source = a.output / 'storage.c'
    source.write_text('#include "personality/system1/code-select.h"\n'
        'const uint32_t cs_owner_sizes[]={' + ','.join('sizeof(' + t + ')' for t in types) + '};\n')
    obj = a.output / 'storage.o'
    run([cc, *flags, '-Os', '-I' + str(repo), '-c', source, '-o', obj])
    raw = a.output / 'storage.bin'
    run([bins / 'm68k-linux-gnu-objcopy', '-O', 'binary', '--only-section=.rodata', obj, raw])
    sizes = dict(zip(types, struct.unpack('>5I', raw.read_bytes())))
    inputs = (*MODULES, 'personality/system1/code-select.h',
        'personality/system1/code-image.h', 'personality/system1/code-run-internal.h',
        'personality/system1/code-run.h', 'personality/system1/quickdraw-select.h',
        'personality/system1/quickdraw-view.h', 'personality/system1/quickdraw-rects.h',
        'core/cpu/m68k.h', 'core/cpu/m68k-bus.h', 'core/cpu/m68k-guard.h',
        'core/cpu/m68k-session.h', 'core/cpu/m68k-call.h', 'core/services/traps.h',
        'core/memory/guest.h', 'core/memory/manager.h', 'core/memory/lifecycle.h',
        'personality/system1/code-preflight.h', 'personality/system1/resources.h',
        'formats/resourcefork/reader.h', 'arch/m68k/runtime.h',
        'docs/specifications/SPEC-0045-integrated-port-selection.md',
        'scripts/Verify-PortOwnerM68k.py')
    report = dict(specification='SPEC-0045', native_cpu='68000',
        object_audits=4 * len(MODULES), complete_relocatable_links=4,
        storage_bytes_68000=sizes, profiles=profiles, target_executed=False,
        inputs={f: hashlib.sha256((repo / f).read_bytes().replace(b'\r\n', b'\n')).hexdigest()
                for f in inputs})
    (a.output / 'manifest.json').write_text(json.dumps(report, indent=2) + '\n')
    print('SPEC-0045 68000 owner closure/storage PASS; no execution claim.', sizes)


if __name__ == '__main__':
    main()
