# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Install the 286 reader on a blank image, then cold boot with separate media."""
import argparse
import hashlib
from importlib import import_module
import json
from pathlib import Path
import struct
import subprocess

from pc286_hard_disk_loader import DISK_BYTES
from startup_files_fixture import original_media

ROOT = Path(__file__).resolve().parents[1]
EMULATOR_SHA = '5fa45b7542409300b7604606a78c81d4ee95e888a57c592a83d0cec43ce1056e'
SYSTEM_DISK_SHA = '1e03b75a05bbf441ae4e1808f70e38ffe0a07e3cc3298de151c0d2775552c647'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def qualify(build, exe, out, selected):
    if ROOT in out.parents or ROOT == out:
        raise ValueError('place all generated media outside the repository')
    if sha(exe) != EMULATOR_SHA:
        raise ValueError('requires the qualified BootHush build with hard disk selection')
    if selected and (ROOT in selected.parents or sha(selected) != SYSTEM_DISK_SHA):
        raise ValueError('private route requires the exact selected System Disk')
    manifest = json.loads((build/'media.json').read_text())
    if manifest['profile'] != 'installed-startup':
        raise ValueError('requires a startup reader build')
    for paths, directory in [(manifest['source_sha256'], ROOT), (manifest['images'], build/'media')]:
        for name, digest in paths.items():
            if sha(directory/name) != digest:
                raise ValueError('build identity mismatch: ' + name)
    binary = build/'startup.bin'
    if sha(binary) != manifest['payload_sha256']:
        raise ValueError('native payload mismatch')
    out.mkdir(parents=True, exist_ok=False)
    observations = []

    def run(name, args, post):
        command = [exe, '--machine', 'pc286', *args, '--until-post', hex(post),
                   '--cycles', '600000000', '--timeout-ms', '60000']
        result = subprocess.run(command, cwd=out, capture_output=True, timeout=70)
        if result.returncode != 0:
            raise ValueError(name + ': BootHush failed: ' + result.stderr.decode(errors='replace')[:400])
        data = json.loads(result.stdout)
        if data['status'] != 'condition_met' or data['post'] != post:
            raise ValueError(name + ': unexpected guest stop ' + str((data['status'], data.get('post'))))
        observations.append({'name': name, 'post': post, 'cycles': data['cycles']})
        return data

    blank, installed = out/'blank.img', out/'installed.img'
    blank.write_bytes(bytes(DISK_BYTES))
    blank_sha = sha(blank)
    install_floppy = build/'media/install-floppy.img'
    keys = []
    for i, key in enumerate((0x43, 0x31, 0x1b, 0x2c, 0x1c, 0x4b, 0x4b, 0x5a)):
        time = 10000000 + i*100000
        for delta, value in ((0,key), (10000,0xf0), (20000,key)):
            keys += ['--scancode', f'{time+delta}:{value}']
    run('install', ['--floppy', install_floppy, '--hard-disk', blank, '--disk-out', installed, *keys], 0xac)
    installed_sha = sha(installed)
    if installed_sha != manifest['images']['hard-disk.img'] or sha(blank) != blank_sha:
        raise ValueError('installed disk or original blank differs')

    original, oracle = original_media()
    words = oracle['expected']
    cases = [('generated', original, [words[1], words[2], words[4], words[5], *words[8:12]])]
    # Both floppy and hard disk have a PC boot signature in this control.
    # The Macintosh reader leaves this unused part of the fixture opaque.
    dual_signed = bytearray(original)
    dual_signed[510:512] = b'\x55\xaa'
    cases.append(('dual-signature', bytes(dual_signed), cases[0][2]))
    bad = bytearray(original)
    bad[0:2] = bytes(2)
    cases.append(('invalid-header', bytes(bad), [2, 1, 1, 3, 0, 0, 0, 0]))
    if selected:
        logical = import_module('Test-MfsVolumeMedia').dc42_data(selected)
        # Independently recorded floppy-boot observations, SPEC-0122.
        cases.append(('selected', logical, [0, 6, 0, 0, 2, 39, 144, 1]))
    payload = binary.read_bytes()
    state_at = manifest['state_address']
    spans = [(0x10000+i, payload[i:i+4096]) for i in range(0, len(payload), 4096)]
    inspect = []
    for address, content in spans:
        inspect += ['--inspect', f'{address}:{len(content)}']
    inspect += ['--inspect', f'{state_at}:32']
    for name, logical, expected in cases:
        if len(logical) != 800*512:
            raise ValueError('unexpected logical disk size')
        carrier = out/(name+'-transport.img')
        carrier.write_bytes(logical + bytes(1600*512))
        before = sha(carrier)
        previous = None
        for attempt in (1, 2):
            result = run(f'{name}-{attempt}', ['--boot-drive', 'hard-disk', '--hard-disk', installed,
                         '--floppy', carrier, *inspect], 0xd1 if expected[0] else 0xb8)
            ranges = result['memory_ranges']
            if result['cs'] != 0x1000 or len(ranges) != len(spans)+1 or ranges[-1]['address'] != state_at:
                raise ValueError(name + ': wrong execution or memory address')
            for (address, content), actual in zip(spans, ranges):
                if actual['address'] != address or bytes.fromhex(actual['bytes']) != content:
                    raise ValueError(name + ': installed payload differs in RAM')
            state = list(struct.unpack('<8I', bytes.fromhex(ranges[-1]['bytes'])))
            if state != expected or (previous is not None and previous != result['cycles']):
                raise ValueError(name + ': guest status or cold repeat differs ' + str(state))
            previous = result['cycles']
            observations[-1]['startup_state'] = state
        if sha(carrier) != before:
            raise ValueError('transport changed')
    if sha(installed) != installed_sha or (selected and sha(selected) != SYSTEM_DISK_SHA):
        raise ValueError('input changed during qualification')
    report = {'status': 'pass', 'installed_sha256': installed_sha,
              'payload_sha256': sha(binary), 'emulator_sha256': sha(exe),
              'manifest_sha256': sha(build/'media.json'), 'verifier_sha256': sha(Path(__file__)),
              'private_container_sha256': SYSTEM_DISK_SHA if selected else None,
              'observations': observations}
    (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps({'status': 'pass', 'cases': len(observations), 'report': str(out/'report.json')}))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('build', 'emulator', 'output'):
        parser.add_argument('--'+name, type=Path, required=True)
    parser.add_argument('--disk', type=Path, help='optional private selected DC42 outside the repository')
    args = parser.parse_args()
    qualify(args.build.resolve(strict=True), args.emulator.resolve(strict=True),
            args.output.resolve(), args.disk.resolve(strict=True) if args.disk else None)
