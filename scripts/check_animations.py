#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Two exact reviewed GIF identities;not a general GIF decoder/admission policy."""
import argparse
import hashlib
import json
import struct
import sys
from check_screenshots import Fail,read_index,read_blob,no_dups,HEX40
PATH='docs/evidence/screenshots/2026-10-21/hosted-window-drag.gif'
MANIFEST='docs/evidence/animations.json'
APPROVED='b610d8372e626e14c1840914ebc9c61e84dc5cb76c4f6c8d25d4f171f16f04b9'
NATIVE_PATH='docs/evidence/screenshots/2026-10-23/native-pc-window-drag.gif'
APPROVALS={
    PATH:(APPROVED,308010,640,480,49,4200,'2026-10-21'),
    NATIVE_PATH:('b709aae8c0a6097142b02813093123d003ddb7b39999e81a0b1cf3a6fab84455',57129,1024,768,8,4850,'2026-10-23')}
FIELDS={'path','sha256','bytes','width','height','frames','duration_ms','captured_on',
        'source_revision','compiled_source_revision','evidence','license','rights','input','caption','decode_audit'}

def verify_blob(path,data,entry):
    if not isinstance(entry,dict) or set(entry)!=FIELDS:raise Fail('animation metadata fields')
    if path not in APPROVALS or entry['path']!=path:raise Fail('unreviewed animation path')
    approved,size,width,height,frames,duration,date=APPROVALS[path]
    if entry['sha256']!=approved or hashlib.sha256(data).hexdigest()!=approved:raise Fail('unreviewed animation identity')
    if len(data)!=size or entry['bytes']!=len(data):raise Fail('animation byte count')
    if not data.startswith(b'GIF89a') or data[-1:]!=b';' or struct.unpack('<HH',data[6:10])!=(width,height):raise Fail('animation header/trailer')
    if any(type(entry[k]) is not int or entry[k]!=v for k,v in [('width',width),('height',height),('frames',frames),('duration_ms',duration)]):raise Fail('animation geometry/timing metadata')
    if entry['input']!='synthetic' or entry['license']!='GPL-3.0-or-later' or entry['captured_on']!=date:raise Fail('animation provenance')
    if any(not isinstance(entry[k],str) or not entry[k].strip() for k in FIELDS-{'bytes','width','height','frames','duration_ms'}):raise Fail('animation string metadata')
    if any(not HEX40.fullmatch(entry[k]) for k in ('source_revision','compiled_source_revision')):raise Fail('animation source pin')

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--root',required=True);a=ap.parse_args()
    try:
        index=read_index(a.root);gifs={p for p in index if p.lower().endswith('.gif')}
        if not gifs and MANIFEST not in index:print(json.dumps({'animations':[]}));return 0
        if PATH not in gifs or not gifs.issubset(APPROVALS) or MANIFEST not in index:raise Fail('only the two reviewed original GIFs are admitted')
        doc=json.loads(read_blob(a.root,index[MANIFEST],MANIFEST).decode('utf-8'),object_pairs_hook=no_dups)
        if set(doc)!={'schema_version','animations'} or type(doc['schema_version']) is not int or doc['schema_version']!=1 or not isinstance(doc['animations'],list) or len(doc['animations'])!=len(gifs):raise Fail('animation manifest schema')
        declared=[entry['path'] for entry in doc['animations']]
        if set(declared)!=gifs or len(set(declared))!=len(declared):raise Fail('animation declarations differ from index')
        for entry in doc['animations']:
            path=entry['path'];verify_blob(path,read_blob(a.root,index[path],path),entry)
            for key,extension in [('evidence','.md'),('decode_audit','.json')]:
                if not entry[key].endswith(extension) or entry[key] not in index:raise Fail('animation evidence not indexed')
        print(json.dumps({'animations':sorted(gifs)}));return 0
    except (Fail,ValueError,TypeError,KeyError,UnicodeError,OSError) as e:
        print('check_animations:FAIL:'+ascii(str(e)),file=sys.stderr);return 1
if __name__=='__main__':sys.exit(main())
