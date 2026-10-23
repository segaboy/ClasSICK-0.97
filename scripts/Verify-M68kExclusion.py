#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Compile/audit arch/m68k/exclusion.c only; no target execution."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import time

SOURCE = 'arch/m68k/exclusion.c'
HEADER = 'arch/m68k/exclusion.h'
APIS = ('cs_m68k_foreground_enter', 'cs_m68k_foreground_leave')
FLAGS = ('-mcpu=68000', '-std=c11', '-ffreestanding', '-fno-builtin',
         '-fno-stack-protector', '-fno-ident', '-fstack-usage', '-Wall',
         '-Wextra', '-Wpedantic', '-Werror', '-Wconversion', '-Wsign-conversion')
FUNC = re.compile(r'^[0-9a-f]+ <([^>]+)>:$')
INSN = re.compile(r'^ *([0-9a-f]+):\t([0-9a-f]{4}(?: [0-9a-f]{4})*) *(?:\t(.*))?$')
SR = re.compile(r'(?<![\w%])%?sr\b')
SR_WORDS = re.compile(r'^(?:40c|46c|007c|027c|0a7c|4e72|4e73)')
DREG = r'%?d[0-7]'
IMM700 = r'#(?:1792|0x0*700)'
MOVE, ORI = ('movew',), ('oriw',)


def parse(text):
    """objdump -d text -> {function: [(words, mnemonic, operands)]}."""
    funcs, cur = {}, None
    for line in text.splitlines():
        head = FUNC.match(line)
        if head:
            if head.group(1) in funcs:
                raise ValueError('duplicate function ' + head.group(1))
            cur = funcs[head.group(1)] = []
            continue
        if not line.strip():
            continue
        if line.startswith('Disassembly of section '):
            cur = None
            continue
        if cur is None:
            continue
        m = INSN.match(line)
        if not m:
            raise ValueError('unexpected disassembly syntax: ' + line.strip())
        words = m.group(2).split()
        if m.group(3) is None:
            if not cur:
                raise ValueError('orphan byte continuation: ' + line.strip())
            cur[-1][0].extend(words)
            continue
        parts = m.group(3).strip().split(None, 1)
        mnem = parts[0].lower().replace('.', '')
        cur.append((words, mnem, parts[1].strip() if len(parts) > 1 else ''))
    return funcs


def sr_ops(insns):
    return [(w, mn, ops) for w, mn, ops in insns
            if SR.search(ops) or SR_WORDS.match(w[0])]


def check(funcs):
    """Raise ValueError unless enter/leave SR operations match the contract."""
    for api in APIS:
        if not funcs.get(api):
            raise ValueError('missing function ' + api)
    enter = sr_ops(funcs[APIS[0]])
    if len(enter) != 2:
        raise ValueError('enter: expected 2 SR operations, found %d' % len(enter))
    (w1, mn1, op1), (w2, mn2, op2) = enter
    if not (mn1 in MOVE and re.fullmatch(r'%?sr,' + DREG, op1)
            and w1[0][:3] == '40c'):
        raise ValueError('enter: first SR op not MOVE SR,Dn: %s %s' % (mn1, op1))
    if not (mn2 in ORI and re.fullmatch(IMM700 + r',%?sr', op2)
            and w2[:2] == ['007c', '0700']):
        raise ValueError('enter: second SR op not ORI #0x700,SR: %s %s' % (mn2, op2))
    leave = sr_ops(funcs[APIS[1]])
    if len(leave) != 1:
        raise ValueError('leave: expected 1 SR operation, found %d' % len(leave))
    w3, mn3, op3 = leave[0]
    if not (mn3 in MOVE and re.fullmatch(DREG + r',%?sr', op3)
            and w3[0][:3] == '46c'):
        raise ValueError('leave: SR op not MOVE Dn,SR: %s %s' % (mn3, op3))
    return dict(enter=[mn1 + ' ' + op1, mn2 + ' ' + op2], leave=[mn3 + ' ' + op3])


