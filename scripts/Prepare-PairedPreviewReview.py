# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Prepare a deterministic source-matched Mac/286 package for release review.

The inputs JSON names retained build/test directories; no execution or publication.
"""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import zipfile
from importlib.util import spec_from_file_location,module_from_spec
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text())
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,timeout=30)
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--inputs',type=Path,required=True);p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();cfg=read(a.inputs);paths={k:Path(v).resolve() for k,v in cfg['paths'].items()};out=a.output.resolve()
    if out==ROOT or ROOT in out.parents:raise ValueError('output must be external')
    candidate=read(ROOT/'releases/paired-native-preview-2-mac286.json')
    receipt=Path(candidate['receipt'])
    if receipt.is_absolute() or '..' in receipt.parts or receipt.parts[:2]!=('docs','development'):
        raise ValueError('qualification receipt must be a repository development record')
    mac=read(paths['mac-build']/'manifest.json');q=read(paths['qualification']/'report.json')
    media=read(paths['pc-media']/'media.json');pcdir=Path(q['session']);pc=read(pcdir/'report.json')
    fast=read(paths['parity-fast']/'report.json');slow=read(paths['parity-slow']/'report.json')
    edges=read(paths['edges']/'report.json');captures=read(paths['captures']/'capture.json')
    inputs={'mac/classick.rom':paths['mac-build']/'Os-a/classick-loaded-app.rom',
            'pc286/hard-disk.img':paths['pc-media']/'hard-disk.img',
            'pc286/install-floppy.img':paths['pc-media']/'install-floppy.img'}
    for name,source in inputs.items():
        expected=candidate['images'][name]
        if source.stat().st_size!=expected['bytes'] or sha(source)!=expected['sha256']:raise ValueError('image changed: '+name)
    romsha=sha(inputs['mac/classick.rom']);disksha=sha(inputs['pc286/hard-disk.img'])
    if (not mac['twins_equal'] or mac['profiles'][0]['image_sha256']!=romsha or
        q['status']!='pass' or len(q['observations'])!=16 or q['media_sha256']!=sha(paths['pc-media']/'media.json') or
        pc['media_sha256']!=disksha or pc['runtime_modules'] or pc['runtime_library_sha256'] is not None):
        raise ValueError('build or media qualification mismatch')
    for report,rate in ((fast,2048),(slow,4096)):
        if (report['status']!='pass' or report['mac_rom_sha256']!=romsha or report['pc_media_sha256']!=disksha or
            report['boothush_sha256']!=candidate['tested_emulator_sha256'] or
            report['requested_edge_cycles']!=rate or not report['mac_replay_cycle_memory_frames_match'] or
            len(report['checkpoints'])!=8):raise ValueError('paired qualification mismatch')
        for row in report['checkpoints']:
            state=row['mac_state']
            if (state['fault'] or state['omissions'] or any(state['losses']) or
                row['normalized_canvas_pixels_match']!=175104 or row['pc_white_margin_pixels_match']!=48896):
                raise ValueError('failed paired checkpoint')
    if (edges['status']!='pass' or edges['mac_rom_sha256']!=romsha or edges['pc_disk_sha256']!=disksha or
        len(edges['checkpoints'])!=14 or captures['status']!='pass' or captures['rom_sha256']!=romsha):
        raise ValueError('edge or capture qualification mismatch')
    # Archive committed source only; source inputs must match that commit.
    head=git('rev-parse','HEAD').decode().strip();source_lf={}
    def admitted(name,digest,normalized=False):
        path=ROOT/name;raw=path.read_bytes();body=raw.replace(b'\r\n',b'\n')
        if hashlib.sha256(body if normalized else raw).hexdigest()!=digest:raise ValueError('changed input: '+name)
        if git('show',head+':'+name).replace(b'\r\n',b'\n')!=body:raise ValueError('uncommitted input: '+name)
        source_lf[name]=hashlib.sha256(body).hexdigest()
    for name,digest in mac['inputs'].items():admitted(name,digest,True)
    for name,digest in {**pc['source_sha256'],**edges['source_sha256']}.items():admitted(name,digest)
    for name in ('scripts/Prepare-PairedPreviewReview.py','scripts/Verify-PairedPreviewReview.py','scripts/Capture-PairedPreview.py',
                 'releases/paired-native-preview-2-mac286.json','releases/TRYING-paired-mac286.md',receipt.as_posix()):
        admitted(name,sha(ROOT/name))
    out.mkdir(parents=True,exist_ok=False)
    def copy(source,name):
        target=out/name;target.parent.mkdir(parents=True,exist_ok=True)
        with source.open('rb') as r,target.open('xb') as w:shutil.copyfileobj(r,w)
        if sha(target)!=sha(source):raise ValueError('copy changed: '+name)
    for name,source in inputs.items():copy(source,name)
    for source,name in ((ROOT/'LICENSE','COPYING'),(ROOT/'COPYRIGHT.md','COPYRIGHT.md'),
        (ROOT/'releases/TRYING-paired-mac286.md','TRYING.md'),
        (ROOT/'releases/TRYING-paired-mac286.md','TRYING.txt'),
        (ROOT/'releases/paired-native-preview-2-mac286.json','candidate.json'),
        (ROOT/receipt,'QUALIFICATION.md'),
        (ROOT/'scripts/Verify-PairedPreviewReview.py','verify-package.py')):copy(source,name)
    reports={'mac-build.json':paths['mac-build']/'manifest.json','pc286-media.json':paths['pc-media']/'media.json',
             'pc286-qualification.json':paths['qualification']/'report.json','pc286-session.json':pcdir/'report.json',
             'parity-2048.json':paths['parity-fast']/'report.json','parity-4096.json':paths['parity-slow']/'report.json',
             'edges.json':paths['edges']/'report.json'}
    reports.update({name:Path(value) for name,value in cfg.get('extra_reports',{}).items()})
    for name,source in reports.items():copy(source,'evidence/'+name)
    for label in ('captures','port-media'):
        folder=paths[label];record=read(folder/'capture.json');prefix='media' if label=='captures' else 'port-media'
        for name,digest in record['files'].items():
            if sha(folder/name)!=digest:raise ValueError('capture changed: '+name)
            copy(folder/name,prefix+'/'+name)
        copy(folder/'capture.json',prefix+'/capture.json')
    font=paths['font']/'diagnostic-8x14.bin';fontmeta=read(paths['font']/'font.json')
    fontsource=paths['boothush']/'scripts/make_diagnostic_font.py'
    if sha(font)!=q['font_sha256'] or sha(fontsource)!=fontmeta['source_sha256']:raise ValueError('font changed')
    for source,name in ((font,'font/diagnostic-8x14.bin'),(fontsource,'font/make_diagnostic_font.py'),
                        (paths['font']/'font.json','font/font.json')):copy(source,name)
    git('archive','--format=zip','--output='+str(out/'source.zip'),head)
    review={'classification':'release review package; approval pending','source_revision':head,
            'compiled_source_lf_sha256':dict(sorted(source_lf.items())),
            'build_source_note':'Pinned compiled inputs match committed source modulo CRLF/LF. Unrelated working-tree edits excluded.',
            'tested_emulator_sha256':candidate['tested_emulator_sha256'],
            'ready_for_release':False,'published':False,'acceptance':candidate['acceptance'],
            'preparation_inputs_sha256':sha(a.inputs),'source_archive_sha256':sha(out/'source.zip'),
            'notes':'Read candidate.json and TRYING.md. No compiler/emulator binaries, reference firmware or private HQ content bundled.'}
    (out/'review.json').write_text(json.dumps(review,indent=2)+'\n')
    (out/'REVIEW-PENDING.txt').write_text('RELEASE REVIEW PACKAGE - OWNER DISPOSITION PENDING\n\n'
        'The exact Mac/286 images pass the included scoped BootHush checks.\n'
        'This is not publication approval or physical/human qualification.\n'
        'See candidate.json acceptance and owner_disposition_required.\n'
        'Proposed versions remain unpublished. System and the game port remain incomplete.\n')
    files=sorted(p for p in out.rglob('*') if p.is_file())
    (out/'SHA256SUMS').write_text(''.join(sha(p)+'  '+p.relative_to(out).as_posix()+'\n' for p in files),encoding='ascii')
    archive=out.with_suffix('.zip')
    with zipfile.ZipFile(archive,'x',compression=zipfile.ZIP_DEFLATED,compresslevel=9) as z:
        for file in sorted(p for p in out.rglob('*') if p.is_file()):
            info=zipfile.ZipInfo(file.relative_to(out).as_posix(),(2026,11,16,0,0,0));info.compress_type=zipfile.ZIP_DEFLATED
            info.external_attr=0o100644<<16;z.writestr(info,file.read_bytes(),compresslevel=9)
    spec=spec_from_file_location('verify',ROOT/'scripts/Verify-PairedPreviewReview.py');v=module_from_spec(spec);spec.loader.exec_module(v)
    result=v.verify(archive);v.verify(out)
    result.update({'archive':str(archive),'archive_sha256':sha(archive),'bytes':archive.stat().st_size})
    print(json.dumps(result,indent=2))
if __name__=='__main__':main()
