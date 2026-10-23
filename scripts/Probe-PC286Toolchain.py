# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Compile-only optional C16 preparation. Never execute a target or link a runtime."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import zipfile


def digest(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--archive', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    a = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    out = a.output.resolve()
    if out == root or root in out.parents or out.exists():
        raise SystemExit('Output must be a new directory outside the checkout')
    lock = json.loads((root / 'scripts/pc286-toolchain-lock.json').read_text())
    if (a.archive.stat().st_size != lock['archive']['bytes'] or
            digest(a.archive) != lock['archive']['sha256']):
        raise SystemExit('Archive identity mismatch; nothing extracted')
    # Validate every selected member before creating any output. No installer run.
    with zipfile.ZipFile(a.archive) as archive:
        selected = []
        for item in lock['selected_members']:
            name = item['path']
            if Path(name).is_absolute() or '..' in Path(name).parts:
                raise SystemExit('Unsafe selected member')
            content = archive.read(name)
            if (len(content) != item['bytes'] or
                    hashlib.sha256(content).hexdigest() != item['sha256']):
                raise SystemExit('Tool member identity mismatch')
            selected.append((name, content))
        out.mkdir(parents=True)
        for name, content in selected:
            p = out / 'tools' / name
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_bytes(content)
    tools = out / 'tools/binnt64'
    objects = out / 'objects'
    objects.mkdir()
    env = os.environ.copy()
    for name in ['WCC', 'WATCOM', 'INCLUDE', 'LIB', 'WCL', 'WLINK']:
        env.pop(name, None)

    def run(args):
        return subprocess.run(args, cwd=objects, env=env, stdin=subprocess.DEVNULL,
                              capture_output=True, timeout=20)

    cases = []
    for model in ['ms', 'mm', 'ml']:
        for name in ['abi', 'arithmetic', 'far', 'c99', 'c11']:
            cases.append((model + '-' + name, 'probes/pc286/' + name + '.c', model,
                          'c89' if name in ['abi', 'arithmetic', 'far'] else 'c99',
                          [], name != 'c11', True))
        cases.append((model + '-controls', 'probes/pc286/controls.c', model,
                      'c89', [], True, True))
        cases.append((model + '-header', 'probes/pc286/header-abi.c', model,
                      'c99', [], True, True))
        for source in ['memory/guest', 'graphics/surface', 'services/traps', 'cpu/m68k']:
            cases.append((model + '-' + source.replace('/', '-'), 'core/' + source + '.c',
                          model, 'c99', [], True, True))
    cases += [
        ('bad-int', 'probes/pc286/controls.c', 'ms', 'c89', ['-dCS_EXPECT_INT=4'], False, True),
        ('bad-far', 'probes/pc286/controls.c', 'ms', 'c89', ['-dCS_EXPECT_FAR=2'], False, True),
        ('checked-stack', 'probes/pc286/controls.c', 'ms', 'c89', [], True, False),
        ('align', 'probes/pc286/align.c', 'mm', 'c99', [], False, True),
        ('c11-mode', 'probes/pc286/c11.c', 'mm', 'c11', [], False, True),
        ('far-o0', 'probes/pc286/far.c', 'mm', 'c89', ['-od'], True, True),
    ]
    results = []
    for name, source, model, dialect, extra, success, no_stack_helper in cases:
        obj = objects / (name + '.obj')
        flags = ['-zq', '-2', '-' + model, '-zl', '-os', '-zastd=' + dialect]
        if no_stack_helper:
            flags.append('-s')  # Diagnostic objects only; no production stack policy.
        flags += extra
        q = run([str(tools / 'wcc.exe'), *flags, '-i=' + str(out / 'tools/h'),
                 '-fo=' + str(obj), str(root / source)])
        (objects / (name + '.log')).write_bytes(q.stdout + q.stderr)
        row = {'case': name, 'source': source, 'source_sha256': digest(root / source),
               'model': model, 'flags': flags, 'exit_code': q.returncode,
               'expected_compile_success': success, 'expectation_met':
               (q.returncode == 0) == success, 'target_executed': False}
        if q.returncode == 0:
            listing = objects / (name + '.lst')
            dis = run([str(tools / 'wdis.exe'), '-l=' + str(listing), str(obj)])
            dump = run([str(tools / 'dmpobj.exe'), str(obj)])
            if dis.returncode or dump.returncode:
                raise SystemExit('Own-object tools failed: ' + name)
            text = listing.read_text(errors='replace')
            meta = dump.stdout.decode(errors='replace')
            (objects / (name + '.omf.txt')).write_text(meta, encoding='utf-8')
            externals = []
            for block in re.findall(r'EXTDEF\(8c\).*?(?=\n\S+.*recnum:|\Z)', meta, re.S):
                externals += re.findall(r"\d+ - '([^']+)' Type:", block)
            instructions = []
            forbidden = []
            in_code = False
            for line in text.splitlines():
                if line.startswith('Segment:'):
                    in_code = bool(re.match(r'Segment:\s+\S*TEXT\b', line))
                if not in_code:
                    continue
                match = re.match(r'^[0-9A-F]{4,8}\s+((?:[0-9A-F]{2}\s+)+)([a-z][a-z0-9]*)\s*(.*)$', line)
                if not match:
                    continue
                code, mnemonic, operands = match.groups()
                instructions.append(mnemonic)
                octets = bytes.fromhex(code)
                first = 0
                while first < len(octets) and octets[first] in [0x26, 0x2e, 0x36, 0x3e, 0xf0, 0xf2, 0xf3]:
                    first += 1
                if (re.search(r'\b(eax|ebx|ecx|edx|esi|edi|esp|ebp)\b', operands) or
                        mnemonic.startswith(('set', 'cmov', 'f')) or
                        mnemonic in ['movzx', 'movsx', 'bsf', 'bsr', 'bswap', 'cpuid', 'rdtsc', 'cwde', 'cdq'] or
                        (first < len(octets) and octets[first] in [0x64, 0x65, 0x66, 0x67])):
                    forbidden.append(line)
            row.update(object_bytes=obj.stat().st_size, object_sha256=digest(obj),
                       segments=re.findall(r'Segment: (\S+)\s+\S+ USE(\d+) ([0-9A-F]+) bytes', text),
                       external_symbols=sorted(set(externals)), instruction_rows=len(instructions),
                       selected_post286_guard_hits=forbidden,
                       compiler_metadata_80286=bool(re.search(r'Processor\s+: 80286', meta)))
        results.append(row)
    report = {'schema_version': 1, 'scope': 'compile-only; no link or target execution',
              'archive_sha256': lock['archive']['sha256'], 'cases': results,
              'compile_invocations': len(results), 'objects_emitted': sum(x['exit_code'] == 0 for x in results),
              'expectations_met': all(x['expectation_met'] for x in results),
              'target_executions': 0, 'runtime_tests': 0,
              'ISA_guard_scope': 'selected own-listing checks plus compiler metadata; not an exhaustive ISA validator or hardware proof',
              'runtime_helpers_linked': False, 'production_stack_policy_changed': False}
    (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    if (not report['expectations_met'] or any(x.get('selected_post286_guard_hits') for x in results)
            or not all(x.get('compiler_metadata_80286') for x in results if x['exit_code'] == 0)):
        raise SystemExit('Unexpected compiler result; inspect report and retained diagnostics')
    print('PASS: %d compile invocations, %d objects, %d expected refusals; zero target executions' %
          (len(results), report['objects_emitted'], len(results) - report['objects_emitted']))


if __name__ == '__main__':
    main()
