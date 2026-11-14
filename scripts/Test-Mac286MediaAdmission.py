#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Reject changed 286 media identity before a paired guest run."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for option in ('emulator','rom','manifest','pc-results','passing-report','output'):
        p.add_argument('--'+option, required=True, type=Path)
    a = p.parse_args()
    exe = a.emulator.resolve(strict=True);rom = a.rom.resolve(strict=True)
    manifest = a.manifest.resolve(strict=True);pc = a.pc_results.resolve(strict=True)
    passing = a.passing_report.resolve(strict=True);out = a.output.resolve()
    if out == ROOT or ROOT in out.parents:
        raise ValueError('output must be outside the repository')
    baseline = json.loads((pc/'report.json').read_text(encoding='utf-8'))
    prior = json.loads(passing.read_text(encoding='utf-8'))
    media_sha = sha(pc/'session.img')
    if (baseline.get('media') != 'hard-disk' or baseline.get('media_sha256') != media_sha or
        prior.get('status') != 'pass' or prior.get('pc_media') != 'hard-disk' or
        prior.get('pc_media_sha256') != media_sha or prior.get('boothush_sha256') != sha(exe) or
        prior.get('mac_rom_sha256') != sha(rom) or prior.get('mac_manifest_sha256') != sha(manifest)):
        raise ValueError('passing baseline identity mismatch')
    out.mkdir(parents=True, exist_ok=False);cases = []
    for name in ('image','hash','type'):
        folder = out/name;folder.mkdir();image = folder/'session.img'
        shutil.copyfile(pc/'session.img', image)
        altered = dict(baseline)
        if name == 'image':
            with image.open('r+b') as stream:
                stream.seek(-1, 2);byte = stream.read(1);stream.seek(-1, 2);stream.write(bytes([byte[0]^1]))
        elif name == 'hash':
            altered['media_sha256'] = '0'*64
        else:
            altered['media'] = 'unknown'
        (folder/'report.json').write_text(json.dumps(altered, indent=2)+'\n', encoding='utf-8')
        dest = out/(name+'-run')
        cmd = [sys.executable,'-B',str(ROOT/'scripts/Test-BootHushMac286Parity.py'),
               '--emulator',str(exe),'--mac-rom',str(rom),'--mac-manifest',str(manifest),
               '--pc-results',str(folder),'--output',str(dest)]
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=15,
                                creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        expected = 'unsupported 286 diagnostic media' if name == 'type' else '286 report, emulator or media identity mismatch'
        if result.returncode == 0 or dest.exists() or expected not in result.stderr:
            raise ValueError(name+': wrong refusal or guest run began: '+result.stderr)
        cases.append({'name':name,'command':cmd,'exit_code':result.returncode,'stderr':result.stderr.strip()})
    if sha(pc/'session.img') != media_sha:
        raise ValueError('original media changed')
    report = {'status':'pass','media_sha256':media_sha,'passing_report_sha256':sha(passing),'cases':cases,
              'limits':'Model input admission only; not media authentication, physical proof or release qualification.'}
    (out/'report.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    print(json.dumps({'status':'pass','case_count':len(cases),'report':str(out/'report.json')}))


if __name__ == '__main__':
    main()
