#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Read bounded staged UTF-8 blobs in three Git calls, not working-tree bodies."""
import argparse
import json
from pathlib import Path
import re
import subprocess
import sys

def read(root):
    def git(*args,**kwargs):
        return subprocess.check_output(['git','-C',str(root),*args],**kwargs)
    entries=[]
    for item in git('ls-files','--stage','-z').split(b'\0'):
        if not item:continue
        meta,name=item.split(b'\t',1);mode,oid,stage=meta.split()
        path=name.decode('utf-8')
        if mode not in (b'100644',b'100755') or stage!=b'0':raise ValueError('Nonregular/unmerged index entry')
        if not re.fullmatch('[A-Za-z0-9._/-]+',path):raise ValueError('Unexpected path')
        if not re.fullmatch(b'[0-9a-f]{40}',oid):raise ValueError('Unexpected object ID')
        if Path(path).suffix.lower() not in ('.png','.gif'):entries.append((path,oid))
    requests=b''.join(oid+b'\n' for _,oid in entries)
    facts=git('cat-file','--batch-check',input=requests).splitlines()
    if len(facts)!=len(entries):raise ValueError('Unexpected size response count')
    for (_,oid),line in zip(entries,facts):
        fields=line.split()
        if len(fields)!=3 or fields[:2]!=[oid,b'blob'] or not 0<=int(fields[2])<=1048576:
            raise ValueError('Unexpected/oversize index blob')
    raw=git('cat-file','--batch',input=requests)
    cursor=0;out=[]
    for path,oid in entries:
        end=raw.index(b'\n',cursor);header=raw[cursor:end].split()
        if len(header)!=3 or header[:2]!=[oid,b'blob']:raise ValueError('Unexpected batch response')
        size=int(header[2]);cursor=end+1
        if size<0 or size>1048576:raise ValueError('Index blob exceeds guard bound')
        body=raw[cursor:cursor+size];cursor+=size
        if len(body)!=size or raw[cursor:cursor+1]!=b'\n':raise ValueError('Truncated batch blob')
        cursor+=1;text=body.decode('utf-8')
        if '\0' in text:raise ValueError('NUL in source/text blob')
        out.append({'path':path,'size':size,'content':text})
    if cursor!=len(raw):raise ValueError('Unexpected trailing batch data')
    return out

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--root',required=True);a=p.parse_args()
    sys.stdout.reconfigure(encoding='utf-8')
    print(json.dumps(read(a.root),ensure_ascii=True,separators=(',',':')))
