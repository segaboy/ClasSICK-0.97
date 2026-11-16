# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Verify review-package membership and hashes; not a signature or release approval."""
import argparse
import hashlib
import json
from pathlib import Path,PurePosixPath
import zipfile
def verify(path):
    archive=zipfile.ZipFile(path) if path.is_file() else None
    if archive:
        names=archive.namelist()
        if len(set(names))!=len(names):raise ValueError('duplicate ZIP entry')
        read=archive.read
    else:
        names=[p.relative_to(path).as_posix() for p in path.rglob('*') if p.is_file()]
        read=lambda n:(path/n).read_bytes()
    for name in names:
        p=PurePosixPath(name)
        if p.is_absolute() or '..' in p.parts or '\\' in name or ':' in name:raise ValueError('unsafe member')
    rows={}
    for line in read('SHA256SUMS').decode('ascii').splitlines():
        digest,name=line.split('  ',1)
        if name in rows or len(digest)!=64:raise ValueError('invalid inventory')
        rows[name]=digest
    if set(names)!=set(rows)|{'SHA256SUMS'}:raise ValueError('package membership differs')
    for name,digest in rows.items():
        if hashlib.sha256(read(name)).hexdigest()!=digest:raise ValueError('changed file: '+name)
    review=json.loads(read('review.json'));candidate=json.loads(read('candidate.json'))
    if review['classification']!='release review package; approval pending' or candidate['ready_for_release'] or candidate['published']:
        raise ValueError('unexpected release status')
    for name,item in candidate['images'].items():
        raw=read(name)
        if len(raw)!=item['bytes'] or hashlib.sha256(raw).hexdigest()!=item['sha256']:raise ValueError('candidate mismatch: '+name)
    with zipfile.ZipFile(__import__('io').BytesIO(read('source.zip'))) as source:
        if source.testzip() is not None:raise ValueError('source archive CRC error')
        for name,digest in review['compiled_source_lf_sha256'].items():
            if hashlib.sha256(source.read(name).replace(b'\r\n',b'\n')).hexdigest()!=digest:raise ValueError('source mismatch: '+name)
        if source.read('LICENSE')!=read('COPYING'):raise ValueError('source/license mismatch')
    if archive:archive.close()
    return {'status':'pass','files':len(names),'compiled_inputs':len(review['compiled_source_lf_sha256']),
            'source_revision':review['source_revision'],'ready_for_release':False}
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--package',type=Path,required=True)
    print(json.dumps(verify(p.parse_args().package),indent=2))
