#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Characterize a recorded raw-keyboard 286 session at shorter input periods."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--emulator', required=True, type=Path)
    p.add_argument('--pc-results', required=True, type=Path)
    p.add_argument('--output', required=True, type=Path)
    a = p.parse_args()
    exe = a.emulator.resolve(strict=True)
    session = a.pc_results.resolve(strict=True)
    out = a.output.resolve()
    if out == ROOT or ROOT in out.parents:
        raise ValueError('output must be outside the repository')
    report_path = session/'report.json'
    report = json.loads(report_path.read_text(encoding='utf-8'))
    image = session/'session.img'
    before = {str(path):sha(path) for path in (exe,report_path,image,Path(__file__))}
    if (report['status'] != 'pass' or report['media'] != 'hard-disk' or
            not report['natural_start'] or not report['raw_keyboard'] or
            report['boothush_sha256'] != sha(exe) or report['media_sha256'] != sha(image)):
        raise ValueError('wrong session, media or emulator identity')
    for name, expected in report['source_sha256'].items():
        path = (ROOT/name).resolve(strict=True)
        if ROOT not in path.parents or sha(path) != expected:
            raise ValueError('source changed: '+name)
    original = next(c for c in report['cases'] if c['name'] == 'content-release')
    original_command = original['command']
    start = report['keyboard_schedule_start']
    events = []
    for i, item in enumerate(original_command):
        if item == '--scancode':
            cycle, value = map(int, original_command[i+1].split(':'))
            group, offset = divmod(cycle-start, 5000000)
            if not 0 <= group < 15 or offset > 102000:
                raise ValueError('unexpected input schedule')
            events.append((group, offset, value))
    if len(events) != 63 or sorted({v[0] for v in events}) != list(range(15)):
        raise ValueError('unexpected input event count')
    out.mkdir(parents=True, exist_ok=False)
    cases = []
    for period in (5000000, 1000000, 500000, 250000, 125000, 62500, 31250, 10000):
        frame = out/f'{period}.ppm'
        command = [str(exe), '--machine', 'pc286', '--hard-disk', str(image),
                   '--cycles', '100000000', '--timeout-ms', '20000', '--until-post', '0xbf',
                   '--inspect', '0x60000:17', '--frame', str(frame)]
        # Preserve ordering within each make/break group even when the group
        # period is shorter than the original 102,000-cycle byte sequence.
        scale = min(period, 125000)
        for group, offset, value in events:
            command += ['--scancode', f'{start+group*period+offset*scale//125000}:{value}']
        result = subprocess.run(command, capture_output=True, text=True, timeout=25,
                                creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        (out/f'{period}.log').write_text(result.stdout+'\n--- stderr ---\n'+result.stderr, encoding='utf-8')
        try:
            data = json.loads(result.stdout)
        except json.JSONDecodeError as exc:
            raise ValueError('invalid BootHush result') from exc
        frame_hash = sha(frame) if frame.exists() else None
        state_match = data.get('memory') == original['guest_result']['memory']
        pixels_match = frame_hash == original['ega_ppm_sha256']
        input_accounting = data.get('pc_keyboard_input', {})
        complete = (input_accounting.get('scheduled') == len(events) and
                    input_accounting.get('accepted') == len(events) and
                    input_accounting.get('rejected') == 0 and input_accounting.get('pending') == 0)
        matched = (result.returncode == 0 and data.get('status') == 'condition_met' and
                   data.get('post') == 0xbf and state_match and pixels_match and complete)
        if period == 5000000 and not matched:
            raise ValueError('baseline did not reproduce the recorded final state')
        if sha(image) != report['media_sha256']:
            raise ValueError('input medium changed')
        checkpoints = []
        for expected in report['cases'][:-1]:
            name = expected['name']
            selected = list(command)
            selected[selected.index('--until-post')+1] = hex(expected['guest_result']['post'])
            checkpoint_frame = out/f'{period}-{name}.ppm'
            selected[selected.index('--frame')+1] = str(checkpoint_frame)
            observed = subprocess.run(selected, capture_output=True, text=True, timeout=25,
                                      creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
            (out/f'{period}-{name}.log').write_text(observed.stdout+'\n--- stderr ---\n'+observed.stderr,
                                                   encoding='utf-8')
            value = json.loads(observed.stdout)
            frame_digest = sha(checkpoint_frame) if checkpoint_frame.exists() else None
            checkpoint_match = (observed.returncode == 0 and value.get('status') == 'condition_met' and
                                value.get('post') == expected['guest_result']['post'] and
                                value.get('memory') == expected['guest_result']['memory'] and
                                frame_digest == expected['ega_ppm_sha256'])
            if period == 5000000 and not checkpoint_match:
                raise ValueError('baseline checkpoint failed: '+name)
            if sha(image) != report['media_sha256']:
                raise ValueError('input medium changed at '+name)
            checkpoints.append({'name':name,'matched':checkpoint_match,'result':value,
                                'frame_sha256':frame_digest,'command':selected})
        cases.append({'period_cycles':period,'intra_group_scale_numerator':scale,
                      'intra_group_scale_denominator':125000,
                      'last_scheduled_cycle':max(start+g*period+o*scale//125000 for g,o,_ in events),
                      'result':data,'exit_code':result.returncode,
                      'frame_sha256':frame_hash,'state_match':state_match,
                      'pixels_match':pixels_match,'all_input_accepted':complete,
                      'scenario_matched':matched,'checkpoints':checkpoints,'command':command})
    changed = bytearray.fromhex(original['guest_result']['memory']['bytes'])
    changed[-1] ^= 1
    if changed.hex() == cases[0]['result']['memory']['bytes']:
        raise ValueError('state mismatch control escaped')
    if before != {str(path):sha(path) for path in (exe,report_path,image,Path(__file__))}:
        raise ValueError('test input changed during the run')
    for name, expected in report['source_sha256'].items():
        if sha(ROOT/name) != expected:
            raise ValueError('source changed during the run: '+name)
    output = {'status':'characterized','session_report_sha256':sha(report_path),
              'script_sha256':sha(Path(__file__)),'boothush_sha256':sha(exe),
              'media_sha256':sha(image),'payload_sha256':report['payload_sha256'],
              'keyboard_schedule_start':start,'scheduled_bytes':len(events),
              'state_mutation_detected':True,'cases':cases,
              'limits':'Scripted modeled bytes and selected guest checkpoints only; acceptance does not prove consumption or physical timing, rollover, responsiveness or human qualification.'}
    (out/'report.json').write_text(json.dumps(output,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({'status':'characterized','report':str(out/'report.json'),
                      'cases':[{**{k:c[k] for k in ('period_cycles','scenario_matched','all_input_accepted')},
                                'checkpoints_matched':sum(x['matched'] for x in c['checkpoints'])}
                               for c in cases]}))


if __name__ == '__main__':
    main()
