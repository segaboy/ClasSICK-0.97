#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Exercise original fixed-disk diagnostic media in bounded BootHush runs."""
import argparse
import binascii
import hashlib
import json
from pathlib import Path
import subprocess
from unittest.mock import patch

import pc286_hard_disk_loader as disk_loader
from pc286_hard_disk_loader import hard_disk, crc16, MAX_PAYLOAD, SECTOR


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

    def run(name, data, post, offsets, fail_read=None, fail_read_once=None, injected=1):
        path = root/(name+'.img');path.write_bytes(data)
        command = [str(exe), '--machine', 'pc286', '--hard-disk', str(path),
                   '--cycles', '40000000', '--timeout-ms', '20000', '--until-post', hex(post)]
        if fail_read is not None:
            command += ['--fail-hard-disk-read', fail_read]
        if fail_read_once is not None:
            command += ['--fail-hard-disk-read-once', fail_read_once]
        for offset in offsets:
            command += ['--inspect', hex(0x10000+offset)+':1']
        result = subprocess.run(command, capture_output=True, text=True, timeout=25,
                                creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        if result.returncode:
            raise RuntimeError(name+': '+result.stdout+result.stderr)
        response = json.loads(result.stdout)
        assert response['status'] == 'condition_met' and response['post'] == post
        if fail_read is not None or fail_read_once is not None:
            fault = response['hard_disk_read_failure']
            selected=fail_read if fail_read is not None else fail_read_once
            assert fault['injected'] == injected and fault['bios_status'] == 32
            assert [fault[x] for x in ('cylinder','head','sector')] == [int(x) for x in selected.split(':')]
            if fail_read_once is not None:assert fault['limit']==1
        assert sha(path.read_bytes()) == sha(data)
        rows = response.get('memory_ranges', []) if len(offsets)>1 else [response['memory']]
        assert [row['address'] for row in rows] == [0x10000+offset for offset in offsets]
        cases.append({'name':name, 'disk_sha256':sha(data), 'command':command, 'result':response})
        return [bytes.fromhex(row['bytes'])[0] for row in rows]

    for payload, entry, options in ((b'', 0, {}), (bytes(MAX_PAYLOAD+1), 0, {}),
                                    (b'\0', -1, {}), (b'\0', 1, {}),
                                    (b'\0', 0, {'bss_end': 0}),
                                    (b'\0', 0, {'bss_end': 0xf001}),
                                    (b'\0', 0, {'poison_bss': True}),
                                    (b'\0', 0, {'verify_payload': True, 'verify_crc16': True})):
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
    # Independent host-library oracle and a control that the additive sum
    # misses. CRC is error detection, not authentication or a safety proof.
    assert crc16(b'123456789') == binascii.crc_hqx(b'123456789', 0xffff) == 0x29b1
    for length in (6, 16*SECTOR+1, 50*SECTOR+1, MAX_PAYLOAD):
        sample = bytearray((i*37+i//251)&255 for i in range(length))
        sample[:6] = bytes.fromhex('b0a5e680ebfe')
        assert crc16(sample) == binascii.crc_hqx(sample, 0xffff)
        crc_image = hard_disk(bytes(sample), 0, verify_crc16=True)
        assert run('crc-valid-%d'%length, crc_image, 0xa5, [0, length-1]) == [sample[0], sample[-1]]
        corrupted = bytearray(crc_image);corrupted[SECTOR+length-1] ^= 1
        assert run('crc-corrupt-%d'%length, corrupted, 0xed, [length-1]) == [corrupted[SECTOR+length-1]]
        if length % SECTOR:
            padding = bytearray(crc_image);padding[SECTOR+length] ^= 1
            assert run('crc-padding-%d'%length, padding, 0xa5, [length-1]) == [sample[-1]]
    crc_collision = bytearray(hard_disk(collision_payload, 0, verify_crc16=True))
    crc_collision[SECTOR+6] += 1;crc_collision[SECTOR+7] -= 1
    assert run('crc-balanced-corruption', crc_collision, 0xed, [6, 7]) == [0x11, 0x1f]
    for chs, offset in (('0:0:2',0), ('0:1:1',16*SECTOR),
                        ('0:2:1',33*SECTOR), ('0:3:1',50*SECTOR)):
        offsets = ([offset-1] if offset else []) + [offset]
        expected = ([sample[offset-1]] if offset else []) + [0]
        assert run('crc-read-failure-'+chs.replace(':','-'), crc_image, 0xee, offsets, fail_read=chs) == expected
    # Geometry preflight must stop before loading if the model reports a
    # different drive geometry or does not recognize the queried drive.
    guarded = hard_disk(bytes(sample), 0, verify_crc16=True, verify_geometry=True)
    assert run('geometry-valid-max', guarded, 0xa5, [0, len(sample)-1]) == [sample[0], sample[-1]]
    for name,needle,replacement in (
            ('geometry-wrong-cylinders', bytes.fromhex('81f99166'), bytes.fromhex('81f99165')),
            ('geometry-wrong-heads', bytes.fromhex('81fa0103'), bytes.fromhex('81fa0102')),
            ('geometry-invalid-drive', bytes.fromhex('b80008ba8000cd13'), bytes.fromhex('b80008ba8100cd13'))):
        assert guarded[:SECTOR].count(needle)==1
        altered = bytearray(guarded);at=guarded.index(needle);altered[at:at+len(needle)]=replacement
        assert run(name, altered, 0xef, [0]) == [0]
    retried=hard_disk(bytes(sample),0,verify_crc16=True,verify_geometry=True,retry_reads=True)
    for chs,offset in (('0:0:2',0),('0:1:1',16*SECTOR),('0:2:1',33*SECTOR),('0:3:1',50*SECTOR)):
        assert run('retry-once-'+chs.replace(':','-'),retried,0xa5,[offset],fail_read_once=chs)==[sample[offset]]
        offsets=([offset-1] if offset else [])+[offset]
        expected=([sample[offset-1]] if offset else [])+[0]
        assert run('retry-persistent-'+chs.replace(':','-'),retried,0xee,offsets,fail_read=chs,injected=3)==expected
    # Force the first reset to request an unsupported operation after an
    # injected read failure; the guest must stop instead of continuing.
    reset=bytes.fromhex('b80000ba800057cd135f')
    assert retried[:SECTOR].count(reset)==4
    failed_reset=bytearray(retried);at=retried.index(reset);failed_reset[at+2]=0xff
    assert run('retry-reset-failure',failed_reset,0xee,[0],fail_read_once='0:0:2')==[0]
    # Retry is independently selectable with no check or the older sum.
    for name,options in (('retry-no-check',{}),('retry-additive',{'verify_payload':True})):
        basic=hard_disk(collision_payload,0,retry_reads=True,**options)
        assert run(name,basic,0xa5,[0],fail_read_once='0:0:2')==[collision_payload[0]]
    # Test-only firmware-call wrapper overwrites DI after INT 13h using MOV,
    # which preserves carry, then restores DI from the stack. This models a
    # clobber at the call boundary without changing BootHush's firmware.
    with patch.object(disk_loader,'_retry_bios_call',return_value=bytes.fromhex('57 cd13 bf0000 5f')):
        clobbered=hard_disk(collision_payload,0,retry_reads=True)
    assert run('retry-di-clobber-once',clobbered,0xa5,[0],fail_read_once='0:0:2')==[collision_payload[0]]
    assert run('retry-di-clobber-persistent',clobbered,0xee,[0],fail_read='0:0:2',injected=3)==[0]
    # Remove preservation in the same test-only wrapper: persistent failure
    # must escape the three-attempt bound, proving the control is sensitive.
    with patch.object(disk_loader,'_retry_bios_call',return_value=bytes.fromhex('cd13 bf0000')):
        unprotected=hard_disk(collision_payload,0,retry_reads=True)
    unprotected_path=root/'retry-di-unprotected.img';unprotected_path.write_bytes(unprotected)
    command=[str(exe),'--machine','pc286','--hard-disk',str(unprotected_path),'--cycles','200000',
             '--timeout-ms','20000','--fail-hard-disk-read','0:0:2','--inspect','0x10000:1']
    result=subprocess.run(command,capture_output=True,text=True,timeout=25,
                          creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
    if result.returncode:raise RuntimeError(result.stdout+result.stderr)
    response=json.loads(result.stdout)
    assert response['status']=='cycle_limit' and response['hard_disk_read_failure']['injected']>3
    assert response['memory']['bytes']=='00' and sha(unprotected_path.read_bytes())==sha(unprotected)
    cases.append({'name':'retry-di-unprotected-control','disk_sha256':sha(unprotected),
                  'command':command,'result':response})
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
