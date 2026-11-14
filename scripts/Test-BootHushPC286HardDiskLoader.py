#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Exercise original fixed-disk diagnostic media in bounded BootHush runs."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

from pc286_hard_disk_loader import hard_disk, MAX_PAYLOAD, SECTOR


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--emulator', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    exe = a.emulator.resolve(strict=True);root = a.output.resolve()
    root.mkdir(parents=True, exist_ok=False)
    exe_hash = sha(exe.read_bytes());cases = []

    def run(name, data, post, offsets, fail_read=None):
        path = root/(name+'.img');path.write_bytes(data)
        command = [str(exe), '--machine', 'pc286', '--hard-disk', str(path),
                   '--cycles', '8000000', '--timeout-ms', '20000', '--until-post', hex(post)]
        if fail_read is not None:
            command += ['--fail-hard-disk-read', fail_read]
        for offset in offsets:
            command += ['--inspect', hex(0x10000+offset)+':1']
        result = subprocess.run(command, capture_output=True, text=True, timeout=25,
                                creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        if result.returncode:
            raise RuntimeError(name+': '+result.stdout+result.stderr)
        response = json.loads(result.stdout)
        assert response['status'] == 'condition_met' and response['post'] == post
        if fail_read is not None:
            fault = response['hard_disk_read_failure']
            assert fault['injected'] == 1 and fault['bios_status'] == 32
            assert [fault[x] for x in ('cylinder','head','sector')] == [int(x) for x in fail_read.split(':')]
        assert sha(path.read_bytes()) == sha(data)
        rows = response.get('memory_ranges', []) if len(offsets)>1 else [response['memory']]
        assert [row['address'] for row in rows] == [0x10000+offset for offset in offsets]
        cases.append({'name':name, 'disk_sha256':sha(data), 'command':command, 'result':response})
        return [bytes.fromhex(row['bytes'])[0] for row in rows]

    for payload, entry, options in ((b'', 0, {}), (bytes(MAX_PAYLOAD+1), 0, {}),
                                    (b'\0', -1, {}), (b'\0', 1, {}),
                                    (b'\0', 0, {'bss_end': 0}),
                                    (b'\0', 0, {'bss_end': 0xf001}),
                                    (b'\0', 0, {'poison_bss': True})):
        try:
            hard_disk(payload, entry, **options)
        except ValueError:
            continue
        raise AssertionError('invalid diagnostic image accepted')
    for length in (6, 16*SECTOR, 16*SECTOR+1, 33*SECTOR+1, 50*SECTOR+1, MAX_PAYLOAD):
        payload = bytearray((i*37+i//251)&255 for i in range(length))
        payload[:6] = bytes.fromhex('b0a5e680ebfe')
        data = hard_disk(bytes(payload), 0, bss_end=length+4, poison_bss=True, verify_payload=True)
        offsets = sorted({offset for offset in (0, 16*SECTOR-1, 16*SECTOR, 33*SECTOR-1,
                                               33*SECTOR, 50*SECTOR-1, 50*SECTOR,
                                               length-1, length, length+3) if offset < length+4})
        actual = run('valid-%d'%length, data, 0xa5, offsets)
        assert actual == [payload[offset] if offset < length else 0 for offset in offsets]
    # Corrupt one loaded payload byte without touching the expected checksum.
    damaged = bytearray(data);damaged[SECTOR+MAX_PAYLOAD-1] ^= 1
    assert run('corrupt-last-byte', damaged, 0xed, [MAX_PAYLOAD-1]) == [damaged[SECTOR+MAX_PAYLOAD-1]]
    # Equal and opposite changes cancel: this check is not authentication.
    collision_payload = bytes.fromhex('b0a5e680ebfe1020')
    collision = bytearray(hard_disk(collision_payload, 0, verify_payload=True))
    collision[SECTOR+6] += 1;collision[SECTOR+7] -= 1
    assert run('balanced-corruption-control', collision, 0xa5, [6, 7]) == [0x11, 0x1f]
    # Invalid fourth-track sector creates a modeled BIOS read error after the
    # first three reads. This tests our error branch, not physical media failure.
    bad = bytearray(data)
    assert data[:SECTOR].count(bytes.fromhex('b90100ba8003')) == 1
    index = bad[:SECTOR].index(bytes.fromhex('b90100ba8003'));bad[index+1] = 0
    assert run('invalid-fourth-sector', bad, 0xee, [50*SECTOR-1, 50*SECTOR]) == [payload[50*SECTOR-1], 0]
    for chs, offset in (('0:0:2',0), ('0:1:1',16*SECTOR),
                        ('0:2:1',33*SECTOR), ('0:3:1',50*SECTOR)):
        offsets = ([offset-1] if offset else []) + [offset]
        expected = ([payload[offset-1]] if offset else []) + [0]
        assert run('injected-'+chs.replace(':','-'), data, 0xee, offsets, fail_read=chs) == expected
    assert sha(exe.read_bytes()) == exe_hash
    report = {'status':'pass', 'emulator_sha256':exe_hash, 'cases':cases,
              'source_sha256':{f.name:sha(f.read_bytes()) for f in
                               (Path(__file__), Path(__file__).with_name('pc286_hard_disk_loader.py'))},
              'limits':'Whole-disk BootHush diagnostic on its documented geometry; not a partitioned installer, production boot or physical BIOS test.'}
    (root/'report.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    print(json.dumps({'status':'pass','case_count':len(cases),'report':str(root/'report.json')}))


if __name__ == '__main__':
    main()
