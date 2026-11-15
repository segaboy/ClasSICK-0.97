#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Prepare a local, explicitly unqualified Mac/286 engineering snapshot."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]


def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('emulator','mac-rom','mac-manifest','pc-results','parity-report','output'):
        p.add_argument('--'+name,required=True,type=Path)
    a=p.parse_args()
    exe=a.emulator.resolve(strict=True);rom=a.mac_rom.resolve(strict=True)
    manifest=a.mac_manifest.resolve(strict=True);pc=a.pc_results.resolve(strict=True)
    paired=a.parity_report.resolve(strict=True);out=a.output.resolve()
    if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside repository')
    mac=json.loads(manifest.read_text(encoding='utf-8'))
    pcreport=pc/'report.json';pcdata=json.loads(pcreport.read_text(encoding='utf-8'))
    parity=json.loads(paired.read_text(encoding='utf-8'));disk=pc/'session.img'
    if (not mac.get('twins_equal') or not mac.get('window_drawing') or
        not mac.get('short_collect') or not mac.get('loss_counts') or mac.get('dcd_invert')!=3 or
        mac['profiles'][0]['image_sha256']!=sha(rom)):
        raise ValueError('Mac build profile or image mismatch')
    for name,digest in mac['inputs'].items():
        item=(ROOT/name).resolve(strict=True)
        if ROOT not in item.parents or hashlib.sha256(item.read_bytes().replace(b'\r\n',b'\n')).hexdigest()!=digest:
            raise ValueError('Mac source identity changed: '+name)
    if (pcdata.get('status')!='pass' or pcdata.get('media')!='hard-disk' or
        not pcdata.get('natural_start') or not pcdata.get('raw_keyboard') or
        bool(pcdata.get('verify_payload'))==bool(pcdata.get('verify_crc16')) or
        pcdata.get('media_sha256')!=sha(disk) or pcdata.get('payload_sha256')!=sha(pc/'session.bin') or
        pcdata.get('boothush_sha256')!=sha(exe)):
        raise ValueError('286 session identity or profile mismatch')
    for name,digest in pcdata['source_sha256'].items():
        item=(ROOT/name).resolve(strict=True)
        if ROOT not in item.parents or sha(item)!=digest:raise ValueError('286 source identity changed: '+name)
    names=('initial','title','pressed','moved','released','post-release','content','content-release')
    if (parity.get('status')!='pass' or parity.get('boothush_sha256')!=sha(exe) or
        parity.get('mac_rom_sha256')!=sha(rom) or parity.get('mac_manifest_sha256')!=sha(manifest) or
        parity.get('pc_media')!='hard-disk' or parity.get('pc_media_sha256')!=sha(disk) or
        parity.get('pc_payload_sha256')!=sha(pc/'session.bin') or
        parity.get('parity_script_sha256')!=sha(ROOT/'scripts/Test-BootHushMac286Parity.py') or
        parity.get('requested_edge_cycles')!=4096 or parity.get('requested_vbl_guard')!=4096 or
        not parity.get('mac_replay_cycle_memory_frames_match') or
        tuple(row['name'] for row in parity['checkpoints'])!=names):
        raise ValueError('paired observation or source identity mismatch')
    for row in parity['checkpoints']:
        if (row['mac_state']['fault'] or row['mac_state']['omissions'] or any(row['mac_state']['losses']) or
            row['normalized_canvas_pixels_match']!=512*342 or row['pc_white_margin_pixels_match']!=640*350-512*342):
            raise ValueError('paired checkpoint did not satisfy modeled checks')
    frames={row['name']:row for row in parity['checkpoints']}
    for name in ('initial','content'):
        if sha(paired.parent/(name+'.pbm'))!=frames[name]['mac_pbm_sha256']:
            raise ValueError('paired frame identity mismatch: '+name)
    head=subprocess.run(['git','rev-parse','HEAD'],cwd=ROOT,capture_output=True,text=True,check=True,timeout=10).stdout.strip()
    inputs={'mac.rom':rom,'pc286-diagnostic.img':disk,'initial.pbm':paired.parent/'initial.pbm',
            'content.pbm':paired.parent/'content.pbm','mac-build-manifest.json':manifest,
            'pc286-session-report.json':pcreport,'paired-report.json':paired}
    input_hashes={name:sha(path) for name,path in inputs.items()}
    exe_hash=sha(exe);out.mkdir(parents=True,exist_ok=False)
    for name,path in inputs.items():
        with path.open('rb') as src,(out/name).open('xb') as dst:shutil.copyfileobj(src,dst,1024*1024)
        if sha(out/name)!=input_hashes[name]:raise ValueError('copy verification failed: '+name)
    note=('UNQUALIFIED ENGINEERING SNAPSHOT — NOT A RELEASE OR INSTALLER\n\n'
          'The 286 image is an original whole-disk diagnostic for BootHush, not a production boot or installation medium. '
          'The paired results are scheduled emulator input, not human or physical testing.\n\n'
          'Open work includes production 286 boot and safe installation, Mac fast-input behavior, source/tool and '
          'provenance reviews, physical and human qualification, release packaging, and the separate owner publication decision.\n'
          'The 286 diagnostic uses '+('CRC-16' if pcdata.get('verify_crc16') else 'an additive sum')+' for accidental payload errors; neither authenticates it.\n'
          'The PBM files are guest-only Mac frames from the paired model run.\n\n'
          'For a bounded headless check, set $snapshot to this directory and $bh to the recorded BootHush executable.\n'
          f'& $bh --machine mac128k --rom "$snapshot/mac.rom" --cycles 100000000 --timeout-ms 20000 --until {hex(mac["profiles"][0]["symbols"]["cs_mac_diag"]+4)}:4:16\n'
          '& $bh --machine pc286 --hard-disk "$snapshot/pc286-diagnostic.img" --cycles 10000000 --timeout-ms 20000 --until-post 0xb0 --inspect 0x60000:17\n\n'
          'These checks show modeled readiness only; they do not replace the paired interaction tests.\n'
          'See docs/development/mac-286-paired-direction-20261114.md and the linked evidence in the source repository.\n')
    (out/'README.txt').write_text(note,encoding='utf-8')
    record={'classification':'unqualified engineering snapshot; not a release candidate',
            'repository_head':head,'preparation_script_sha256':sha(Path(__file__)),
            'boothush_sha256':exe_hash,'input_sha256':input_hashes,
            'mac_rom_sha256':sha(rom),'pc_disk_sha256':sha(disk),'pc_payload_sha256':sha(pc/'session.bin'),
            'pc_payload_check':'crc16' if pcdata.get('verify_crc16') else 'additive-sum',
            'limits':'Self-reported build/source identities are integrity checks, not independent review, physical proof or authenticity.'}
    (out/'snapshot.json').write_text(json.dumps(record,indent=2)+'\n',encoding='utf-8')
    files=[out/name for name in (*inputs.keys(),'README.txt','snapshot.json')]
    (out/'SHA256SUMS').write_text(''.join(sha(path)+'  '+path.name+'\n' for path in files),encoding='ascii')
    if any(sha(path)!=input_hashes[name] for name,path in inputs.items()) or sha(exe)!=exe_hash:
        raise ValueError('input changed during snapshot preparation')
    print(json.dumps({'status':'pass','classification':record['classification'],'directory':str(out)}))


if __name__=='__main__':
    try:main()
    except (OSError,ValueError,KeyError,subprocess.SubprocessError) as exc:
        print('engineering snapshot failed: '+str(exc),file=sys.stderr);sys.exit(1)
