# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Unpack pinned compiler tools into a workspace directory; no system installation."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import urllib.request

def main():
    p=argparse.ArgumentParser(); p.add_argument('--output',required=True); a=p.parse_args()
    out=Path(a.output).resolve(); debs=out/'debs'; root=out/'root'
    debs.mkdir(parents=True,exist_ok=True); root.mkdir(exist_ok=True)
    lock=json.loads((Path(__file__).parent/'m68k-toolchain-lock.json').read_text())
    for package in lock['packages']:
        target=debs/package['archive']
        if not target.exists():
            with urllib.request.urlopen(package['url'],timeout=60) as response:
                target.write_bytes(response.read())
        b=target.read_bytes()
        if len(b)!=package['size_bytes'] or hashlib.sha256(b).hexdigest()!=package['sha256']:
            raise ValueError('Compiler package identity mismatch: '+package['package'])
        subprocess.run(['dpkg-deb','-x',str(target),str(root)],check=True)
    print('Pinned workspace compiler packages verified:',len(lock['packages']))
if __name__=='__main__': main()
