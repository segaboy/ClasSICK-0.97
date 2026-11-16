#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Compare modeled cursor completion on two original native window ROMs.

Inputs are accepted guest vectors, not physical device counts. Each command
resets the guest and stops on the first diagnostic pointer write after
composition. No host window, live input, or physical latency is measured.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
from pathlib import Path
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
CLOCK = 7833600


def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('emulator', 'baseline', 'candidate', 'output'):
        p.add_argument('--' + name, required=True, type=Path)
    a = p.parse_args()
    exe = a.emulator.resolve(strict=True)
    out = a.output.resolve()
    if out == ROOT or ROOT in out.parents:
        raise ValueError('output must be outside the repository')
    builds = {}
    for label, folder in (('baseline', a.baseline), ('candidate', a.candidate)):
        folder = folder.resolve(strict=True)
        manifest = folder / 'manifest.json'
        m = json.loads(manifest.read_text())
        rom = folder / 'Os-a/classick-loaded-app.rom'
        if not m['twins_equal'] or sha(rom) != m['profiles'][0]['image_sha256']:
            raise ValueError(label + ' image/manifest mismatch')
        s = m['profiles'][0]['symbols']
        addresses = [s[n] for n in ('cs_mac_trace', 'cs_mac_diag',
                     'cs_mac_loss_trace', 'cs_mac_motion_drop_trace', 'cs_mac_capture_trace')]
        start, end = min(addresses), max(s['cs_mac_trace'] + 128,
            s['cs_mac_diag'] + 28, s['cs_mac_loss_trace'] + 20,
            s['cs_mac_motion_drop_trace'] + 8, s['cs_mac_capture_trace'] + 16)
        if not 0 <= start < end <= 0x2000 or end - start > 4096:
            raise ValueError('diagnostics outside expected low RAM')
        builds[label] = dict(rom=rom, symbols=s, start=start, end=end,
            rom_sha256=sha(rom), manifest_sha256=sha(manifest))
    out.mkdir(parents=True, exist_ok=False)

    def run(case):
        label, mode, phase, axis, sign = case
        b = builds[label]
        name = f'{label}-{mode}-{phase}-{axis}-{sign:+}'
        dest = out / name
        dest.mkdir()
        start = 100000000 + phase * 16320
        stop = start + 12000000
        count, amount = (64, 1) if mode == 'stream' else (1, 64)
        rows = [(start + i * 62669, sign * amount if axis == 0 else 0,
                 sign * amount if axis == 1 else 0, 0) for i in range(count)]
        trace = dest / 'input.bhmouse'
        trace.write_text(f'BHMOUSE1 4096 3 {stop} 4096\n' +
                         ''.join(' '.join(map(str, r)) + '\n' for r in rows))
        expected = [256, 171]
        expected[axis] += sign * 64
        ptr = b['symbols']['cs_mac_trace'] + 52
        cmd = [str(exe), '--machine', 'mac128k', '--rom', str(b['rom']),
               '--mouse-replay', str(trace), '--cycles', str(stop + 100),
               '--timeout-ms', '25000', '--until', f'{ptr + axis * 4}:4:{expected[axis]}',
               '--inspect', f'{b["start"]}:{b["end"] - b["start"]}']
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=30,
                           creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        (dest / 'stdout.json').write_text(r.stdout)
        (dest / 'stderr.txt').write_text(r.stderr)
        r.check_returncode()
        g = json.loads(r.stdout)
        data = bytes.fromhex(g['memory']['bytes'])
        def u(address):
            return struct.unpack_from('>I', data, address - g['memory']['address'])[0]
        losses = [u(b['symbols']['cs_mac_loss_trace'] + i * 4) for i in range(1, 5)]
        omissions = u(b['symbols']['cs_mac_motion_drop_trace'] + 4)
        fault = u(b['symbols']['cs_mac_diag'] + 24)
        total_losses = u(b['symbols']['cs_mac_trace'] + 36)
        fifo = [u(b['symbols']['cs_mac_capture_trace'] + i * 4) for i in range(1, 4)]
        actual = [u(ptr), u(ptr + 4)]
        passed = (g['status'] == 'condition_met' and g['cycles'] >= rows[-1][0]
                  and actual == expected and not any(losses) and not omissions
                  and not total_losses and not fifo[2]
                  and not fault and not g['mouse']['pending'])
        return dict(label=label, mode=mode, phase=phase, axis=axis, sign=sign,
            command=cmd, cycles=g['cycles'], pointer=actual, losses=losses,
            omissions=omissions, total_losses=total_losses, fifo=fifo, fault=fault, passed=passed,
            from_first_input_ms=(g['cycles'] - start) * 1000 / CLOCK,
            after_last_input_ms=(g['cycles'] - rows[-1][0]) * 1000 / CLOCK)

    cases = [(label, mode, phase, axis, sign) for label in builds
             for mode in ('burst', 'stream')
             for phase in (range(8) if mode == 'stream' else (0,))
             for axis in range(2) for sign in (1, -1)]
    with ThreadPoolExecutor(max_workers=4) as pool:
        results = list(pool.map(run, cases))
    comparisons = []
    for baseline, candidate in zip(results[:36], results[36:]):
        assert all(baseline[k] == candidate[k] for k in ('mode', 'phase', 'axis', 'sign'))
        comparisons.append({**{k: baseline[k] for k in ('mode', 'phase', 'axis', 'sign')},
            'baseline_ms': baseline['after_last_input_ms'],
            'candidate_ms': candidate['after_last_input_ms'],
            'improvement_ms': baseline['after_last_input_ms'] - candidate['after_last_input_ms']})
    report = dict(status='pass' if all(r['passed'] for r in results) else 'fail',
        emulator_sha256=sha(exe), script_sha256=sha(Path(__file__)),
        builds={k: {n: v[n] for n in ('rom_sha256', 'manifest_sha256')} for k, v in builds.items()},
        cases=results, comparisons=comparisons,
        limits='Diagnostic pointer after composition; modeled guest time includes input delivery, '
               'foreground work and interrupts. No physical/host-to-screen or human pass claim.')
    (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(dict(status=report['status'], cases=len(results),
        improvements_ms=[round(c['improvement_ms'], 3) for c in comparisons])))
    return 0 if report['status'] == 'pass' else 1


if __name__ == '__main__':
    raise SystemExit(main())
