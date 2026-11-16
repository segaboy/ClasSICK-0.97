# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Qualify original preview boot/install media using disposable BootHush disks."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
from pc286_loader import floppy
from pc286_hard_disk_loader import DISK_BYTES

ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('emulator','media','session','font','output'):p.add_argument('--'+n,type=Path,required=True)
    a=p.parse_args();exe=a.emulator.resolve(strict=True);media=a.media.resolve(strict=True)
    session=a.session.resolve(strict=True);font=a.font.resolve(strict=True);out=a.output.resolve()
    m=json.loads((media/'media.json').read_text());prior=json.loads((session/'report.json').read_text())
    if sha(session/'report.json')!=m['session_report_sha256'] or sha(exe)!=prior['boothush_sha256']:
        raise ValueError('session or emulator identity differs')
    for n,d in {**prior['source_sha256'],**m['source_sha256']}.items():
        if sha(ROOT/n)!=d:raise ValueError('source changed: '+n)
    for n,d in m['images'].items():
        if sha(media/n)!=d:raise ValueError('media changed: '+n)
    if sha(font)!='0d2224d4908bd4856e001b1e7aeb2409b92ebd785d5691d41db6253759ea0517':
        raise ValueError('original diagnostic font identity differs')
    if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside repository')
    out.mkdir(parents=True,exist_ok=False)
    blank=out/'blank.img';blank.write_bytes(bytes(DISK_BYTES));blank_sha=sha(blank)
    observations=[]
    def run(name,options,post,keys=None,stop_cycles=300000000):
        cmd=[str(exe),'--machine','pc286',*map(str,options),'--ega-font',str(font),
             '--cycles',str(stop_cycles),'--timeout-ms','60000','--until-post',hex(post),
             '--frame',str(out/(name+'.ppm'))]
        for i,key in enumerate(keys or []):
            t=10000000+i*100000
            for c,v in ((t,key),(t+10000,0xf0),(t+20000,key)):cmd+=['--scancode',f'{c}:{v}']
        r=subprocess.run(cmd,capture_output=True,text=True,timeout=65,
                         creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        (out/(name+'.log')).write_text(r.stdout+'\n--- stderr ---\n'+r.stderr)
        d=json.loads(r.stdout)
        if r.returncode or d['status']!='condition_met' or d['post']!=post:
            raise ValueError(name+': unexpected result '+str(d))
        for kind in ('read','write'):
            base='--fail-hard-disk-'+kind
            flag=next((f for f in (base,base+'-at',base+'-once') if f in cmd),None)
            if flag:
                expected=3 if flag==base else 1
                if d.get('hard_disk_'+kind+'_failure',{}).get('injected')!=expected:
                    raise ValueError(name+': failure injection count differs')
        observation={'name':name,'command':cmd,'result':d,'frame_sha256':sha(out/(name+'.ppm'))}
        observations.append(observation)
        # Retain progress and completed evidence even if a later control fails.
        (out/'observations.json').write_text(json.dumps(observations,indent=2)+'\n')
        print(name+': pass',flush=True)
        return d
    install=media/'install-floppy.img';hard=media/'hard-disk.img'
    keys=[0x43,0x31,0x1b,0x2c,0x1c,0x4b,0x4b,0x5a]
    run('prompt',['--floppy',install,'--hard-disk',blank],0xa0,stop_cycles=10000000)
    cancelled=out/'cancelled.img'
    run('cancel',['--floppy',install,'--hard-disk',blank,'--disk-out',cancelled],0xad,[0x76])
    if sha(cancelled)!=blank_sha:raise ValueError('cancellation wrote the disk')
    installed=out/'installed.img'
    run('install',['--floppy',install,'--hard-disk',blank,'--disk-out',installed],0xac,keys)
    if sha(installed)!=sha(hard):raise ValueError('installed whole disk differs')
    run('installed-boot',['--hard-disk',installed,'--inspect','0x60000:17'],0xb0,stop_cycles=20000000)
    if bytes.fromhex(observations[-1]['result']['memory']['bytes'])!=bytes.fromhex(prior['cases'][0]['guest_result']['memory']['bytes']):
        raise ValueError('installed boot differs from initial scene')
    # A dirty byte at either end must refuse the entire disk without writes.
    for name,offset in [('occupied-start',0),('occupied-end',DISK_BYTES-1)]:
        source=out/(name+'-source.img');data=bytearray(DISK_BYTES);data[offset]=0x5a;source.write_bytes(data)
        saved=out/(name+'-result.img')
        run(name,['--floppy',install,'--hard-disk',source,'--disk-out',saved],0xe4,keys)
        if sha(saved)!=sha(source):raise ValueError('occupied disk changed')
    # Reject damaged staged data even with a recomputed outer floppy checksum.
    stage=bytearray((media/'installer-stage.bin').read_bytes());stage[m['constants']['BLOB_OFFSET']+512]^=1
    corrupt=out/'corrupt-stage-floppy.img';corrupt.write_bytes(floppy(bytes(stage),0,verify_payload=True))
    saved=out/'corrupt-stage-result.img'
    run('staged-crc',['--floppy',corrupt,'--hard-disk',blank,'--disk-out',saved],0xe2,keys)
    if sha(saved)!=blank_sha:raise ValueError('bad staged CRC wrote disk')
    # Persistent writes fail after bounded retries; boot sector stays blank.
    for name,chs,completed in [('payload-write','0:0:2',0),('boot-commit','0:0:1',1)]:
        saved=out/(name+'.img')
        run(name,['--floppy',install,'--hard-disk',blank,'--disk-out',saved,'--fail-hard-disk-write',chs],0xe5,keys)
        data=saved.read_bytes();expected=hard.read_bytes() if completed else bytes(DISK_BYTES)
        if data[:512]!=bytes(512) or data[512:]!=expected[512:]:raise ValueError(name+': boot-last invariant failed')
    saved=out/'preflight-read.img'
    run('preflight-read',['--floppy',install,'--hard-disk',blank,'--disk-out',saved,'--fail-hard-disk-read','0:0:1'],0xe3,keys)
    if sha(saved)!=blank_sha:raise ValueError('preflight failure wrote disk')
    # Transient payload write and verification-read failures are recoverable.
    for name,flag,point in [('write-retry','--fail-hard-disk-write-at','0:0:2:1'),
                            ('verify-retry','--fail-hard-disk-read-at','0:0:2:1')]:
        saved=out/(name+'.img')
        run(name,['--floppy',install,'--hard-disk',blank,'--disk-out',saved,flag,point],0xac,keys)
        if sha(saved)!=sha(hard):raise ValueError(name+': recovered disk differs')
    # Native boot refusal/retry paths. No renderer runs after damaged payload.
    corrupt_disk=out/'corrupt-payload.img';data=bytearray(hard.read_bytes());data[512]^=1;corrupt_disk.write_bytes(data)
    run('boot-crc',['--hard-disk',corrupt_disk],0xe3,stop_cycles=20000000)
    run('boot-read',['--hard-disk',hard,'--fail-hard-disk-read','0:0:2'],0xe2,stop_cycles=20000000)
    last=m['constants']['PAYLOAD_SECTORS']
    last_chs=f'{last//68}:{last//17%4}:{last%17+1}'
    run('boot-final-read',['--hard-disk',hard,'--fail-hard-disk-read',last_chs],0xe2,stop_cycles=20000000)
    run('boot-retry',['--hard-disk',hard,'--fail-hard-disk-read-once','0:0:2'],0xb0,stop_cycles=20000000)
    # Repeat the same independently-oracled eight native session cases on the
    # new boot media. Keep a parity-compatible report without rewriting history.
    pair=out/'session';pair.mkdir();shutil.copyfile(hard,pair/'session.img')
    shutil.copyfile(session/'session.bin',pair/'session.bin')
    result=copy.deepcopy(prior);result['cases']=[]
    for case in prior['cases']:
        name=case['name'];cmd=list(case['command']);cmd[cmd.index('--hard-disk')+1]=str(pair/'session.img')
        frame=pair/(name+'.ppm');cmd[cmd.index('--frame')+1]=str(frame)
        r=subprocess.run(cmd,capture_output=True,text=True,timeout=25,
                         creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        (pair/(name+'.log')).write_text(r.stdout+r.stderr);d=json.loads(r.stdout)
        if (r.returncode or d['status']!='condition_met' or d['post']!=case['guest_result']['post'] or
            d['memory']!=case['guest_result']['memory'] or sha(frame)!=case['ega_ppm_sha256']):
            raise ValueError(name+': new-media native state/frame differs')
        row=copy.deepcopy(case);row['command']=cmd;row['guest_result']=d;result['cases'].append(row)
    result['media_sha256']=sha(pair/'session.img');result['source_sha256'].update(m['source_sha256'])
    result['source_sha256']['scripts/Test-PC286PreviewMedia.py']=sha(Path(__file__))
    result['preview_media_manifest_sha256']=sha(media/'media.json')
    result['prior_oracle_report_sha256']=sha(session/'report.json')
    result['invocation']=[sys.executable,*sys.argv]
    result['bss_poisoned_before_load']=False
    result['prior_payload_checks']={k:result.pop(k) for k in ('pixel_mutations_rejected','irq_mask_observation')}
    result['prior_payload_checks_scope']='Unchanged payload checks from the pinned prior report; not rerun by this media test.'
    result['limits']='Original preview loader, same shared native payload/oracle; no physical/human/release qualification.'
    (pair/'report.json').write_text(json.dumps(result,indent=2)+'\n')
    report={'status':'pass','observations':observations,'session':str(pair),'media_sha256':sha(media/'media.json'),
            'font_sha256':sha(font),'emulator_sha256':sha(exe),'source_sha256':result['source_sha256'],
            'limits':'disposable image tests in the built-in firmware model; no physical or publication claim'}
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({'status':'pass','media_cases':len(observations),'native_states':len(result['cases']),'report':str(out/'report.json')}))
if __name__=='__main__':main()
