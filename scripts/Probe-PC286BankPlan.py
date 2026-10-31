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
    parser.add_argument('--catalog', action='store_true',
                        help='Compile the unbound catalog plus bank-plan closure witness')
    parser.add_argument('--span-access', action='store_true',
                        help='Compile checked-access closure; far data only')
    parser.add_argument('--span-cap', action='store_true',
                        help='Compile capability/borrow closure; far data only')
    parser.add_argument('--span-build', action='store_true',
                        help='Compile store4 plus the span construction experiment (far data only)')
    parser.add_argument('--store4-model', action='store_true',
                        help='Compile the host-memory experiment closure; not a native provider')
    args = parser.parse_args()
    if args.catalog and args.store4_model:
        parser.error('--catalog and --store4-model are separate profiles')
    if args.span_build and (args.catalog or args.store4_model):
        parser.error('--span-build is its own profile')
    if args.span_access and (args.span_build or args.catalog or args.store4_model):
        parser.error('--span-access is its own profile')
    if args.span_cap and (args.span_access or args.span_build or args.catalog or args.store4_model):
        parser.error('--span-cap is its own profile')
    span = args.span_build
    if args.span_access or args.span_cap:
        args.catalog = True
    if span:
        args.store4_model = True
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
    source = root / 'probes/pc286/bank-plan.c'
    public_names = {'cs_bm_validate_', 'cs_bm_check_', 'cs_bm_plan4_'}
    input_names = ['arch/i286/c16.h', 'probes/pc286/bank-plan.h', 'probes/pc286/bank-plan.c']
    if args.catalog or args.store4_model:
        # Original single-translation-unit witness, not a target link/ABI proof.
        source = out / 'catalog-closure.c'
        source.write_text('#include "' + (root / 'probes/pc286/bank-plan.c').as_posix() +
                          '"\n#include "' + (root / 'probes/pc286/bank-catalog.c').as_posix() +
                          '"\n', encoding='utf-8')
        public_names.add('cs_bc_validate_')
        input_names += ['probes/pc286/bank-catalog.h', 'probes/pc286/bank-catalog.c']
    if args.store4_model:
        with source.open('a', encoding='utf-8') as f:
            f.write('#include "' + (root / 'probes/pc286/store4-model.c').as_posix() + '"\n')
        public_names.update(('cs_bs_store4_', 'cs_bs_epoch_next_'))
        input_names += ['probes/pc286/store4-model.h', 'probes/pc286/store4-model.c']
    if span:
        with source.open('a', encoding='utf-8') as f:
            f.write('#include "' + (root / 'probes/pc286/span-build.c').as_posix() + '"\n')
        public_names.update(('cs_bk_plan_', 'cs_bk_bind_'))
        input_names += ['probes/pc286/span-build.h', 'probes/pc286/span-build.c']
    if args.span_access:
        with source.open('a', encoding='utf-8') as f:
            for part in ('span-bind', 'span-access'):
                f.write('#include "' + (root / ('probes/pc286/' + part + '.c')).as_posix() + '"\n')
        public_names.update(('cs_sb_env_', 'cs_sb_walk_', 'cs_sb_find_', 'cs_bx_read_', 'cs_bx_write_'))
        input_names += ['probes/pc286/store4-model.h'] + [
            'probes/pc286/' + part + ext for part in ('span-bind', 'span-access') for ext in ('.h', '.c')]
    if args.span_cap:
        with source.open('a', encoding='utf-8') as f:
            for part in ('span-bind', 'span-cap'):
                f.write('#include "' + (root / ('probes/pc286/' + part + '.c')).as_posix() + '"\n')
        public_names.update(('cs_sb_env_', 'cs_sb_walk_', 'cs_sb_find_',
                             'cs_bq_registry_init_', 'cs_bq_owner_open_', 'cs_bq_owner_advance_',
                             'cs_bq_owner_rebind_', 'cs_bq_open_', 'cs_bq_chunk_',
                             'cs_bq_borrow_', 'cs_bq_rights_', 'cs_bq_release_'))
        input_names += ['probes/pc286/store4-model.h'] + [
            'probes/pc286/' + part + ext for part in ('span-bind', 'span-cap') for ext in ('.h', '.c')]
    rows, refusals = [], []
    if span or args.span_access or args.span_cap:
        # Near-data models cannot hold a span pointer: the sizeof guard must refuse.
        for model in ('ms', 'mm'):
            flags = ['-zq', '-2', '-' + model, '-zl', '-s', '-os', '-zastd=c99',
                     '-dCS_C16_REQUIRE_16BIT', '-i=' + str(args.tool_root / 'h'),
                     '-fo=' + str(out / (model + '-refused.obj'))]
            q = subprocess.run([str(tools / 'wcc.exe'), *flags, str(source)],
                               cwd=out, env=env, stdin=subprocess.DEVNULL,
                               capture_output=True, timeout=25)
            (out / (model + '-refused.log')).write_bytes(q.stdout + q.stderr)
            if q.returncode == 0:
                raise ValueError(model + ': near-data model unexpectedly compiled')
            guard_file = 'span-bind.c' if args.span_access or args.span_cap else 'span-build.c'
            guard_name = 'cs_sb_data_pointer_is_far' if args.span_access or args.span_cap else 'cs_bk_data_pointer_is_far'
            guard = next(i for i, line in enumerate((root / 'probes/pc286' / guard_file).read_text(
                encoding='utf-8').splitlines(), 1) if guard_name in line)
            if (guard_file + '(%d)' % guard).encode() not in q.stdout + q.stderr:
                raise ValueError(model + ': refused somewhere other than the far-data guard')
            refusals.append(dict(case=model + '-refused', flags=flags, returncode=q.returncode))
    for model in (('mc', 'ml') if span or args.span_access or args.span_cap else ('ms', 'mm', 'ml')):
        for opt in ('od', 'os'):
            name = model + '-' + opt
            obj, listing = out / (name + '.obj'), out / (name + '.lst')
            flags = ['-zq', '-2', '-' + model, '-zl', '-s', '-' + opt,
                     '-zastd=c99', '-dCS_C16_REQUIRE_16BIT',
                     '-i=' + str(args.tool_root / 'h'), '-fo=' + str(obj)]
            q = subprocess.run([str(tools / 'wcc.exe'), *flags, str(source)],
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
            if set(publics) != public_names:
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
              for p in input_names}
    report = dict(compiler_pin=lock['release'], compiler_calls=len(rows) + len(refusals),
                  objects=len(rows), inputs=inputs, records=rows, refusals=refusals,
                  linked=False, target_executed=False,
                  profile='span-cap-closure' if args.span_cap else 'span-access-closure' if args.span_access else 'span-build-closure' if span else 'store4-model-closure' if args.store4_model else 'catalog-closure' if args.catalog else 'bank-plan',
                  scope='Compiler metadata/import audit only, not target link/complete ISA/ABI/runtime proof',
                  stack_policy='-s is diagnostic only; no production stack-check waiver')
    (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print('PASS: %d 80286/USE16 unlinked objects, %d expected refusals, no unresolved imports; '
          'no target execution.' % (len(rows), len(refusals)))


if __name__ == '__main__':
    main()
