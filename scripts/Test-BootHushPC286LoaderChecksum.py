#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Exercise the optional diagnostic payload checksum in BootHush."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

from pc286_loader import floppy, MAX_PAYLOAD, SECTOR


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--emulator', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    exe = a.emulator.resolve(strict=True)
    root = a.output.resolve()
    root.mkdir(parents=True, exist_ok=False)
    emulator_sha = sha(exe.read_bytes())
    cases = []

    def run(name, data, post, inspect_offset=None, fail_read=None):
        image = root / (name + '.img')
        image.write_bytes(data)
        command = [str(exe), '--machine', 'pc286', '--floppy', str(image),
                   '--cycles', '8000000', '--timeout-ms', '20000', '--until-post', hex(post)]
        if inspect_offset is not None:
            command += ['--inspect', hex(0x10000 + inspect_offset) + ':1']
        if fail_read is not None:
            command += ['--fail-floppy-read', fail_read]
        result = subprocess.run(command, capture_output=True, text=True, timeout=25,
                                creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        if result.returncode:
            raise RuntimeError(result.stdout + result.stderr)
        response = json.loads(result.stdout)
        assert response['status'] == 'condition_met' and response['post'] == post
        assert sha(image.read_bytes()) == sha(data)
        cases.append({'name': name, 'floppy_sha256': sha(data), 'command': command,
                      'result': response})
        return response

    for length in (6, 14 * SECTOR + 1, 44 * SECTOR + 1, MAX_PAYLOAD):
        payload = bytearray((i * 37 + i // 251) & 255 for i in range(length))
        payload[:6] = bytes.fromhex('b0a5e680ebfe')
        image = floppy(bytes(payload), 0, verify_payload=True)
        run('valid-%d' % length, image, 0xa5)
        for offset in sorted({length - 1, min(length - 1, 14 * SECTOR),
                              min(length - 1, 44 * SECTOR)}):
            damaged = bytearray(image)
            damaged[SECTOR + offset] ^= 1
            response = run('damaged-%d-%d' % (length, offset), damaged, 0xed, offset)
            assert bytes.fromhex(response['memory']['bytes']) == damaged[SECTOR + offset:SECTOR + offset + 1]
        # The sector-padding bytes are not part of the payload contract.
        if length % SECTOR:
            padded = bytearray(image)
            padded[SECTOR + length] ^= 1
            run('padding-%d' % length, padded, 0xa5)

    # Equal and opposite byte changes are intentionally undetectable by a sum.
    payload = bytearray(bytes.fromhex('b0a5e680ebfe') + b'\x10\x20')
    collision = bytearray(floppy(bytes(payload), 0, verify_payload=True))
    collision[SECTOR + 6] += 1
    collision[SECTOR + 7] -= 1
    run('balanced-corruption-control', collision, 0xa5)
    payload = bytearray((i * 37 + i // 251) & 255 for i in range(MAX_PAYLOAD))
    payload[:6] = bytes.fromhex('b0a5e680ebfe')
    image = floppy(bytes(payload), 0, verify_payload=True)
    # A BIOS that reports success after an incomplete read must not lead to
    # entry when the resulting byte sum differs from the expected payload.
    short = bytearray(image)
    needle = bytes.fromhex('bb0058b80f02')
    assert short[:SECTOR].count(needle) == 1
    short[short[:SECTOR].index(needle) + 4] = 14
    response = run('short-final-read-control', short, 0xed, MAX_PAYLOAD-SECTOR)
    assert bytes.fromhex(response['memory']['bytes']) == b'\x00'
    for chs in ('0:0:2', '0:1:1', '1:0:1', '1:1:1'):
        response = run('read-failure-' + chs.replace(':', '-'), image, 0xee, fail_read=chs)
        fault = response['floppy_read_failure']
        assert fault['injected'] == 1 and fault['bios_status'] == 32
        assert [fault[x] for x in ('cylinder', 'head', 'sector')] == [int(x) for x in chs.split(':')]
    assert sha(exe.read_bytes()) == emulator_sha
    report = {'status': 'pass', 'emulator_sha256': emulator_sha, 'cases': cases,
              'source_sha256': {path.name: sha(path.read_bytes()) for path in
                                (Path(__file__), Path(__file__).with_name('pc286_loader.py'))},
              'limits': 'BootHush diagnostic only. Additive sum is not authentication and misses balanced changes; no physical or production qualification.'}
    (root / 'report.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(json.dumps({'status': 'pass', 'case_count': len(cases), 'report': str(root / 'report.json')}))


if __name__ == '__main__':
    main()
