#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Compare an extracted Windows toolchain tree with a pinned ZIP archive."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import stat
import zipfile

EXPECTED='18d36e1233b54fb2346e134ac849edc436db4e3f1cd50fdea5435aa02c1e5ba2'


def digest_stream(f):
    h=hashlib.sha256()
    for part in iter(lambda:f.read(1024*1024),b''):h.update(part)
    return h.hexdigest()


def digest_file(p):
    with p.open('rb') as f:return digest_stream(f)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--archive',required=True,type=Path);p.add_argument('--root',required=True,type=Path)
    p.add_argument('--output',required=True,type=Path);a=p.parse_args()
    archive=a.archive.resolve(strict=True);root=a.root.resolve(strict=True);out=a.output.resolve()
    if root==out or root in out.parents:raise ValueError('output must be outside inspected tree')
    if digest_file(archive)!=EXPECTED:raise ValueError('archive identity mismatch')
    entries=[];expected=set()
    with zipfile.ZipFile(archive) as z:
        for info in z.infolist():
            name=info.filename.replace('\\','/');parts=PurePosixPath(name).parts
            if name.startswith('/') or ':' in name or '..' in parts or stat.S_ISLNK(info.external_attr>>16):
                raise ValueError('unsafe archive entry: '+name)
            if info.is_dir():continue
            key=name.lower()
            if key in expected:raise ValueError('duplicate archive member: '+name)
            expected.add(key);item=root.joinpath(*parts)
            if not item.is_file() or item.is_symlink() or root not in item.resolve(strict=True).parents:
                raise ValueError('missing or unsafe extracted file: '+name)
            with z.open(info) as f:archive_hash=digest_stream(f)
            if item.stat().st_size!=info.file_size or digest_file(item)!=archive_hash:
                raise ValueError('extracted file differs from archive: '+name)
            entries.append({'path':name,'bytes':info.file_size,'sha256':archive_hash})
    actual={p.relative_to(root).as_posix().lower() for p in root.rglob('*') if p.is_file()}
    if actual!=expected:raise ValueError('extracted tree has missing or additional files')
    report={'status':'pass','archive_sha256':EXPECTED,'archive_bytes':archive.stat().st_size,
            'files':entries,'total_files':len(entries),'total_bytes':sum(x['bytes'] for x in entries),
            'limits':'File identity comparison, not compiler correctness, license review or distribution approval.'}
    with out.open('x',encoding='utf-8') as f:json.dump(report,f,indent=2);f.write('\n')
    print(json.dumps({k:v for k,v in report.items() if k!='files'},indent=2))


if __name__=='__main__':main()
