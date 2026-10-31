#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Measure one admitted proposal's type sizes with the pinned C16 compiler.

Compilation only: no link, code execution, emulator, or physical measurement.
The proposed header remains outside Git. Use a new output directory outside the
repository; this script refuses an existing output and a mismatched input.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess

HEADER_SHA256 = 'eafa2e79e0aa75e15ab475712af6d26bab7577a6d02e2e00063ed74228a14ff7'
TYPES = {'store': 'cs_bg_store', 'capability': 'cs_bq_cap', 'owner': 'cs_bq_owner',
         'environment': 'cs_bx_env', 'catalog_entry': 'cs_bc_bank',
         'span': 'cs_bs_span', 'chunk': 'cs_bk_chunk'}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--tool-root', required=True, type=Path)
    parser.add_argument('--proposed-header', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    tool = args.tool_root.resolve()
    out = args.output.resolve()
    if out == root or root in out.parents:
        parser.error('output must be outside the repository')
    if out.exists():
        parser.error('output already exists')
    if digest(args.proposed_header) != HEADER_SHA256:
        parser.error('proposed header identity mismatch')
    lock = json.loads((root / 'scripts/pc286-toolchain-lock.json').read_text(encoding='utf-8'))
    for member in lock['selected_members']:
        path = tool / member['path']
        if path.stat().st_size != member['bytes'] or digest(path) != member['sha256']:
            raise ValueError('pinned tool/header identity mismatch: ' + member['path'])
    (out / 'probes/pc286').mkdir(parents=True)
    (out / 'arch/i286').mkdir(parents=True)
    for header in ('span-access.h', 'span-bind.h', 'span-build.h', 'span-cap.h',
                   'store4-model.h', 'bank-plan.h', 'bank-catalog.h'):
        shutil.copy2(root / 'probes/pc286' / header, out / 'probes/pc286' / header)
    shutil.copy2(root / 'arch/i286/c16.h', out / 'arch/i286/c16.h')
    shutil.copy2(args.proposed_header, out / 'probes/pc286/span-store.h')
    env = os.environ.copy()
    for key in ('WCC', 'WATCOM', 'INCLUDE', 'LIB', 'WCL', 'WLINK'):
        env.pop(key, None)
    source, obj = out / 'measure.c', out / 'measure.obj'
    compiler = tool / 'binnt64/wcc.exe'
    flags = ['-zq', '-2', '-mc', '-zl', '-s', '-os', '-zastd=c99',
             '-dCS_C16_REQUIRE_16BIT', '-i=' + str(tool / 'h'), '-fo=' + str(obj)]
    header = (out / 'probes/pc286/span-store.h').as_posix()
    calls = 0

    def fits(typ, value):
        nonlocal calls
        source.write_text('#include "' + header + '"\n' +
                          'typedef char measure[(sizeof(' + typ + ') <= ' +
                          str(value) + 'UL) ? 1 : -1];\n', encoding='utf-8')
        proc = subprocess.run([str(compiler), *flags, str(source)], cwd=out, env=env,
                              stdin=subprocess.DEVNULL, capture_output=True, timeout=25)
        calls += 1
        if proc.returncode and b'measure.c(2)' not in proc.stdout + proc.stderr:
            raise RuntimeError('unexpected compiler failure for ' + typ)
        return proc.returncode == 0

    result = {}
    for name, typ in TYPES.items():
        if not fits(typ, 65535):
            raise ValueError('type is not representable in the selected size range: ' + typ)
        lo, hi = 0, 65535
        while lo < hi:
            mid = (lo + hi) // 2
            if fits(typ, mid):
                hi = mid
            else:
                lo = mid + 1
        if not fits(typ, lo) or (lo and fits(typ, lo - 1)):
            raise ValueError('size boundary did not reproduce: ' + typ)
        result[name] = lo
    inputs = {str(p.relative_to(out)).replace('\\', '/'): digest(p)
              for p in sorted((out / 'probes/pc286').glob('*.h'))}
    inputs['arch/i286/c16.h'] = digest(out / 'arch/i286/c16.h')
    report = {'compiler_pin': lock['release'], 'model': 'mc', 'optimization': 'os',
              'sizes_bytes': result, 'compiler_calls': calls, 'inputs': inputs,
              'linked': False, 'target_executed': False,
              'scope': 'Compile-time type sizes only; no stack, target or performance proof.'}
    (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