def clobbers_ok(text):
    text = re.sub(r'/\*.*?\*/|//[^\n]*', '', text, flags=re.S)
    blocks = re.findall(r'__asm__\s+volatile\s*\((.*?)\)\s*;', text, re.S)
    return len(blocks) == 2 and all('"cc"' in b and '"memory"' in b for b in blocks)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--tool-root', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    repo = Path(__file__).resolve().parents[1]
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    env = os.environ.copy()
    env['LD_LIBRARY_PATH'] = str(args.tool_root / 'usr/lib/x86_64-linux-gnu')
    bins = args.tool_root / 'usr/bin'
    gcc, nm = bins / 'm68k-linux-gnu-gcc-13', bins / 'm68k-linux-gnu-nm'
    objdump = bins / 'm68k-linux-gnu-objdump'

    def run(argv):
        return subprocess.check_output([str(v) for v in argv], env=env, text=True, timeout=60)
    if not clobbers_ok((repo / SOURCE).read_text(encoding='utf-8')):
        raise ValueError('source: need exactly 2 asm volatile blocks with cc+memory')
    records = []
    for opt in ('0', '2', 's'):
        twins, row = [], {}
        for twin in ('a', 'b'):
            if twin == 'b':
                time.sleep(1.5)
            folder = out / ('O' + opt + '-' + twin)
            folder.mkdir()
            obj = folder / 'exclusion.o'
            run([gcc, *FLAGS, '-O' + opt, '-ffile-prefix-map=' + str(repo) + '=.',
                 '-c', repo / SOURCE, '-o', obj])
            twins.append(hashlib.sha256(obj.read_bytes()).hexdigest())
            if twin != 'a':
                continue
            if run([nm, '-u', obj]).strip():
                raise ValueError('O%s: unexpected imports' % opt)
            defined = run([nm, '-S', '--defined-only', '--format=posix', obj])
            if re.search(r'^\S+ [BbCcDdGgSsVv] ', defined, re.M):
                raise ValueError('O%s: mutable data' % opt)
            sizes = {m.group(1): int(m.group(2), 16) for m in
                     re.finditer(r'^(\S+) T \S+ (\S+)$', defined, re.M)}
            if sorted(sizes) != sorted(APIS):
                raise ValueError('O%s: text symbols %s' % (opt, sorted(sizes)))
            frames = {}
            for line in obj.with_suffix('.su').read_text().splitlines():
                label, size, kind = line.rsplit('\t', 2)
                if kind != 'static' or int(size) > 64:
                    raise ValueError('O%s: frame bound: %s' % (opt, line))
                frames[label.split(':')[-1]] = int(size)
            if sorted(frames) != sorted(APIS):
                raise ValueError('O%s: stack usage functions %s' % (opt, sorted(frames)))
            disassembly=run([objdump, '-d', obj])
            row = dict(function_bytes=sizes, frame_bytes=frames,
                       sr_operations=check(parse(disassembly)), disassembly=disassembly)
        if twins[0] != twins[1]:
            raise ValueError('O%s: twin objects differ' % opt)
        records.append(dict(optimization='O' + opt, twins_equal=True,
                            object_sha256=twins[0], **row))
    inputs = {n: hashlib.sha256((repo / n).read_bytes().replace(b'\r\n', b'\n')).hexdigest()
              for n in (SOURCE, HEADER)}
    report = dict(specifications=['SPEC-0068'], cpu='68000', lto=False, inputs=inputs,
                  object_compilations=6, undefined_imports=0, mutable_data=0,
                  frame_bound_bytes=64, records=records,
                  note='Selected-object checks only; not proof of all compiler ordering, '
                       'CPU concurrency, latency or opcode completeness.',
                  target_executed=False)
    (out / 'manifest.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print('M68k exclusion PASS: 6 objects, matching twins, SR order ok; no run.')


if __name__ == '__main__':
    try:
        main()
    except (ValueError, subprocess.CalledProcessError, OSError) as exc:
        sys.exit('M68k exclusion FAIL: ' + str(exc))
