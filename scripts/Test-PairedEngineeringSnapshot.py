#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Test local snapshot identity refusal and preservation using original fixtures."""
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
    a=p.parse_args();exe=a.emulator.resolve(strict=True);rom=a.mac_rom.resolve(strict=True)
    manifest=a.mac_manifest.resolve(strict=True);pc=a.pc_results.resolve(strict=True)
    parity=a.parity_report.resolve(strict=True);out=a.output.resolve()
    if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside repository')
    before={str(x):sha(x) for x in (exe,rom,manifest,pc/'session.img',pc/'session.bin',pc/'report.json',parity,
                                     parity.parent/'initial.pbm',parity.parent/'content.pbm')}
    out.mkdir(parents=True,exist_ok=False);cases=[]
    def run(name,r=rom,m=manifest,c=pc,q=parity,expect_success=False,target=None,expected_error=None):
        destination=target if target else out/(name+'-snapshot')
        readiness=[]
        cmd=[sys.executable,'-B',str(ROOT/'scripts/Prepare-PairedEngineeringSnapshot.py'),
             '--emulator',str(exe),'--mac-rom',str(r),'--mac-manifest',str(m),
             '--pc-results',str(c),'--parity-report',str(q),'--output',str(destination)]
        result=subprocess.run(cmd,capture_output=True,text=True,timeout=30,
                              creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        if expect_success:
            if result.returncode:raise ValueError(name+': '+result.stderr)
            for line in (destination/'SHA256SUMS').read_text(encoding='ascii').splitlines():
                digest,filename=line.split('  ',1)
                if sha(destination/filename)!=digest:raise ValueError('snapshot checksum mismatch: '+filename)
            if 'NOT A RELEASE' not in (destination/'README.txt').read_text(encoding='utf-8'):
                raise ValueError('snapshot classification missing')
            snapshot=json.loads((destination/'snapshot.json').read_text(encoding='utf-8'))
            session=json.loads((c/'report.json').read_text(encoding='utf-8'))
            if snapshot['pc_geometry_preflight']!=bool(session.get('verify_geometry')):
                raise ValueError('snapshot geometry profile not recorded')
            diag=json.loads(manifest.read_text(encoding='utf-8'))['profiles'][0]['symbols']['cs_mac_diag']
            commands=([str(exe),'--machine','mac128k','--rom',str(destination/'mac.rom'),
                       '--cycles','100000000','--timeout-ms','20000','--until',hex(diag+4)+':4:16'],
                      [str(exe),'--machine','pc286','--hard-disk',str(destination/'pc286-diagnostic.img'),
                       '--cycles','10000000','--timeout-ms','20000','--until-post','0xb0','--inspect','0x60000:17'])
            for command in commands:
                observed=subprocess.run(command,capture_output=True,text=True,timeout=25,
                                        creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
                if observed.returncode:raise ValueError('snapshot readiness failed: '+observed.stderr)
                state=json.loads(observed.stdout)
                if state.get('status')!='condition_met' or (state.get('machine')=='pc286' and state.get('post')!=0xb0):
                    raise ValueError('snapshot readiness condition not met')
                readiness.append({'command':command,'result':state})
            for line in (destination/'SHA256SUMS').read_text(encoding='utf-8').splitlines():
                digest,filename=line.split('  ',1)
                if sha(destination/filename)!=digest:raise ValueError('readiness changed snapshot: '+filename)
        elif result.returncode==0 or (target is None and destination.exists()):
            raise ValueError(name+': invalid input accepted or output created')
        if expected_error and expected_error not in result.stderr:
            raise ValueError(name+': unexpected refusal: '+result.stderr)
        cases.append({'name':name,'command':cmd,'exit_code':result.returncode,'stderr':result.stderr.strip(),
                      'readiness':readiness})
    run('baseline',expect_success=True)
    damaged=bytearray(rom.read_bytes());damaged[1024]^=1
    altered_rom=out/'altered.rom';altered_rom.write_bytes(damaged)
    run('rom',r=altered_rom,expected_error='Mac build profile or image mismatch')
    mac=json.loads(manifest.read_text(encoding='utf-8'));mac['inputs']['core/graphics/surface.c']='0'*64
    altered_manifest=out/'altered-manifest.json';altered_manifest.write_text(json.dumps(mac),encoding='utf-8')
    run('source',m=altered_manifest,expected_error='Mac source identity changed: core/graphics/surface.c')
    altered_pc=out/'altered-pc';altered_pc.mkdir()
    for name in ('session.img','session.bin'):shutil.copyfile(pc/name,altered_pc/name)
    data=json.loads((pc/'report.json').read_text(encoding='utf-8'));data['media_sha256']='0'*64
    (altered_pc/'report.json').write_text(json.dumps(data),encoding='utf-8')
    run('disk_identity',c=altered_pc,expected_error='286 session identity or profile mismatch')
    for name,additive,crc in (('no_payload_check',False,False),('conflicting_payload_checks',True,True)):
        data=json.loads((pc/'report.json').read_text(encoding='utf-8'))
        data['verify_payload']=additive;data['verify_crc16']=crc
        (altered_pc/'report.json').write_text(json.dumps(data),encoding='utf-8')
        run(name,c=altered_pc,expected_error='286 session identity or profile mismatch')
    data=json.loads(parity.read_text(encoding='utf-8'));data['parity_script_sha256']='0'*64
    altered_parity=out/'altered-parity.json';altered_parity.write_text(json.dumps(data),encoding='utf-8')
    run('parity_identity',q=altered_parity,expected_error='paired observation or source identity mismatch')
    altered_frames=out/'altered-frames';altered_frames.mkdir()
    for name in ('initial.pbm','content.pbm'):shutil.copyfile(parity.parent/name,altered_frames/name)
    damaged_frame=altered_frames/'initial.pbm';data=bytearray(damaged_frame.read_bytes());data[-1]^=1
    damaged_frame.write_bytes(data)
    frame_report=altered_frames/'report.json';shutil.copyfile(parity,frame_report)
    run('frame_identity',q=frame_report,expected_error='paired frame identity mismatch: initial')
    existing=out/'existing';existing.mkdir();sentinel=existing/'keep.txt';sentinel.write_text('preserve',encoding='utf-8')
    run('existing_output',target=existing)
    if sentinel.read_text(encoding='utf-8')!='preserve' or len(list(existing.iterdir()))!=1:
        raise ValueError('existing output changed')
    if before!={str(Path(x)):sha(Path(x)) for x in before}:raise ValueError('test input changed')
    report={'status':'pass','cases':cases,'input_sha256':before,
            'test_script_sha256':sha(Path(__file__)),'snapshot_script_sha256':sha(ROOT/'scripts/Prepare-PairedEngineeringSnapshot.py'),
            'limits':'Identity and preservation checks; not independent provenance, hardware or release qualification.'}
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({'status':'pass','case_count':len(cases),'report':str(out/'report.json')}))


if __name__=='__main__':main()
