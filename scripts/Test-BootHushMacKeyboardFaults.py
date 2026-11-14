#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Check injected VIA keyboard faults in the SPEC-0100 ROM; no physical claim."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
ROM_HASH = 'b3a50ad806b589fdc02a4de579e6d72700c9107fb273d9ca73026320bebed807'

def digest(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--emulator', required=True, type=Path)
    parser.add_argument('--rom', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    out = args.output.resolve()
    if out == ROOT or ROOT in out.parents or args.rom.stat().st_size != 65536 or digest(args.rom) != ROM_HASH:
        raise ValueError('output location or pinned ROM mismatch')
    out.mkdir(parents=True, exist_ok=False)
    cases = [('normal',None,0,0),('stall','0x14:1:1:0',0,1),
             ('no-reply','0x14:1:2:0',0,1),('delay-before','0x14:1:3:991993',0,0),
             ('delay-after','0x14:1:3:991994',0,1),('invalid','0x14:1:4:0',1,0),
             ('invalid-followup','0x14:1:4:0',1,0),
             ('disconnect','0x14:1:5:1500000',0,1)]
    results = []
    for name, fault, invalid, timeout in cases:
        observed = []
        for trace in (False,True):
            cmd = [args.emulator,'--machine','mac128k','--rom',args.rom,
                   '--cycles','65000000','--timeout-ms','20000',
                   '--inspect','0xce8:56','--inspect','0xca8:28']
            if fault:
                cmd += ['--keyboard-fault',fault]
            if name=='invalid-followup':
                cmd += ['--key','60000000:0:1','--key','61000000:0:0']
            if trace:
                cmd += ['--keyboard-witness',out/(name+'.bhkey'),
                        '--keyboard-witness-range',
                        '56800000:63000000' if name=='invalid-followup' else '56800000:59000000']
            run = subprocess.run([str(x) for x in cmd],stdin=subprocess.DEVNULL,
                                 capture_output=True,timeout=30,check=False,
                                 creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            (out/(name+('-trace' if trace else '-plain')+'.json')).write_bytes(run.stdout)
            if run.returncode:
                raise ValueError(f'{name}: emulator returned {run.returncode}: {run.stderr[:300]!r}')
            result=json.loads(run.stdout)
            ranges=result['memory_ranges']
            if [r['address'] for r in ranges] != [0xce8,0xca8]:
                raise ValueError(f'{name}: incorrect memory spans')
            keyboard=bytes.fromhex(ranges[0]['bytes'])
            diag=bytes.fromhex(ranges[1]['bytes'])
            words=[int.from_bytes(keyboard[i:i+4],'big') for i in range(0,56,4)]
            event_count=2 if name=='invalid-followup' else 0
            if (result['status']!='cycle_limit' or words[:4]!=[0x4b424431,event_count,invalid,timeout] or
                    int.from_bytes(diag[:4],'big')!=0x43533039 or
                    int.from_bytes(diag[4:8],'big')!=16 or int.from_bytes(diag[24:28],'big')!=0):
                raise ValueError(f'{name}: unexpected guest result {words[:4]}')
            if name=='invalid-followup' and words[4:8]!=[0,0,1,0]:
                raise ValueError('invalid-followup: press/release or final key state mismatch')
            if fault and not result.get('keyboard_fault',{}).get('triggered'):
                raise ValueError(f'{name}: injection did not trigger')
            if trace:
                witness=result['keyboard_witness']
                if witness['dropped'] or witness['rejected'] or not witness['range_reached']:
                    raise ValueError(f'{name}: incomplete witness')
                if name=='invalid-followup':
                    lines=(out/(name+'.bhkey')).read_text().splitlines()[2:]
                    delivered=[int(row.split()[3]) for row in lines if row.split()[2]=='key_delivered']
                    if delivered != [1,129]:
                        raise ValueError(f'{name}: wrong delivered replies {delivered}')
            observed.append({'cycles':result['cycles'],'pc':result['pc'],
                             'memory_ranges':ranges,'fault':result.get('keyboard_fault')})
        if observed[0]!=observed[1]:
            raise ValueError(f'{name}: witness changed guest state')
        results.append({'case':name,'injection':fault,'invalid_count':invalid,
                        'timeout_latched':timeout,'noninterference':True,
                        'result':observed[0]})
    report={'emulator_sha256':digest(args.emulator),'rom_sha256':digest(args.rom),
            'cases':results,'physical_keyboard_qualified':False}
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(report,indent=2))

if __name__=='__main__':
    main()
