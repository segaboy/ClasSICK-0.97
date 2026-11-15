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
BLOB_OFFSET = 256
SECTORS_PER_TRACK = 17


def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def stage2(prefix, *, corrupt_readback=False, require_blank_prefix=False):
    if not prefix or len(prefix) % SECTOR:
        raise ValueError('sector-aligned nonempty target required')
    count = len(prefix)//SECTOR;blob = BLOB_OFFSET;scratch = (blob+len(prefix)+15)&~15
    if count > 4*SECTORS_PER_TRACK or scratch+len(prefix) > 0xf000 or blob+len(prefix) > MAX_PAYLOAD:
        raise ValueError('diagnostic staging exceeds memory or disk limits')
    code = bytearray.fromhex('b80010 8ed8 8ec0 fb')
    branches = [];near_branches = []
    if require_blank_prefix:
        # Inspect only the sectors this diagnostic will overwrite. This is a
        # model-only guard, not a safe-media or whole-disk blankness test.
        remaining=count;head=0;offset=0
        while remaining:
            part=min(remaining,SECTORS_PER_TRACK)
            code += b'\xbb'+struct.pack('<H',scratch+offset)+bytes((0xb8,part,2))
            code += bytes.fromhex('b90100 ba')+struct.pack('<H',(head<<8)|0x80)
            code += bytes.fromhex('cd13 7303 e9');near_branches.append((len(code),5));code += b'\0\0'
            remaining-=part;head+=1;offset+=part*SECTOR
        code += bytes.fromhex('fc bf')+struct.pack('<H',scratch)
        code += b'\xb9'+struct.pack('<H',len(prefix))+bytes.fromhex('30c0 f3ae 7403 e9')
        near_branches.append((len(code),4));code += b'\0\0'
    for operation, address in ((3,blob),(2,scratch)):
        remaining = count;head = 0;offset = 0
        while remaining:
            part = min(remaining, SECTORS_PER_TRACK)
            code += b'\xbb'+struct.pack('<H',address+offset)+bytes((0xb8,part,operation))
            code += bytes.fromhex('b90100 ba')+struct.pack('<H',(head<<8)|0x80)
            code += bytes.fromhex('cd13 72');branches.append((len(code),operation));code.append(0)
            remaining-=part;head+=1;offset+=part*SECTOR
    if corrupt_readback:
        # Test-only corruption of the readback buffer after BIOS returns.
        code += bytes.fromhex('8036')+struct.pack('<H',scratch)+b'\x01'
    code += bytes.fromhex('fc be')+struct.pack('<H',blob)
    code += b'\xbf'+struct.pack('<H',scratch)+b'\xb9'+struct.pack('<H',len(prefix))
    code += bytes.fromhex('f3a6 75');branches.append((len(code),4));code.append(0)
    code += bytes.fromhex('b0ac e680 ebfe')
    failures = []
    for value in (0xe1,0xe2,0xe3)+((0xe4,0xe5) if require_blank_prefix else ()):
        failures.append(len(code));code += bytes((0xb0,value,0xe6,0x80,0xeb,0xfe))
    for offset,operation in branches:
        target=failures[{3:0,2:1,4:2}[operation]]
        delta = target-offset-1
        if not -128 <= delta <= 127:raise ValueError('branch outside diagnostic range')
        code[offset] = delta & 255
    for offset,operation in near_branches:
        delta=failures[{4:3,5:4}[operation]]-(offset+2)
        if not -32768<=delta<=32767:raise ValueError('near branch outside diagnostic range')
        code[offset:offset+2]=struct.pack('<h',delta)
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
    write_faults=[];read_faults=[]
    for head in range((prefix_size//SECTOR+SECTORS_PER_TRACK-1)//SECTORS_PER_TRACK):
        chs=f'0:{head}:1';written=head*SECTORS_PER_TRACK*SECTOR
        failed_write=out/('failed-write-%d.img'%head)
        write_fault=run('write-failure-%d'%head,['--floppy',source,'--hard-disk',blank,'--disk-out',failed_write,
                        '--until-post','0xe1','--fail-hard-disk-write',chs],0xe1)
        expected=hashlib.sha256(prefix[:written]+bytes(DISK_BYTES-written)).hexdigest()
        if write_fault['result']['hard_disk_write_failure']['injected']!=1 or sha(failed_write)!=expected:
            raise ValueError('write failure did not preserve exactly the preceding modeled writes')
        write_faults.append(write_fault)
        failed_read=out/('failed-read-after-write-%d.img'%head)
        fault=run('read-failure-%d'%head,['--floppy',source,'--hard-disk',blank,'--disk-out',failed_read,
                  '--until-post','0xe2','--fail-hard-disk-read',chs],0xe2)
        if fault['result']['hard_disk_read_failure']['injected']!=1 or sha(failed_read)!=before['disk']:
            raise ValueError('verification failure did not retain the preceding completed writes')
        read_faults.append(fault)
    corrupted_reader=out/'corrupted-readback-floppy.img'
    corrupted_reader.write_bytes(floppy(stage2(prefix,corrupt_readback=True),0))
    mismatched_disk=out/'mismatched-readback-disk.img'
    mismatch=run('readback-mismatch',['--floppy',corrupted_reader,'--hard-disk',blank,
                                      '--disk-out',mismatched_disk,'--until-post','0xe3'],0xe3)
    if sha(mismatched_disk)!=before['disk']:
        raise ValueError('readback mutation unexpectedly altered the disk')
    # Change a staged padding byte: guest readback still agrees with its input,
    # but the host's whole-image identity check must reject the resulting disk.
    mutant=bytearray(source.read_bytes());mutant[SECTOR+BLOB_OFFSET+prefix_size-1]^=1
    mutated_source=out/'mutated-installer-floppy.img';mutated_source.write_bytes(mutant)
    mutated_disk=out/'mutated-installed.img'
    mutation=run('mutated-copy',['--floppy',mutated_source,'--hard-disk',blank,
                                 '--disk-out',mutated_disk,'--until-post','0xac'],0xac)
    if sha(mutated_disk)==before['disk']:
        raise ValueError('whole-image identity check missed changed staged input')
    # The opt-in guard runs only against generated images. A nonzero byte in
    # the target prefix must be refused before the guest writes anything.
    guarded=out/'guarded-installer.img';guarded.write_bytes(floppy(stage2(prefix,require_blank_prefix=True),0))
    guarded_disk=out/'guarded-blank-copy.img'
    guarded_copy=run('guarded-copy',['--floppy',guarded,'--hard-disk',blank,'--disk-out',guarded_disk,
                                    '--until-post','0xac'],0xac)
    if sha(guarded_disk)!=before['disk']:raise ValueError('guarded blank copy differs')
    guarded_boot=run('guarded-boot',['--hard-disk',guarded_disk,'--until-post','0xb0',
                                   '--inspect','0x60000:17'],0xb0)
    if guarded_boot['result']['memory']['bytes']!=prior['cases'][0]['guest_result']['memory']['bytes']:
        raise ValueError('guarded output boot state differs')
    guard_refusals=[]
    offsets=sorted({0,prefix_size-1,*[head*SECTORS_PER_TRACK*SECTOR for head in
                    range((prefix_size//SECTOR+SECTORS_PER_TRACK-1)//SECTORS_PER_TRACK)]})
    for offset in offsets:
        disk_bytes=bytearray(DISK_BYTES);disk_bytes[offset]=0x5a
        nonblank=out/('nonblank-%d.img'%offset);nonblank.write_bytes(disk_bytes)
        snapshot=out/('refused-%d.img'%offset)
        refusal=run('guard-refusal-%d'%offset,['--floppy',guarded,'--hard-disk',nonblank,
                    '--disk-out',snapshot,'--until-post','0xe4'],0xe4)
        expected_nonblank=hashlib.sha256(disk_bytes).hexdigest()
        if sha(snapshot)!=expected_nonblank or sha(nonblank)!=expected_nonblank:
            raise ValueError('guard changed nonblank target')
        guard_refusals.append({'offset':offset,'input_sha256':sha(nonblank),'result':refusal})
    preflight_failures=[]
    guarded_write_failures=[];guarded_verify_failures=[]
    for head in range((prefix_size//SECTOR+SECTORS_PER_TRACK-1)//SECTORS_PER_TRACK):
        preflight_fault=out/('guarded-read-failure-%d.img'%head)
        preflight=run('guarded-read-failure-%d'%head,['--floppy',guarded,'--hard-disk',blank,
                      '--disk-out',preflight_fault,'--until-post','0xe5',
                      '--fail-hard-disk-read',f'0:{head}:1'],0xe5)
        if sha(preflight_fault)!=blank_sha or preflight['result']['hard_disk_read_failure']['injected']!=1:
            raise ValueError('preflight read failure changed target or did not trigger')
        preflight_failures.append(preflight)
        written=head*SECTORS_PER_TRACK*SECTOR
        write_snapshot=out/('guarded-write-failure-%d.img'%head)
        wf=run('guarded-write-failure-%d'%head,['--floppy',guarded,'--hard-disk',blank,
               '--disk-out',write_snapshot,'--until-post','0xe1',
               '--fail-hard-disk-write',f'0:{head}:1'],0xe1)
        if (wf['result']['hard_disk_write_failure']['injected']!=1 or
                sha(write_snapshot)!=hashlib.sha256(prefix[:written]+bytes(DISK_BYTES-written)).hexdigest()):
            raise ValueError('guarded write failure did not preserve prior writes')
        guarded_write_failures.append(wf)
        verify_snapshot=out/('guarded-verify-failure-%d.img'%head)
        vf=run('guarded-verify-failure-%d'%head,['--floppy',guarded,'--hard-disk',blank,
               '--disk-out',verify_snapshot,'--until-post','0xe2',
               '--fail-hard-disk-read-at',f'0:{head}:1:2'],0xe2)
        injected=vf['result']['hard_disk_read_failure']
        if (injected['injected']!=1 or injected['occurrence']!=2 or injected['matched']!=2 or
                sha(verify_snapshot)!=before['disk']):
            raise ValueError('guarded verification failure did not preserve completed writes')
        guarded_verify_failures.append(vf)
    # Negative control: omitting the opt-in guard overwrites the generated
    # nonblank prefix, proving that the refusal is due to the added guard.
    unguarded_snapshot=out/'unguarded-overwrite.img'
    unguarded=run('unguarded-overwrite',['--floppy',source,'--hard-disk',out/'nonblank-0.img',
                  '--disk-out',unguarded_snapshot,'--until-post','0xac'],0xac)
    if sha(unguarded_snapshot)!=before['disk']:raise ValueError('unguarded control did not overwrite target prefix')
    # Boundary control: this preflight intentionally cannot detect nonzero
    # data after the staged prefix, and must preserve that out-of-scope byte.
    outside_bytes=bytearray(DISK_BYTES);outside_bytes[prefix_size]=0x5a
    outside=out/'outside-prefix.img';outside.write_bytes(outside_bytes)
    outside_sha=sha(outside)
    outside_snapshot=out/'outside-prefix-after.img'
    outside_result=run('outside-prefix',['--floppy',guarded,'--hard-disk',outside,'--disk-out',outside_snapshot,
                       '--until-post','0xac'],0xac)
    outside_bytes[:prefix_size]=prefix
    if sha(outside)!=outside_sha or sha(outside_snapshot)!=hashlib.sha256(outside_bytes).hexdigest():
        raise ValueError('out-of-prefix control did not preserve tail')
    guarded_mutant=out/'guarded-mismatch-floppy.img'
    guarded_mutant.write_bytes(floppy(stage2(prefix,corrupt_readback=True,require_blank_prefix=True),0))
    guarded_mismatch_disk=out/'guarded-mismatch-disk.img'
    guarded_mismatch=run('guarded-mismatch',['--floppy',guarded_mutant,'--hard-disk',blank,
                         '--disk-out',guarded_mismatch_disk,'--until-post','0xe3'],0xe3)
    if sha(guarded_mismatch_disk)!=before['disk']:raise ValueError('guarded readback mutation changed disk')
    if (sha(disk)!=before['disk'] or sha(payload)!=before['payload'] or sha(exe)!=before['emulator'] or
        sha(report_path)!=before['session_report'] or sha(blank)!=blank_sha or sha(source)!=source_sha):
        raise ValueError('input changed during diagnostic installation')
    report={'status':'pass','input_sha256':before,'blank_disk_sha256':blank_sha,'installer_floppy_sha256':source_sha,
            'installed_disk_sha256':sha(installed),'mutated_disk_sha256':sha(mutated_disk),
            'stage_bytes':prefix_size,'source_sha256':{p.name:sha(p) for p in
                (Path(__file__),Path(__file__).with_name('pc286_loader.py'),Path(__file__).with_name('pc286_hard_disk_loader.py'))},
            'copy':copy,'boot':boot,'injected_write_failures':write_faults,
            'injected_verification_failures':read_faults,'readback_mismatch':mismatch,'mutated_copy':mutation,
            'guarded_copy':guarded_copy,'guarded_boot':guarded_boot,'guarded_installer_sha256':sha(guarded),
            'guard_refusals':guard_refusals,'guarded_read_failures':preflight_failures,
            'guarded_write_failures':guarded_write_failures,'guarded_verify_failures':guarded_verify_failures,
            'guarded_readback_mismatch':guarded_mismatch,
            'unguarded_overwrite':unguarded,'outside_prefix_control':outside_result,
            'limits':'Fresh whole-disk BootHush test only; no partitioning, recovery, physical installation or production claim.'}
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({'status':'pass','report':str(out/'report.json'),'installed_sha256':sha(installed)}))


if __name__=='__main__':main()
