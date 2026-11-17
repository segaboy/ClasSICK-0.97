# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Qualify the experimental 4096-cycle mouse profile on an exact original ROM.

Checks actual decoded coordinates, queue completion, loss counters and faults.
The retained owner trace additionally demonstrates why 2048 is unsupported.
These are modeled input checks, not physical or human response measurements.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
from pathlib import Path
import struct
import subprocess


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('emulator', 'rom', 'manifest', 'owner-trace', 'output'):
        p.add_argument('--' + name, required=True, type=Path)
    a = p.parse_args()
    manifest = json.loads(a.manifest.read_text())
    if sha(a.rom) != manifest['profiles'][0]['image_sha256']:
        raise ValueError('ROM does not match build manifest')
    symbols = manifest['profiles'][0]['symbols']
    a.output.mkdir(parents=True, exist_ok=False)
    cases = []
    for mode in ('burst', 'stream'):
        for phase in (range(8) if mode == 'stream' else (0,)):
            for axis in range(2):
                for sign in (-1, 1):
                    start = 100000000 + phase * 16320
                    amount, count = (64, 1) if mode == 'burst' else (1, 64)
                    rows = [(start + i * 62669, sign * amount if axis == 0 else 0,
                             sign * amount if axis == 1 else 0, 0) for i in range(count)]
                    expected = [256, 171]
                    expected[axis] += sign * 64
                    cases.append((f'{mode}-{phase}-{axis}-{sign}', 4096,
                                  start + 12000000, rows, expected, 0))
    for phase in (0, 4):
        for dx, dy in ((1, 1), (-1, 1), (1, -1), (-1, -1)):
            start = 100000000 + phase * 16320
            rows = [(start + i * 62669, dx, dy, 0) for i in range(64)]
            cases.append((f'diagonal-{phase}-{dx}-{dy}', 4096,
                          start + 12000000, rows, [256 + dx * 64, 171 + dy * 64], 0))
    original = a.owner_trace.read_text().splitlines()
    header = original[0].split()
    if header[0] != 'BHMOUSE1' or header[2] != '3' or header[4] != '4096':
        raise ValueError('owner trace profile changed')
    rows = [tuple(map(int, line.split())) for line in original[1:] if line.strip()]
    # These identities are the retained M1R5 observation, not a generic oracle.
    if sha(a.owner_trace) != '081d544555ab3be94b66ff308a5b5a719fe329f809dfec1bf0cd054a661dbf37':
        raise ValueError('unexpected owner trace')
    for edge, omissions in ((2048, 27), (4096, 0)):
        cases.append((f'owner-trace-{edge}', edge, int(header[3]), rows, [328, 196], omissions))

    def run(case):
        name, edge, stop, rows, expected, expected_omissions = case
        dest = a.output / name
        dest.mkdir()
        trace = dest / 'input.bhmouse'
        trace.write_text(f'BHMOUSE1 {edge} 3 {stop} 4096\n' +
                         ''.join(' '.join(map(str, row)) + '\n' for row in rows))
        cmd = [str(a.emulator), '--machine', 'mac128k', '--rom', str(a.rom),
               '--mouse-replay', str(trace), '--cycles', str(stop + 100),
               '--timeout-ms', '120000', '--inspect', '0xb00:1024',
               '--frame', str(dest / 'final.pbm')]
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=130,
                           creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        (dest / 'stdout.json').write_text(r.stdout)
        (dest / 'stderr.txt').write_text(r.stderr)
        r.check_returncode()
        g = json.loads(r.stdout)
        memory = bytes.fromhex(g['memory']['bytes'])
        def u(address):
            return struct.unpack_from('>I', memory, address - g['memory']['address'])[0]
        t = symbols['cs_mac_trace']
        state = dict(pointer=[u(t + 52), u(t + 56)], window=[u(t + 60), u(t + 64)],
                     pattern=u(t + 68), losses=u(t + 36),
                     omissions=u(symbols['cs_mac_motion_drop_trace'] + 4),
                     loss_classes=[u(symbols['cs_mac_loss_trace'] + i * 4) for i in range(1, 5)],
                     fifo=[u(symbols['cs_mac_capture_trace'] + i * 4) for i in range(1, 4)],
                     fault=u(symbols['cs_mac_diag'] + 24), pending=g['mouse']['pending'])
        passed = (g['status'] == 'cycle_limit' and state['pointer'] == expected
                  and state['omissions'] == expected_omissions and not state['losses']
                  and not any(state['loss_classes']) and not state['fifo'][0]
                  and not state['fifo'][2] and not state['fault'] and not state['pending'])
        if name.startswith('owner-trace'):
            passed = passed and state['window'] == [231, 110] and state['pattern'] == 0
        return dict(name=name, edge_cycles=edge, expected_pointer=expected,
                    expected_omissions=expected_omissions, state=state, passed=passed,
                    frame_sha256=sha(dest / 'final.pbm'), command=cmd)

    with ThreadPoolExecutor(max_workers=4) as pool:
        results = list(pool.map(run, cases))
    endpoint_equal = results[-2]['frame_sha256'] == results[-1]['frame_sha256']
    report = dict(status='pass' if all(r['passed'] for r in results) and endpoint_equal else 'fail',
                  emulator_sha256=sha(a.emulator), rom_sha256=sha(a.rom),
                  manifest_sha256=sha(a.manifest), script_sha256=sha(Path(__file__)),
                  owner_trace_sha256=sha(a.owner_trace), qualified_edge_cycles=4096,
                  dcd_invert=3, vbl_guard=4096, cases=results,
                  owner_trace_final_frames_equal=endpoint_equal,
                  limits='Bounded modeled E2 checks; 2048 reproduces known loss and is excluded. '
                         'Equal final frames alone do not prove lossless motion. '
                         'No physical timing, host pointer gain or human response qualification.')
    (a.output / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(dict(status=report['status'], cases=len(results),
                          failures=[r['name'] for r in results if not r['passed']])))
    return 0 if report['status'] == 'pass' else 1


if __name__ == '__main__':
    raise SystemExit(main())
