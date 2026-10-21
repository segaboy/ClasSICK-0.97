#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""One reviewed GIF identity exception;not a general GIF decoder/admission policy."""
import argparse
import hashlib
import json
import struct
import sys
from check_screenshots import Fail,read_index,read_blob,no_dups,HEX40
PATH='docs/evidence/screenshots/2026-10-21/hosted-window-drag.gif'
MANIFEST='docs/evidence/animations.json'
APPROVED='b610d8372e626e14c1840914ebc9c61e84dc5cb76c4f6c8d25d4f171f16f04b9'
FIELDS={'path','sha256','bytes','width','height','frames','duration_ms','captured_on',
        'source_revision','compiled_source_revision','evidence','license','rights','input','caption','decode_audit'}

def verify_blob(path,data,entry):
    if not isinstance(entry,dict) or set(entry)!=FIELDS:raise Fail('animation metadata fields')
    if path!=PATH or entry['path']!=PATH:raise Fail('unreviewed animation path')
    if entry['sha256']!=APPROVED or hashlib.sha256(data).hexdigest()!=APPROVED:raise Fail('unreviewed animation identity')
    if len(data)!=308010 or entry['bytes']!=len(data):raise Fail('animation byte count')
    if not data.startswith(b'GIF89a') or data[-1:]!=b';' or struct.unpack('<HH',data[6:10])!=(640,480):raise Fail('animation header/trailer')
    if any(type(entry[k]) is not int or entry[k]!=v for k,v in [('width',640),('height',480),('frames',49),('duration_ms',4200)]):raise Fail('animation geometry/timing metadata')
    if entry['input']!='synthetic' or entry['license']!='GPL-3.0-or-later' or entry['captured_on']!='2026-10-21':raise Fail('animation provenance')
    if any(not isinstance(entry[k],str) or not entry[k].strip() for k in FIELDS-{'bytes','width','height','frames','duration_ms'}):raise Fail('animation string metadata')
    if any(not HEX40.fullmatch(entry[k]) for k in ('source_revision','compiled_source_revision')):raise Fail('animation source pin')

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--root',required=True);a=ap.parse_args()
    try:
        index=read_index(a.root);gifs={p for p in index if p.lower().endswith('.gif')}
        if not gifs and MANIFEST not in index:print(json.dumps({'animations':[]}));return 0
        if gifs!={PATH} or MANIFEST not in index:raise Fail('only the reviewed original GIF is admitted')
        doc=json.loads(read_blob(a.root,index[MANIFEST],MANIFEST).decode('utf-8'),object_pairs_hook=no_dups)
        if set(doc)!={'schema_version','animations'} or type(doc['schema_version']) is not int or doc['schema_version']!=1 or not isinstance(doc['animations'],list) or len(doc['animations'])!=1:raise Fail('animation manifest schema')
        entry=doc['animations'][0];verify_blob(PATH,read_blob(a.root,index[PATH],PATH),entry)
        for key,extension in [('evidence','.md'),('decode_audit','.json')]:
            if not entry[key].endswith(extension) or entry[key] not in index:raise Fail('animation evidence not indexed')
        print(json.dumps({'animations':[PATH]}));return 0
    except (Fail,ValueError,TypeError,KeyError,UnicodeError,OSError) as e:
        print('check_animations:FAIL:'+ascii(str(e)),file=sys.stderr);return 1
if __name__=='__main__':sys.exit(main())
