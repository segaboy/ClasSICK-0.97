# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Build original fixed-profile preview media from a qualified native payload.

Writes new image files only. Does not install to a device or launch an emulator.
The physical platform and release gates remain separate.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
from pc286_loader import floppy,MAX_PAYLOAD
from pc286_hard_disk_loader import crc16,DISK_BYTES

ROOT=Path(__file__).resolve().parents[1]
BLOB=4096
SCRATCH=0x7000
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def build(session,assembler,linker,out):
    report=json.loads((session/'report.json').read_text())
    payload=(session/'session.bin').read_bytes()
    if report.get('status')!='pass' or not report.get('natural_start') or not report.get('raw_keyboard'):
        raise ValueError('requires the natural raw-keyboard shared-session result')
    if report.get('runtime_library_sha256') is not None or report.get('runtime_modules'):
        raise ValueError('external runtime modules cannot enter this package')
    if hashlib.sha256(payload).hexdigest()!=report['payload_sha256']:raise ValueError('payload changed')
    for n,d in report['source_sha256'].items():
        if sha(ROOT/n)!=d:raise ValueError('session source changed: '+n)
    mapping=(session/'session.map').read_text()
    entry=int(re.search(r'Entry point address:\s*([0-9a-fA-F]+)',mapping)[1],16)
    bss_end=report['bss_start']+report['bss_bytes']
    bss=re.search(r'^_BSS\s+BSS\s+DGROUP\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)',mapping,re.M)
    if not bss or [int(v,16) for v in bss.groups()]!=[report['bss_start'],report['bss_bytes']]:
        raise ValueError('BSS map/report disagreement')
    result=build_install_media(payload,entry,bss_end,assembler,linker,out)
    result['session_report_sha256']=sha(session/'report.json')
    (out/'media.json').write_text(json.dumps(result,indent=2)+'\n')
    return result

def build_install_media(payload,entry,bss_end,assembler,linker,out):
    """Package one near-model native payload with the SPEC-0110 installer."""
    if sha(assembler)!='9e779dda024c6cee24943f917b0f761ce862951caec0a4c0fbadbc760895c0cb':
        raise ValueError('assembler identity mismatch')
    lock=json.loads((ROOT/'scripts/pc286-toolchain-lock.json').read_text())
    expected_linker=next(r for r in lock['selected_members'] if r['path']=='binnt64/wlink.exe')
    if sha(linker)!=expected_linker['sha256'] or linker.stat().st_size!=expected_linker['bytes']:
        raise ValueError('linker identity mismatch')
    if not 0<=entry<len(payload)<=59*512 or not len(payload)<=bss_end<0xf000:
        raise ValueError('payload or BSS outside the near-segment budget')
    sectors=(len(payload)+511)//512
    constants={'PAYLOAD_BYTES':len(payload),'PAYLOAD_SECTORS':sectors,
               'PAYLOAD_ENTRY':entry,'PAYLOAD_BSS_END':bss_end,'PAYLOAD_CRC':crc16(payload),
               'BLOB_OFFSET':BLOB,'SCRATCH_OFFSET':SCRATCH,
               'PREFIX_BYTES':(sectors+1)*512,'PREFIX_SECTORS':sectors+1,'PREFIX_CRC':0}
    if BLOB+constants['PREFIX_BYTES']>min(MAX_PAYLOAD,SCRATCH):
        raise ValueError('installer staging would overlap scratch or exceed floppy loader')
    out.mkdir(parents=True,exist_ok=False)
    commands=[]
    def assemble(name):
        (out/'preview-values.inc').write_text(''.join(f'{k} equ {v}\n' for k,v in constants.items()))
        source=ROOT/'platform/pc286'/('preview-'+name+'.asm')
        obj=out/(name+'.obj');binary=out/(name+'.bin');mapfile=out/(name+'.map')
        for cmd in ([str(assembler),'-zq','-2','-i='+str(out),'-fo='+str(obj),str(source)],
                    [str(linker),'format','raw','bin','option','start=start_',
                     'option','nodefaultlibs','option','map='+str(mapfile),'name',str(binary),'file',str(obj)]):
            commands.append(cmd)
            r=subprocess.run(cmd,capture_output=True,text=True,timeout=30,
                             creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            with (out/(name+'.log')).open('a') as f:f.write(r.stdout+r.stderr+'\n')
            if r.returncode:raise RuntimeError(r.stdout+r.stderr)
        return binary.read_bytes()
    boot=assemble('boot')
    if len(boot)>510:raise ValueError(f'boot sector exceeds 510 bytes: {len(boot)}')
    prefix=boot+bytes(510-len(boot))+b'\x55\xaa'+payload+bytes(sectors*512-len(payload))
    constants['PREFIX_CRC']=crc16(prefix)
    installer=assemble('install')
    if len(installer)>BLOB:raise ValueError('installer exceeds reserved code area')
    stage=installer+bytes(BLOB-len(installer))+prefix
    (out/'hard-disk.img').write_bytes(prefix+bytes(DISK_BYTES-len(prefix)))
    (out/'installer-stage.bin').write_bytes(stage)
    (out/'install-floppy.img').write_bytes(floppy(stage,0,verify_payload=True))
    sources=('platform/pc286/preview-boot.asm','platform/pc286/preview-install.asm',
             'scripts/Build-PC286PreviewMedia.py','scripts/pc286_loader.py','scripts/pc286_hard_disk_loader.py',
             'scripts/pc286-toolchain-lock.json')
    result={'status':'built-unqualified',
            'payload_sha256':hashlib.sha256(payload).hexdigest(),'constants':constants,'boot_bytes':len(boot),
            'installer_bytes':len(installer),'assembler_sha256':sha(assembler),'linker_sha256':sha(linker),
            'commands':commands,'source_sha256':{n:sha(ROOT/n) for n in sources},
            'images':{n:sha(out/n) for n in ('hard-disk.img','install-floppy.img','installer-stage.bin')},
            'limits':'fixed BootHush BIOS profile; physical/human/release qualification pending'}
    return result
def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('session','assembler','linker','output'):p.add_argument('--'+n,type=Path,required=True)
    a=p.parse_args();out=a.output.resolve()
    if out==ROOT or ROOT in out.parents:raise ValueError('output must be outside repository')
    r=build(a.session.resolve(strict=True),a.assembler.resolve(strict=True),a.linker.resolve(strict=True),out)
    print(json.dumps({'status':r['status'],'boot_bytes':r['boot_bytes'],'installer_bytes':r['installer_bytes'],'output':str(out)}))
if __name__=='__main__':main()
