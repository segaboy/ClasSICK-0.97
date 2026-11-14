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
NATURAL_EVENTS=('100000000:256:171:0','108000000:256:75:0','120000000:256:75:1',
                '132000000:272:75:2','144000000:272:59:2','156000000:272:59:3',
                '168000000:272:107:0','180000000:272:107:1','192000000:272:107:3')
NATURAL_CASES=(('initial',104000000,256,171,80,70,0,0,0),('title',118000000,256,75,80,70,0,0,0),
               ('pressed',130000000,256,75,80,70,0,1,1),('moved',154000000,272,59,96,54,0,1,1),
               ('released',166000000,272,59,96,54,0,0,1),('post-release',178000000,272,107,96,54,0,0,1),
               ('content',190000000,272,107,96,54,1,1,2),('content-release',202000000,272,107,96,54,1,0,2))

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def module(name,path):
    s=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--emulator',required=True,type=Path);p.add_argument('--mac-rom',required=True,type=Path)
    p.add_argument('--pc-results',required=True,type=Path);p.add_argument('--output',required=True,type=Path)
    p.add_argument('--mac-manifest',type=Path,help='source-matched experimental Mac build manifest')
    p.add_argument('--edge-cycles',type=int,help='optional modeled mouse edge interval');a=p.parse_args()
    try:
        exe=a.emulator.resolve(strict=True);rom=a.mac_rom.resolve(strict=True);pc=a.pc_results.resolve(strict=True);out=a.output.resolve()
        if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside the repository')
        mac_sha=sha(rom);manifest_sha=None
        if a.mac_manifest:
            manifest=a.mac_manifest.resolve(strict=True);build=json.loads(manifest.read_text(encoding='utf-8'))
            if (not build.get('twins_equal') or not build.get('window_drawing') or
                build.get('dcd_invert')!=3 or not build.get('short_collect') or
                not build.get('loss_counts') or build['profiles'][0]['image_sha256']!=mac_sha):
                raise ValueError('Mac manifest, profile or image mismatch')
            for name,digest in build['inputs'].items():
                item=(ROOT/name).resolve(strict=True)
                if ROOT not in item.parents:raise ValueError('Mac manifest input outside repository')
                if hashlib.sha256(item.read_bytes().replace(b'\r\n',b'\n')).hexdigest()!=digest:
                    raise ValueError('Mac source identity changed: '+name)
            symbols=build['profiles'][0]['symbols'];manifest_sha=sha(manifest)
            trace_address=symbols['cs_mac_trace'];diag=symbols['cs_mac_diag']
            loss=symbols['cs_mac_loss_trace'];drop=symbols['cs_mac_motion_drop_trace']
            start=min(trace_address,diag,loss,drop);end=max(trace_address+128,diag+28,loss+20,drop+8)
            if start<0 or end>0x2000 or end-start>4096:raise ValueError('Mac diagnostic symbols outside RAM')
        else:
            if mac_sha!=MAC_SHA:raise ValueError('Mac ROM identity mismatch')
            trace_address,diag,loss,drop,start,end=0xc28,0xca8,0xc04,0xc18,0xc04,0xcc4
        report=json.loads((pc/'report.json').read_text(encoding='utf-8'))
        media_sha=report.get('media_sha256') or report['floppy_sha256']
        media=report.get('media','floppy')
        if media not in ('floppy','hard-disk'):
            raise ValueError('unsupported 286 diagnostic media')
        expected_media_bytes=1228800 if media=='floppy' else 615*4*17*512
        if (pc/'session.img').stat().st_size!=expected_media_bytes:
            raise ValueError('286 diagnostic media size mismatch')
        if report['status']!='pass' or report['boothush_sha256']!=sha(exe) or sha(pc/'session.img')!=media_sha:
            raise ValueError('286 report, emulator or media identity mismatch')
        for name,digest in report['source_sha256'].items():
            if sha(ROOT/name)!=digest:raise ValueError('286 source identity changed: '+name)
        natural=report.get('natural_start',False)
        cases=NATURAL_CASES if natural else CASES;events=NATURAL_EVENTS if natural else EVENTS
        stop=cases[-1][1]
        oracle=module('compositor',ROOT/'scripts/Test-BootHushPC286SharedCompositor.py')
        out.mkdir(parents=True,exist_ok=False);series_path=out/'mac.pbmseq';trace=out/'mac.bhmouse'
        cmd=[str(exe),'--machine','mac128k','--rom',str(rom),'--cycles',str(stop),'--timeout-ms','20000',
             '--mouse-dcd-invert','3','--mouse-record',str(trace),'--capture-series',str(series_path),
             '--capture-memory',hex(start)+':'+str(end-start)]
        if a.edge_cycles is not None:cmd+=['--mouse-edge-cycles',str(a.edge_cycles)]
        for _,cycle,*_ in cases:cmd+=['--capture-cycle',str(cycle)]
        for event in events:cmd+=['--window-mouse',event]
        result=subprocess.run(cmd,capture_output=True,text=True,timeout=25,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        (out/'run.log').write_text(result.stdout+'\n--- stderr ---\n'+result.stderr,encoding='utf-8')
        if result.returncode:raise ValueError('Mac run failed; see run.log')
        data=json.loads(result.stdout);series=data['frame_series']
        if (data['status']!='cycle_limit' or data.get('input_source')!='injected_window_messages' or
            data.get('mouse',{}).get('pending')!=0 or series['rom_sha256']!=mac_sha or
            series['executable_sha256']!=sha(exe) or
            (series['requested'],series['processed'],series['pending'],series['missing_images'],series['dropped'])!=(len(cases),len(cases),0,0,0)):
            raise ValueError('Mac run incomplete or capture identity mismatch')
        payload=series_path.read_bytes();observations=[]
        pc_cases={case['name']:case for case in report['cases']}
        for expected,sample in zip(cases,series['samples']):
            name,cycle,px,py,wx,wy,pattern,held,presses=expected
            if sample['requested']!=cycle or not sample['processed'] or not sample['image_available']:
                raise ValueError(name+': missing requested Mac capture')
            memory=bytes.fromhex(sample['memory']['bytes'])
            if len(memory)!=end-start or sample['memory']['address']!=start:raise ValueError('unexpected diagnostics')
            def word(address):return int.from_bytes(memory[address-start:address-start+4],'big')
            if (word(trace_address)!=0x4d575031 or word(trace_address+4)!=2 or word(trace_address+8)!=128 or
                word(loss)!=0x4d4c4331 or word(drop)!=0x4d4d4431 or word(diag)!=0x43533039):
                raise ValueError(name+': diagnostic identity mismatch')
            actual={'pointer':[word(trace_address+52),word(trace_address+56)],'window':[word(trace_address+60),word(trace_address+64)],
                    'pattern':word(trace_address+68),'held':word(trace_address+88),'presses':word(diag+12),
                    'losses':[word(loss+i*4) for i in range(1,5)],'omissions':word(drop+4),
                    'stage':word(diag+4),'fault':word(diag+24)}
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
        replay_cmd=[str(exe),'--machine','mac128k','--rom',str(rom),'--cycles',str(stop),
                    '--timeout-ms','20000','--mouse-replay',str(trace),'--capture-series',str(replay_path),
                    '--capture-memory',hex(start)+':'+str(end-start)]
        for _,cycle,*_ in cases:replay_cmd+=['--capture-cycle',str(cycle)]
        replay=subprocess.run(replay_cmd,capture_output=True,text=True,timeout=25,
                              creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        (out/'replay.log').write_text(replay.stdout+'\n--- stderr ---\n'+replay.stderr,encoding='utf-8')
        if replay.returncode:raise ValueError('Mac replay failed; see replay.log')
        rep=json.loads(replay.stdout);rseries=rep['frame_series']
        if (rep['status']!='cycle_limit' or rep['cycles']!=data['cycles'] or
            rseries['processed']!=len(cases) or rseries['pending']!=0 or rseries['dropped']!=0 or
            replay_path.read_bytes()!=payload):raise ValueError('Mac replay frame or stop mismatch')
        for first,second in zip(series['samples'],rseries['samples']):
            if first['cycle']!=second['cycle'] or first['memory']!=second['memory']:
                raise ValueError('Mac replay memory or checkpoint cycle mismatch')
        if sha(rom)!=mac_sha or sha(pc/'session.img')!=media_sha:
            raise ValueError('input image changed during parity test')
        if a.mac_manifest and sha(manifest)!=manifest_sha:raise ValueError('Mac manifest changed during test')
        summary={'status':'pass','boothush_sha256':sha(exe),'mac_rom_sha256':mac_sha,'mac_manifest_sha256':manifest_sha,
                 'parity_script_sha256':sha(Path(__file__)),
                 'pc_payload_sha256':report['payload_sha256'],'pc_media_sha256':media_sha,'pc_media':media,
                 'pc_runtime_library_sha256':report['runtime_library_sha256'],'events':events,'natural_start':natural,
                 'requested_edge_cycles':a.edge_cycles,
                 'mouse':data['mouse'],'checkpoints':observations,'mac_replay_cycle_memory_frames_match':True,
                 'limits':'Injected Mac window messages and scheduled 286 keyboard; no hardware, human, production boot or release claim.'}
        if media=='floppy':summary['pc_floppy_sha256']=media_sha
        (out/'report.json').write_text(json.dumps(summary,indent=2)+'\n',encoding='utf-8')
        (out/'command.json').write_text(json.dumps(cmd,indent=2)+'\n',encoding='utf-8')
        print(json.dumps(summary,indent=2));return 0
    except (OSError,ValueError,KeyError,subprocess.TimeoutExpired) as exc:
        print('Mac/286 parity test failed: '+str(exc),file=sys.stderr);return 1

if __name__=='__main__':sys.exit(main())
