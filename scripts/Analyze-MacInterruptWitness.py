# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Attribute a complete modeled SCC mask span to our matching linked symbols.

Only instruction-boundary costs are available. An execute operation may include
exception entry; this is not a physical timing measurement or bus-cycle audit.
"""
import argparse
from bisect import bisect_right
from collections import Counter
import hashlib
import json
from pathlib import Path
import subprocess


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def analyze(trace, functions, irq):
    header, fields, *lines = trace.splitlines()
    h = header.split()
    if h[0] != 'BHMACMOUSE2' or h[5:8] != ['0', '1', '1']:
        raise ValueError('requires a complete, undropped detailed witness')
    names = fields.split()
    rows = [dict(zip(names, line.split())) for line in lines]
    if len(rows) != int(h[4]):
        raise ValueError('row count differs')
    masks = [r for r in rows if r['kind'] == 'cpu_mask']
    if not masks or masks[0]['reg'] != '8' or any(a['value'] != b['reg'] for a, b in zip(masks, masks[1:])):
        raise ValueError('missing baseline or discontinuous mask changes')
    spans = [(int(a['cycle']), int(b['cycle'])) for a, b in zip(masks, masks[1:])
             if int(a['address']) == irq and a['reg'] != '8' and a['value'] == '7']
    if not spans:
        raise ValueError('no complete identified SCC-handler mask span')
    start, end = max(spans, key=lambda pair: pair[1] - pair[0])
    steps = [r for r in rows if r['kind'] == 'cpu_step']
    addresses = [f[0] for f in functions]
    costs = Counter()
    observations = Counter()
    for a, b in zip(steps, steps[1:]):
        first, last = int(a['cycle']), int(b['cycle'])
        if start <= first < last <= end:
            if a['value'] != '7':
                raise ValueError('unexpected unmasked step inside selected span')
            pc = int(a['address']); index = bisect_right(addresses, pc) - 1
            if index < 0:
                raise ValueError('instruction below known text symbols')
            name = functions[index][1]
            costs[name] += last - first; observations[name] += 1
    if sum(costs.values()) != end - start:
        raise ValueError('CPU steps do not completely cover selected span')
    return {'header': header, 'scc_spans': len(spans), 'selected_start': start,
            'selected_end': end, 'cycles': end - start,
            'functions': [{'name': name, 'cycles': cost, 'execute_operations': observations[name]}
                          for name, cost in costs.most_common()]}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('trace', 'rom', 'elf', 'manifest', 'nm', 'output'):
        p.add_argument('--' + name, type=Path, required=True)
    a = p.parse_args()
    manifest = json.loads(a.manifest.read_text(encoding='utf-8'))
    profiles = [x for x in manifest['profiles'] if x['image_sha256'] == sha(a.rom)]
    if not profiles:
        p.error('ROM is not identified by the supplied build manifest')
    profile = profiles[0]
    cmd = [str(a.nm), '-n', '-S', '--defined-only', str(a.elf)]
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=20,
                            creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
    if result.returncode:
        raise RuntimeError(result.stderr)
    functions = []
    for line in result.stdout.splitlines():
        fields = line.split()
        if len(fields) in (3, 4) and fields[-2] in ('t', 'T'):
            address, name = int(fields[0], 16), fields[-1]
            functions.append((address, name))
    functions.sort()
    # The older manifest stores one address per name; file-local names may
    # repeat across objects. Require its retained address to exist in that set.
    for name in {name for _, name in functions}:
        if profile['symbols'].get(name) not in {address for address, n in functions if n == name}:
            raise ValueError('ELF text symbol differs from manifest: ' + name)
    report = analyze(a.trace.read_text(encoding='ascii'), functions,
                     profile['symbols']['cs_mac_window_scc_irq'])
    report.update(input_sha256={name: sha(getattr(a, name)) for name in ('trace', 'rom', 'elf', 'manifest', 'nm')},
                  script_sha256=sha(Path(__file__)), symbol_command=cmd,
                  scope='BootHush execute-boundary attribution to original linked text symbols; '
                        'calls charged to caller, returns to callee; no physical or sole-cause claim.')
    with a.output.open('x', encoding='utf-8') as stream:
        json.dump(report, stream, indent=2); stream.write('\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
