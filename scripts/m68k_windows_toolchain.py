#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Inspect a Windows-hosted m68k ELF toolchain before experimental builds."""
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess

TOOLS=('gcc','nm','objcopy','objdump')


def sha(path):
    h=hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda:f.read(1024*1024),b''):h.update(chunk)
    return h.hexdigest()


def pe_machine(path):
    # This identifies the host executable format; it does not establish trust.
    with path.open('rb') as f:
        hdr=f.read(64)
        if len(hdr)!=64 or hdr[:2]!=b'MZ':raise ValueError('not a PE executable: '+str(path))
        offset=struct.unpack_from('<I',hdr,60)[0]
        if offset<64 or offset>16*1024*1024:raise ValueError('invalid PE offset: '+str(path))
        f.seek(offset);pe=f.read(6)
        if len(pe)!=6 or pe[:4]!=b'PE\0\0':raise ValueError('invalid PE signature: '+str(path))
        machine=struct.unpack_from('<H',pe,4)[0]
        if machine not in (0x14c,0x8664):raise ValueError('unsupported PE host: '+str(path))
        return machine


def inspect(bin_dir,prefix='m68k-unknown-elf-'):
    if not re.fullmatch(r'm68k-[A-Za-z0-9_-]+-',prefix):raise ValueError('invalid m68k tool prefix')
    root=Path(bin_dir).resolve(strict=True)
    if not root.is_dir():raise ValueError('tool bin is not a directory')
    paths={name:root/(prefix+name+'.exe') for name in TOOLS}
    for path in paths.values():
        if not path.is_file():raise ValueError('missing native tool: '+str(path))
    before={name:{'sha256':sha(path),'pe_machine':pe_machine(path)} for name,path in paths.items()}
    def query(option):
        run=subprocess.run([str(paths['gcc']),option],capture_output=True,text=True,timeout=15,
                           creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        if run.returncode:raise ValueError('compiler query failed: '+option)
        return run.stdout.strip()
    target=query('-dumpmachine');version=query('--version').splitlines()
    if not re.fullmatch(r'm68k-[A-Za-z0-9_.-]*elf',target):raise ValueError('not an m68k ELF compiler: '+target)
    if not version:raise ValueError('compiler did not identify its version')
    after={name:{'sha256':sha(path),'pe_machine':pe_machine(path)} for name,path in paths.items()}
    if before!=after:raise ValueError('tool changed while inspected')
    return paths,{'host':'windows','bin_dir':str(root),'prefix':prefix,'target':target,
                  'compiler_version':version[0],'tools':before,
                  'limits':'Identity and target checks only; build, provenance and runtime review remain open.'}


def main():
    import argparse
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--bin-dir',required=True,type=Path);p.add_argument('--prefix',default='m68k-unknown-elf-')
    p.add_argument('--output',required=True,type=Path);a=p.parse_args()
    _,report=inspect(a.bin_dir,a.prefix)
    with a.output.open('x',encoding='utf-8') as f:json.dump(report,f,indent=2);f.write('\n')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
