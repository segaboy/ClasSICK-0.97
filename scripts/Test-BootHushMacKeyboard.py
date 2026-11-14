#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Exercise the opt-in keyboard ROM with a concurrent headless mouse sequence.

This fixture applies only to the KBD1/MWP1 layouts from SPEC-0100. It does
not qualify physical input, timing, host capture, or the paired release.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.dont_write_bytecode = True


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--emulator', required=True, type=Path)
    p.add_argument('--rom', required=True, type=Path)
    p.add_argument('--rom-sha256', required=True)
    p.add_argument('--output', required=True, type=Path)
    a = p.parse_args()
    out = a.output.resolve()
    if out == ROOT or ROOT in out.parents:
        raise ValueError('output must be outside the repository')
    if a.rom.stat().st_size != 65536 or sha(a.rom) != a.rom_sha256:
        raise ValueError('ROM size or hash mismatch')
    out.mkdir(parents=True, exist_ok=False)
    spec = importlib.util.spec_from_file_location('window_test', ROOT/'scripts/Test-BootHushMacWindow.py')
    window = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(window)
    results = {}
    for name, keyed in (('mouse', False), ('mixed', True), ('overlap', True)):
        frame = out/(name+'.pbm')
        args = [a.emulator, '--machine', 'mac128k', '--rom', a.rom,
                '--cycles', '165000000', '--timeout-ms', '30000',
                '--mouse-dcd-invert', '3', '--input-after', '0xc48:4:1',
                '--inspect', '0xb00:1024', '--inspect', '0x8000:512', '--frame', frame]
        events = []
        for event in window.EVENTS:
            cycle, tail = event.split(':', 1)
            events.append((int(cycle)-window.EVENT_ORIGIN, '--window-mouse', tail))
        if keyed:
            if name=='mixed':
                events += [(40000000, '--key', '0:1'), (40500000, '--key', '0:0'),
                           (50000000, '--key', '1:1'), (50500000, '--key', '1:0')]
                args += ['--video', out/'mixed.y4m', '--video-start', '57000000',
                         '--video-fps', '5', '--video-frames', '70']
            else:
                events += [(40000000, '--key', '0:1'), (40500000, '--key', '1:1'),
                           (50000000, '--key', '0:0'), (50500000, '--key', '1:0')]
            args += ['--keyboard-witness', out/(name+'-keyboard.txt'),
                     '--keyboard-witness-range', '96800000:108000000']
        for cycle, option, tail in sorted(events, key=lambda item: item[0]):
            args += [option, f'{cycle}:{tail}']
        run = subprocess.run([str(x) for x in args], stdin=subprocess.DEVNULL,
                             capture_output=True, timeout=45, check=False,
                             creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        (out/(name+'.json')).write_bytes(run.stdout)
        (out/(name+'.stderr')).write_bytes(run.stderr)
        if run.returncode:
            raise ValueError(f'{name}: emulator exited {run.returncode}')
        result = json.loads(run.stdout)
        state = window.decode(result)
        expected = {'pointer':[150,110], 'window':[112,46], 'presses':3,
                    'pattern':1, 'held':0, 'hard_losses':[0,0,0,0],
                    'omissions':0, 'stage':16, 'fault':0}
        schedule = result.get('input_schedule', {})
        if (result.get('status') != 'cycle_limit' or state != expected or
                not schedule.get('ready') or schedule.get('accepted') != len(events) or
                schedule.get('scheduled') != len(events) or result['mouse']['pending']):
            raise ValueError(f'{name}: mouse or readiness failure: {state}, {schedule}')
        data = bytes.fromhex(result['memory_ranges'][0]['bytes'])
        keyboard = [int.from_bytes(data[x-0xb00:x-0xb00+4], 'big')
                    for x in range(0xce8, 0xd20, 4)]
        want = ([0x4b424431,4,0,0,0,0,1,0,3,2,0,0,0,0] if name=='mixed' else
                [0x4b424431,4,0,0,0,0,1,3,0,2,0,0,0,0] if name=='overlap' else
                [0x4b424431]+[0]*13)
        if keyboard != want:
            raise ValueError(f'{name}: keyboard record mismatch: {keyboard}')
        if keyed:
            lines=(out/(name+'-keyboard.txt')).read_text().splitlines()
            header=lines[0].split()
            delivered=[int(line.split()[3]) for line in lines[2:]
                       if line.split()[2]=='key_delivered']
            expected_delivery=[1,129,3,131] if name=='mixed' else [1,3,129,131]
            if (header[0]!='BHMACKEY1' or header[5:7]!=['0','0'] or
                    delivered != expected_delivery or
                    (name=='mixed' and (result['video']['frames']<2 or result['video']['truncated']))):
                raise ValueError(f'keyboard witness or capture failure: {header} {delivered}')
        results[name]={'guest':state, 'keyboard':keyboard, 'schedule':schedule,
                       'frame_sha256':sha(frame), 'video':result.get('video')}
    if (out/'mouse.pbm').read_bytes() != (out/'mixed.pbm').read_bytes():
        raise ValueError('mouse-only and mixed final framebuffers differ')
    receipt={'emulator_sha256':sha(a.emulator), 'rom_sha256':sha(a.rom),
             'cases':results, 'physical_input_qualified':False}
    (out/'report.json').write_text(json.dumps(receipt,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(receipt,indent=2))


if __name__=='__main__':
    main()
