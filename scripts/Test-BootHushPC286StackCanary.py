#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Measure an opt-in diagnostic stack watermark during a recorded 286 session."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

from pc286_hard_disk_loader import hard_disk


def digest(data):
    return hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--session', required=True, type=Path)
    parser.add_argument('--emulator', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    session = args.session.resolve(strict=True)
    exe = args.emulator.resolve(strict=True)
    out = args.output.resolve()
    repo = Path(__file__).resolve().parents[1]
    if out == repo or repo in out.parents:
        raise ValueError('output must be outside repository')
    previous = json.loads((session/'report.json').read_text(encoding='utf-8'))
    payload = (session/'session.bin').read_bytes()
    if (previous['status'] != 'pass' or previous['boothush_sha256'] != digest(exe.read_bytes())
            or previous['payload_sha256'] != digest(payload) or not previous['natural_start']
            or not previous['raw_keyboard'] or not previous['verify_crc16']
            or not previous['verify_geometry'] or not previous['retry_reads']
            or previous['media'] != 'hard-disk'):
        raise ValueError('session or emulator does not match the required profile')
    bss_end = previous['bss_start'] + previous['bss_bytes']
    # Leave enough of the emulator's 16,384-byte inspection budget for the
    # original session's 17-byte state record in the very same run.
    start = 0xc020
    if bss_end > start:
        raise ValueError('no room for the selected diagnostic watermark')
    # The recorded session's map supplies the linked near entry address.
    import re
    match = re.search(r'Entry point address:\s*([0-9a-fA-F]+)',
                      (session/'session.map').read_text(encoding='utf-8', errors='replace'))
    if not match:
        raise ValueError('missing linked entry')
    image = hard_disk(payload, int(match.group(1), 16), bss_end=bss_end,
                      poison_bss=True, verify_crc16=True, verify_geometry=True,
                      retry_reads=True, stack_canary_start=start)
    out.mkdir(parents=True, exist_ok=False)
    disk = out/'stack-canary.img'
    disk.write_bytes(image)
    # A separate guest pushes a known 256 bytes before stopping. This checks
    # the loader's watermark and memory reader, not the window workload.
    control_payload = bytes.fromhex('b98000 50 e2fd b0a5 e680 ebfe')
    control_image = hard_disk(control_payload, 0, bss_end=len(control_payload),
                              stack_canary_start=start)
    control_disk = out/'known-depth.img'
    control_disk.write_bytes(control_image)
    control_command = [str(exe), '--machine', 'pc286', '--hard-disk', str(control_disk),
                       '--cycles', '1000000', '--timeout-ms', '20000', '--until-post', '0xa5',
                       '--inspect', '0x1fe00:510']
    control_run = subprocess.run(control_command, capture_output=True, text=True, timeout=25,
                                 creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
    (out/'known-depth.log').write_text(control_run.stdout+'\n--- stderr ---\n'+control_run.stderr,
                                      encoding='utf-8')
    if control_run.returncode:
        raise RuntimeError('known-depth guest failed')
    control_result = json.loads(control_run.stdout)
    control_bytes = bytes.fromhex(control_result['memory']['bytes'])
    if (control_result['status'] != 'condition_met' or control_result['post'] != 0xa5 or
            len(control_bytes) != 510 or next((i for i,b in enumerate(control_bytes) if b != 0xa5), -1) != 254 or
            digest(control_disk.read_bytes()) != digest(control_image)):
        raise ValueError('known-depth guest did not mark exactly the expected lower boundary')
    ranges = [(0x10000+offset, min(4096, 0xfffe-offset))
              for offset in range(start, 0xfffe, 4096)]
    observations = []
    for case in previous['cases']:
        name = case['name']
        command = list(case['command'])
        command[0] = str(exe)
        command[command.index('--hard-disk')+1] = str(disk)
        if command[command.index('--inspect')+1] != '0x60000:17':
            raise ValueError(name+': unexpected state inspection')
        frame = out/(name+'.ppm')
        command[command.index('--frame')+1] = str(frame)
        for address, length in ranges:
            command += ['--inspect', f'{address:#x}:{length}']
        result = subprocess.run(command, capture_output=True, text=True, timeout=25,
                                creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        (out/(name+'.log')).write_text(result.stdout+'\n--- stderr ---\n'+result.stderr, encoding='utf-8')
        if result.returncode:
            raise RuntimeError(name+': BootHush failed')
        value = json.loads(result.stdout)
        old = case['guest_result']
        if (value['status'] != 'condition_met' or value['post'] != old['post']
                or digest(frame.read_bytes()) != case['ega_ppm_sha256']
                or digest(disk.read_bytes()) != digest(image)):
            raise ValueError(name+': status, pixels or disk mismatch')
        rows = value['memory_ranges']
        if (rows[0]['address'] != 0x60000 or rows[0]['bytes'] != old['memory']['bytes'] or
                [(r['address'],len(bytes.fromhex(r['bytes']))) for r in rows[1:]] != ranges):
            raise ValueError(name+': watermark memory ranges differ')
        data = b''.join(bytes.fromhex(r['bytes']) for r in rows[1:])
        changed = [i for i,byte in enumerate(data) if byte != 0xa5]
        if not changed or changed[0] == 0:
            raise ValueError(name+': no stack use or canary floor changed')
        # Verify the detector reacts to a synthetic lower write.
        control = bytearray(data); control[0] ^= 1
        if next(i for i,b in enumerate(control) if b != 0xa5) != 0:
            raise ValueError('watermark control escaped')
        observations.append({'name':name,'post':value['post'],'cycles':value['cycles'],
                             'lowest_changed_offset':start+changed[0],
                             'observed_depth_bytes':0xfffe-(start+changed[0]),
                             'changed_bytes':len(changed),'frame_sha256':digest(frame.read_bytes()),
                             'command':command})
    report = {'status':'pass','session_report_sha256':digest((session/'report.json').read_bytes()),
              'payload_sha256':digest(payload),'boothush_sha256':digest(exe.read_bytes()),
              'loader_sha256':digest((repo/'scripts/pc286_hard_disk_loader.py').read_bytes()),
              'test_sha256':digest(Path(__file__).read_bytes()),'disk_sha256':digest(image),
              'bss_end':bss_end,'canary_start':start,'canary_end_exclusive':0xfffe,
              'known_depth_control':{'expected_bytes':256,'disk_sha256':digest(control_image),
                                     'command':control_command},
              'control_floor_mutation_detected':True,'observations':observations,
              'limits':'Changed-byte watermark is a lower bound for these modeled runs, not a proof of maximum stack or physical memory fit; writes equal to the canary and unobserved paths may be missed.'}
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({'status':'pass','report':str(out/'report.json'),
                      'maximum_observed_depth':max(x['observed_depth_bytes'] for x in observations)}))


if __name__ == '__main__':
    main()
