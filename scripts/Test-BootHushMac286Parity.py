#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Compare an original Mac ROM and the experimental 286 shared session."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
MAC_SHA='faf37c6f96f8eb2ee6cf7a1e58a8b1b9538055d2fc89493c7419eeea816f6c8c'
EVENTS=('100000000:256:171:0','108000000:100:80:0','120000000:100:80:1',
        '128000000:116:80:2','136000000:132:80:2','144000000:132:64:2',
        '152000000:132:64:3','160000000:148:64:0','168000000:148:80:0',
        '176000000:148:96:0','184000000:148:112:0','192000000:148:112:1',
        '200000000:148:112:3')
CASES=(('initial',116000000,100,80,80,70,0,0,0),('pressed',126000000,100,80,80,70,0,1,1),
       ('moved',142000000,132,80,112,70,0,1,1),('released',158000000,132,64,112,54,0,0,1),
       ('post-release',166000000,148,64,112,54,0,0,1),('content',198000000,148,112,112,54,1,1,2),
       ('content-release',206000000,148,112,112,54,1,0,2))

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def module(name,path):
    s=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--emulator',required=True,type=Path);p.add_argument('--mac-rom',required=True,type=Path)
    p.add_argument('--pc-results',required=True,type=Path);p.add_argument('--output',required=True,type=Path);a=p.parse_args()
    try:
        exe=a.emulator.resolve(strict=True);rom=a.mac_rom.resolve(strict=True);pc=a.pc_results.resolve(strict=True);out=a.output.resolve()
        if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside the repository')
        if sha(rom)!=MAC_SHA:raise ValueError('Mac ROM identity mismatch')
        report=json.loads((pc/'report.json').read_text(encoding='utf-8'))
        if report['status']!='pass' or report['boothush_sha256']!=sha(exe) or sha(pc/'session.img')!=report['floppy_sha256']:
            raise ValueError('286 report, emulator or floppy identity mismatch')
        for name,digest in report['source_sha256'].items():
            if sha(ROOT/name)!=digest:raise ValueError('286 source identity changed: '+name)
        oracle=module('compositor',ROOT/'scripts/Test-BootHushPC286SharedCompositor.py')
        out.mkdir(parents=True,exist_ok=False);series_path=out/'mac.pbmseq';trace=out/'mac.bhmouse'
        cmd=[str(exe),'--machine','mac128k','--rom',str(rom),'--cycles','206000000','--timeout-ms','20000',
             '--mouse-dcd-invert','3','--mouse-record',str(trace),'--capture-series',str(series_path),
             '--capture-memory','0xc04:192']
        for _,cycle,*_ in CASES:cmd+=['--capture-cycle',str(cycle)]
        for event in EVENTS:cmd+=['--window-mouse',event]
        result=subprocess.run(cmd,capture_output=True,text=True,timeout=25,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        (out/'run.log').write_text(result.stdout+'\n--- stderr ---\n'+result.stderr,encoding='utf-8')
        if result.returncode:raise ValueError('Mac run failed; see run.log')
        data=json.loads(result.stdout);series=data['frame_series']
        if (data['status']!='cycle_limit' or data.get('input_source')!='injected_window_messages' or
            data.get('mouse',{}).get('pending')!=0 or series['rom_sha256']!=MAC_SHA or
            series['executable_sha256']!=sha(exe) or
            (series['requested'],series['processed'],series['pending'],series['missing_images'],series['dropped'])!=(7,7,0,0,0)):
            raise ValueError('Mac run incomplete or capture identity mismatch')
        payload=series_path.read_bytes();observations=[]
        pc_cases={case['name']:case for case in report['cases']}
        for expected,sample in zip(CASES,series['samples']):
            name,cycle,px,py,wx,wy,pattern,held,presses=expected
            if sample['requested']!=cycle or not sample['processed'] or not sample['image_available']:
                raise ValueError(name+': missing requested Mac capture')
            memory=bytes.fromhex(sample['memory']['bytes'])
            if len(memory)!=192 or sample['memory']['address']!=0xc04:raise ValueError('unexpected diagnostics')
            def word(address):return int.from_bytes(memory[address-0xc04:address-0xc04+4],'big')
            actual={'pointer':[word(0xc5c),word(0xc60)],'window':[word(0xc64),word(0xc68)],
                    'pattern':word(0xc6c),'held':word(0xc80),'presses':word(0xcb4),
                    'losses':[word(0xc04+i*4) for i in range(1,5)],'omissions':word(0xc1c),
                    'stage':word(0xcac),'fault':word(0xcc0)}
            want={'pointer':[px,py],'window':[wx,wy],'pattern':pattern,'held':held,'presses':presses,
                  'losses':[0,0,0,0],'omissions':0,'stage':16,'fault':0}
            if actual!=want:raise ValueError(name+': unexpected Mac state '+str(actual))
            frame=payload[sample['offset']:sample['offset']+sample['length']]
            expected_pc=pc_cases[name];pcframe=pc/(name+'.ppm')
            if sha(pcframe)!=expected_pc['ega_ppm_sha256']:raise ValueError(name+': changed 286 frame')
            oracle.compare(frame,pcframe.read_bytes(),margin_white=True)
            mac_frame=out/(name+'.pbm')
            with mac_frame.open('xb') as f:f.write(frame)
            observations.append({'name':name,'mac_cycle':sample['cycle'],'mac_state':actual,
                'mac_pbm_sha256':sha(mac_frame),'pc_ega_ppm_sha256':sha(pcframe),
                'normalized_canvas_pixels_match':512*342,'pc_white_margin_pixels_match':640*350-512*342})
        replay_path=out/'replay.pbmseq'
        replay_cmd=[str(exe),'--machine','mac128k','--rom',str(rom),'--cycles','206000000',
                    '--timeout-ms','20000','--mouse-replay',str(trace),'--capture-series',str(replay_path),
                    '--capture-memory','0xc04:192']
        for _,cycle,*_ in CASES:replay_cmd+=['--capture-cycle',str(cycle)]
        replay=subprocess.run(replay_cmd,capture_output=True,text=True,timeout=25,
                              creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        (out/'replay.log').write_text(replay.stdout+'\n--- stderr ---\n'+replay.stderr,encoding='utf-8')
        if replay.returncode:raise ValueError('Mac replay failed; see replay.log')
        rep=json.loads(replay.stdout);rseries=rep['frame_series']
        if (rep['status']!='cycle_limit' or rep['cycles']!=data['cycles'] or
            rseries['processed']!=7 or rseries['pending']!=0 or rseries['dropped']!=0 or
            replay_path.read_bytes()!=payload):raise ValueError('Mac replay frame or stop mismatch')
        for first,second in zip(series['samples'],rseries['samples']):
            if first['cycle']!=second['cycle'] or first['memory']!=second['memory']:
                raise ValueError('Mac replay memory or checkpoint cycle mismatch')
        summary={'status':'pass','boothush_sha256':sha(exe),'mac_rom_sha256':MAC_SHA,
                 'pc_payload_sha256':report['payload_sha256'],'pc_floppy_sha256':report['floppy_sha256'],
                 'pc_runtime_library_sha256':report['runtime_library_sha256'],'events':EVENTS,
                 'mouse':data['mouse'],'checkpoints':observations,'mac_replay_cycle_memory_frames_match':True,
                 'limits':'Injected Mac window messages and scheduled 286 keyboard; no hardware, human, production boot or release claim.'}
        (out/'report.json').write_text(json.dumps(summary,indent=2)+'\n',encoding='utf-8')
        (out/'command.json').write_text(json.dumps(cmd,indent=2)+'\n',encoding='utf-8')
        print(json.dumps(summary,indent=2));return 0
    except (OSError,ValueError,KeyError,subprocess.TimeoutExpired) as exc:
        print('Mac/286 parity test failed: '+str(exc),file=sys.stderr);return 1

if __name__=='__main__':sys.exit(main())
