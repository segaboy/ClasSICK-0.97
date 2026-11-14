#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Copy a diagnostic to a fresh modeled disk, verify it, and boot the copy.

Only newly generated media are used. This is a destructive whole-disk test,
not an installer for a real disk or a production partitioning scheme.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

from pc286_loader import floppy, MAX_PAYLOAD
from pc286_hard_disk_loader import DISK_BYTES, SECTOR

ROOT = Path(__file__).resolve().parents[1]


def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def stage2(prefix):
    if not prefix or len(prefix) % SECTOR:
        raise ValueError('sector-aligned nonempty target required')
    count = len(prefix)//SECTOR;blob = 128;scratch = (blob+len(prefix)+15)&~15
    if count > 127 or scratch+len(prefix) > 0xf000 or blob+len(prefix) > MAX_PAYLOAD:
        raise ValueError('diagnostic staging exceeds memory or disk limits')
    code = bytearray.fromhex('b80010 8ed8 8ec0 fb')
    branches = []
    for operation, address in ((3,blob),(2,scratch)):
        code += b'\xbb'+struct.pack('<H',address)+bytes((0xb8,count,operation))
        code += bytes.fromhex('b90100 ba8000 cd13 72')
        branches.append(len(code));code.append(0)
    code += bytes.fromhex('fc be')+struct.pack('<H',blob)
    code += b'\xbf'+struct.pack('<H',scratch)+b'\xb9'+struct.pack('<H',len(prefix))
    code += bytes.fromhex('f3a6 75');branches.append(len(code));code.append(0)
    code += bytes.fromhex('b0ac e680 ebfe')
    failures = []
    for value in (0xe1,0xe2,0xe3):
        failures.append(len(code));code += bytes((0xb0,value,0xe6,0x80,0xeb,0xfe))
    for offset,target in zip(branches,failures):
        delta = target-offset-1
        if not -128 <= delta <= 127:raise ValueError('branch outside diagnostic range')
        code[offset] = delta & 255
    if len(code)>blob:raise ValueError('installer exceeds staging offset')
    return bytes(code)+bytes(blob-len(code))+prefix


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--emulator',required=True,type=Path)
    p.add_argument('--session',required=True,type=Path)
    p.add_argument('--output',required=True,type=Path)
    a=p.parse_args();exe=a.emulator.resolve(strict=True);session=a.session.resolve(strict=True);out=a.output.resolve()
    if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside repository')
    report_path=session/'report.json';prior=json.loads(report_path.read_text(encoding='utf-8'))
    disk=session/'session.img';payload=session/'session.bin'
    if (prior.get('status')!='pass' or prior.get('media')!='hard-disk' or
        prior.get('media_sha256')!=sha(disk) or prior.get('payload_sha256')!=sha(payload)):
        raise ValueError('source session identity mismatch')
    for name,digest in prior['source_sha256'].items():
        if sha(ROOT/name)!=digest:raise ValueError('source session changed: '+name)
    before={'disk':sha(disk),'payload':sha(payload),'emulator':sha(exe),'session_report':sha(report_path)}
    image=disk.read_bytes();prefix_size=((SECTOR+len(payload.read_bytes())+SECTOR-1)//SECTOR)*SECTOR
    prefix=image[:prefix_size]
    if len(image)!=DISK_BYTES or any(image[prefix_size:]):
        raise ValueError('source is not the expected original diagnostic image')
    out.mkdir(parents=True,exist_ok=False)
    source=out/'installer-floppy.img';source.write_bytes(floppy(stage2(prefix),0))
    blank=out/'blank-disk.img';blank.write_bytes(bytes(DISK_BYTES))
    blank_sha=sha(blank);source_sha=sha(source)

    def run(name,extra,expected):
        cmd=[str(exe),'--machine','pc286',*map(str,extra),'--cycles','12000000','--timeout-ms','20000']
        result=subprocess.run(cmd,capture_output=True,text=True,timeout=25,
                              creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        (out/(name+'.log')).write_text(result.stdout+'\n--- stderr ---\n'+result.stderr,encoding='utf-8')
        if result.returncode:raise RuntimeError(name+': '+result.stdout+result.stderr)
        response=json.loads(result.stdout)
        if response['status']!='condition_met' or response['post']!=expected:
            raise ValueError(name+': expected condition not reached')
        return {'command':cmd,'result':response}

    installed=out/'installed.img'
    copy=run('copy',['--floppy',source,'--hard-disk',blank,'--disk-out',installed,'--until-post','0xac'],0xac)
    if sha(installed)!=before['disk']:
        raise ValueError('readback succeeded but resulting disk differs from source image')
    boot=run('boot',['--hard-disk',installed,'--until-post','0xb0','--inspect','0x60000:17'],0xb0)
    expected=bytes.fromhex(prior['cases'][0]['guest_result']['memory']['bytes'])
    if bytes.fromhex(boot['result']['memory']['bytes'])!=expected:
        raise ValueError('installed diagnostic did not reach expected initial state')
    failed_write=out/'failed-write.img'
    write_fault=run('write-failure',['--floppy',source,'--hard-disk',blank,'--disk-out',failed_write,
                                     '--until-post','0xe1','--fail-hard-disk-write','0:0:1'],0xe1)
    if write_fault['result']['hard_disk_write_failure']['injected']!=1 or sha(failed_write)!=blank_sha:
        raise ValueError('write failure did not preserve the modeled blank disk')
    failed_read=out/'failed-read-after-write.img'
    fault=run('read-failure',['--floppy',source,'--hard-disk',blank,'--disk-out',failed_read,'--until-post','0xe2',
                              '--fail-hard-disk-read','0:0:1'],0xe2)
    if fault['result']['hard_disk_read_failure']['injected']!=1:
        raise ValueError('verification read failure was not injected')
    if sha(failed_read)!=before['disk']:
        raise ValueError('read failure did not retain the preceding completed write')
    # Change a staged padding byte: guest readback still agrees with its input,
    # but the host's whole-image identity check must reject the resulting disk.
    mutant=bytearray(source.read_bytes());mutant[SECTOR+128+prefix_size-1]^=1
    mutated_source=out/'mutated-installer-floppy.img';mutated_source.write_bytes(mutant)
    mutated_disk=out/'mutated-installed.img'
    mutation=run('mutated-copy',['--floppy',mutated_source,'--hard-disk',blank,
                                 '--disk-out',mutated_disk,'--until-post','0xac'],0xac)
    if sha(mutated_disk)==before['disk']:
        raise ValueError('whole-image identity check missed changed staged input')
    if (sha(disk)!=before['disk'] or sha(payload)!=before['payload'] or sha(exe)!=before['emulator'] or
        sha(report_path)!=before['session_report'] or sha(blank)!=blank_sha or sha(source)!=source_sha):
        raise ValueError('input changed during diagnostic installation')
    report={'status':'pass','input_sha256':before,'blank_disk_sha256':blank_sha,'installer_floppy_sha256':source_sha,
            'installed_disk_sha256':sha(installed),'mutated_disk_sha256':sha(mutated_disk),
            'stage_bytes':prefix_size,'source_sha256':{p.name:sha(p) for p in
                (Path(__file__),Path(__file__).with_name('pc286_loader.py'),Path(__file__).with_name('pc286_hard_disk_loader.py'))},
            'copy':copy,'boot':boot,'injected_write_failure':write_fault,
            'injected_verification_failure':fault,'mutated_copy':mutation,
            'limits':'Fresh whole-disk BootHush test only; no partitioning, recovery, physical installation or production claim.'}
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({'status':'pass','report':str(out/'report.json'),'installed_sha256':sha(installed)}))


if __name__=='__main__':main()
