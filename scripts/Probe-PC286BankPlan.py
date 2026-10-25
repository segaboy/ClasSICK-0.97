#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Compile only the original C99 bank-plan prototype; never link or execute it."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--tool-root', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    lock = json.loads((root / 'scripts/pc286-toolchain-lock.json').read_text(encoding='utf-8'))
    for row in lock['selected_members']:
        data = (args.tool_root / row['path']).read_bytes()
        if len(data) != row['bytes'] or hashlib.sha256(data).hexdigest() != row['sha256']:
            raise ValueError('Pinned tool/header identity mismatch: ' + row['path'])
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    env = os.environ.copy()
    for key in ('WCC', 'WATCOM', 'INCLUDE', 'LIB', 'WCL', 'WLINK'):
        env.pop(key, None)
    tools = args.tool_root / 'binnt64'
    rows = []
    for model in ('ms', 'mm', 'ml'):
        for opt in ('od', 'os'):
            name = model + '-' + opt
            obj, listing = out / (name + '.obj'), out / (name + '.lst')
            flags = ['-zq', '-2', '-' + model, '-zl', '-s', '-' + opt,
                     '-zastd=c99', '-dCS_C16_REQUIRE_16BIT',
                     '-i=' + str(args.tool_root / 'h'), '-fo=' + str(obj)]
            q = subprocess.run([str(tools / 'wcc.exe'), *flags,
                                str(root / 'probes/pc286/bank-plan.c')],
                               cwd=out, env=env, stdin=subprocess.DEVNULL,
                               capture_output=True, timeout=25)
            (out / (name + '.log')).write_bytes(q.stdout + q.stderr)
            if q.returncode != 0:
                raise ValueError(name + ': compile failed; see private log')
            subprocess.run([str(tools / 'wdis.exe'), '-l=' + str(listing), str(obj)],
                           cwd=out, env=env, stdin=subprocess.DEVNULL,
                           capture_output=True, timeout=20, check=True)
            meta = subprocess.check_output([str(tools / 'dmpobj.exe'), str(obj)],
                                           cwd=out, env=env, stdin=subprocess.DEVNULL,
                                           timeout=20).decode(errors='replace')
            externals, publics = [], []
            for block in re.findall(r'EXTDEF\(8c\).*?(?=\n\S+.*recnum:|\Z)', meta, re.S):
                externals += re.findall(r"\d+ - '([^']+)' Type:", block)
            for block in re.findall(r'PUBDEF\(90\).*?(?=\n\S+.*recnum:|\Z)', meta, re.S):
                publics += re.findall(r"[0-9a-f]+h - '([^']+)' Type:", block)
            if set(publics) != {'cs_bm_validate_', 'cs_bm_check_', 'cs_bm_plan4_'}:
                raise ValueError(name + ': unexpected public definition set')
            unresolved = sorted(set(externals) - set(publics))
            if unresolved:
                raise ValueError(name + ': unadmitted helper/import: ' + ','.join(unresolved))
            if not re.search(r'Processor\s+: 80286', meta):
                raise ValueError(name + ': CPU metadata')
            asm = listing.read_text(encoding='utf-8', errors='replace')
            segments = re.findall(r'Segment: (\S+)\s+\S+ USE(\d+) ([0-9A-F]+) bytes', asm)
            if not segments or any(bits != '16' for _, bits, _ in segments):
                raise ValueError(name + ': missing/incorrect segment width')
            rows.append(dict(case=name, flags=flags, external_symbols=externals,
                             defined_symbols=publics, unresolved_symbols=unresolved,
                             segments=segments, object_bytes=obj.stat().st_size,
                             object_sha256=hashlib.sha256(obj.read_bytes()).hexdigest()))
    inputs = {p: hashlib.sha256((root / p).read_bytes().replace(b'\r\n', b'\n')).hexdigest()
              for p in ('arch/i286/c16.h', 'probes/pc286/bank-plan.h', 'probes/pc286/bank-plan.c')}
    report = dict(compiler_pin=lock['release'], compiler_calls=6, objects=6,
                  inputs=inputs, records=rows, linked=False, target_executed=False,
                  scope='Compiler metadata/import audit only, not complete ISA/ABI/runtime proof',
                  stack_policy='-s is diagnostic only; no production stack-check waiver')
    (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print('PASS: six 80286/USE16 unlinked objects, no unresolved imports; no target execution.')


if __name__ == '__main__':
    main()
