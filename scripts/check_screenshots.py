#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
'''Read-only validator for indexed screenshot evidence; the Git index is the sole authority.'''
import argparse
import datetime
import hashlib
import json
import os
import re
import struct
import subprocess
import sys
import zlib

MANIFEST = 'docs/evidence/screenshots/manifest.json'
MAX_BLOB = 1048576
PATH_RE = re.compile(r'docs/evidence/screenshots/([0-9]{4}-[0-9]{2}-[0-9]{2})/[a-z0-9]+(?:-[a-z0-9]+)*[.]png')
ID_RE = re.compile(r'[a-z0-9]+(?:-[a-z0-9]+)*')
DATE_RE = re.compile(r'[0-9]{4}-[0-9]{2}-[0-9]{2}')
EVID_RE = re.compile(r'[A-Za-z0-9_.-]+(?:/[A-Za-z0-9_.-]+)*[.]md')
HEX40 = re.compile(r'[0-9a-f]{40}')
HEX64 = re.compile(r'[0-9a-f]{64}')
STRINGS = ('id', 'path', 'sha256', 'captured_on', 'target', 'caption', 'alt', 'source_revision',
           'compiled_source_revision', 'evidence', 'license', 'rights', 'input')
FIELDS = frozenset(STRINGS + ('bytes', 'width', 'height', 'limitations'))
TARGETS = ('native-mac68000', 'native-x86_64')
INPUTS = ('synthetic', 'human-confirmed', 'none')
REGULAR = ('100644', '100755')
SIG = bytes([137, 80, 78, 71, 13, 10, 26, 10])
PAIRS = {0: (1, 2, 4, 8, 16), 2: (8, 16), 3: (1, 2, 4, 8), 4: (8, 16), 6: (8, 16)}
BEFORE_PLTE = {b'cHRM', b'gAMA', b'iCCP', b'sBIT', b'sRGB'}
AFTER_PLTE = {b'bKGD', b'hIST', b'tRNS'}
BEFORE_IDAT = BEFORE_PLTE | AFTER_PLTE | {b'pHYs', b'sPLT'}
SINGLE = BEFORE_PLTE | AFTER_PLTE | {b'pHYs', b'tIME'}


class Fail(Exception):
    pass


def git(root, *args):
    env = dict(os.environ, GIT_OPTIONAL_LOCKS='0')
    proc = subprocess.run(['git', '-C', root] + list(args), stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE, env=env)
    if proc.returncode:
        raise Fail('git failed: ' + ' '.join(args))
    return proc.stdout


def read_index(root):
    entries = {}
    for rec in git(root, 'ls-files', '--stage', '--full-name', '-z').split(chr(0).encode()):
        if not rec:
            continue
        meta, _, raw = rec.partition(chr(9).encode())
        mode, sha, stage = meta.decode('ascii').split(' ')
        name = raw.decode('utf-8', 'surrogateescape')
        if stage != '0':
            raise Fail('unmerged index entry ' + ascii(name))
        entries[name] = (mode, sha)
    return entries


def read_blob(root, entry, name):
    mode, sha = entry
    if mode not in REGULAR:
        raise Fail('not a regular indexed file ' + ascii(name))
    size = int(git(root, 'cat-file', '-s', sha).strip())
    if size > MAX_BLOB:
        raise Fail('indexed blob too large ' + ascii(name))
    data = git(root, 'cat-file', 'blob', sha)
    if len(data) != size:
        raise Fail('blob size mismatch ' + ascii(name))
    return data


def no_dups(pairs):
    obj = {}
    for key, value in pairs:
        if key in obj:
            raise Fail('duplicate JSON key ' + ascii(key))
        obj[key] = value
    return obj


def bad_constant(name):
    raise Fail('non-finite JSON number ' + name)


def is_int(value):
    return isinstance(value, int) and not isinstance(value, bool)


def check_png(data):
    if data[:8] != SIG:
        raise Fail('bad PNG signature')
    pos, ihdr, color, seen = 8, None, None, {}
    plte = idat = idat_done = False
    while True:
        if pos + 12 > len(data):
            raise Fail('truncated chunk or missing IEND')
        length, kind = struct.unpack('>I4s', data[pos:pos + 8])
        if length > 0x7fffffff or pos + 12 + length > len(data):
            raise Fail('chunk length out of bounds')
        if not all(65 <= c <= 90 or 97 <= c <= 122 for c in kind) or kind[2] & 32:
            raise Fail('invalid chunk type')
        body = data[pos + 8:pos + 8 + length]
        (crc,) = struct.unpack('>I', data[pos + 8 + length:pos + 12 + length])
        if zlib.crc32(kind + body) & 0xffffffff != crc:
            raise Fail('chunk CRC mismatch')
        pos += 12 + length
        if ihdr is None:
            if kind != b'IHDR' or length != 13:
                raise Fail('first chunk must be 13-byte IHDR')
            width, height, depth, color, comp, filt, inter = struct.unpack('>IIBBBBB', body)
            if not (0 < width <= 0x7fffffff and 0 < height <= 0x7fffffff):
                raise Fail('invalid IHDR dimensions')
            if depth not in PAIRS.get(color, ()) or comp or filt or inter not in (0, 1):
                raise Fail('invalid IHDR format fields')
            ihdr = (width, height)
            continue
        if idat and kind != b'IDAT':
            idat_done = True
        if kind == b'IHDR':
            raise Fail('duplicate IHDR')
        elif kind == b'IDAT':
            if idat_done or (color == 3 and not plte):
                raise Fail('misplaced IDAT')
            idat = True
        elif kind == b'PLTE':
            if plte or idat or color in (0, 4) or length % 3 or not 0 < length <= 768:
                raise Fail('invalid PLTE')
            if color == 3 and length // 3 > (1 << depth):
                raise Fail('palette exceeds indexed bit depth')
            if any(k in seen for k in AFTER_PLTE):
                raise Fail('chunk precedes PLTE illegally')
            plte = True
        elif kind == b'IEND':
            if length or not idat or pos != len(data):
                raise Fail('invalid IEND or trailing bytes')
            return ihdr
        elif kind[0] < 97:
            raise Fail('unknown critical chunk')
        else:
            if kind in SINGLE and kind in seen:
                raise Fail('duplicate ancillary chunk')
            if (kind in BEFORE_IDAT and idat) or (kind in BEFORE_PLTE and plte):
                raise Fail('ancillary chunk out of order')
            if kind in AFTER_PLTE and (kind == b'hIST' or color == 3) and not plte:
                raise Fail('ancillary chunk requires PLTE')
        seen[kind] = seen.get(kind, 0) + 1


def check_entry(entry, index, root):
    if not isinstance(entry, dict) or set(entry) != FIELDS:
        raise Fail('entry fields must match the contract exactly')
    if not all(isinstance(entry[k], str) for k in STRINGS):
        raise Fail('string field has wrong type')
    if not ID_RE.fullmatch(entry['id']):
        raise Fail('invalid id')
    match = PATH_RE.fullmatch(entry['path'])
    if not match or not DATE_RE.fullmatch(entry['captured_on']) or entry['captured_on'] != match.group(1):
        raise Fail('invalid path or captured_on')
    datetime.date(*(int(part) for part in entry['captured_on'].split('-')))
    if entry['target'] not in TARGETS or entry['input'] not in INPUTS:
        raise Fail('invalid target or input')
    if entry['license'] != 'GPL-3.0-or-later':
        raise Fail('invalid license')
    if not all(entry[k].strip() for k in ('caption', 'alt', 'rights')):
        raise Fail('empty caption, alt or rights')
    lims = entry['limitations']
    if not isinstance(lims, list) or not all(isinstance(x, str) and x.strip() for x in lims):
        raise Fail('invalid limitations')
    if not (HEX40.fullmatch(entry['source_revision']) and HEX40.fullmatch(entry['compiled_source_revision'])):
        raise Fail('invalid revision')
    if not HEX64.fullmatch(entry['sha256']):
        raise Fail('invalid sha256')
    size, width, height = entry['bytes'], entry['width'], entry['height']
    if not (is_int(size) and 1 <= size <= MAX_BLOB):
        raise Fail('invalid bytes')
    if not (is_int(width) and is_int(height) and 1 <= width <= 4096 and 1 <= height <= 4096
            and width * height <= 16777216):
        raise Fail('invalid dimensions')
    evidence = entry['evidence']
    if (not EVID_RE.fullmatch(evidence) or any(seg in ('.', '..') for seg in evidence.split('/'))
            or evidence not in index or index[evidence][0] not in REGULAR):
        raise Fail('evidence must be an indexed repo-relative .md file')
    if entry['path'] not in index:
        raise Fail('registered screenshot not indexed')
    data = read_blob(root, index[entry['path']], entry['path'])
    if len(data) != size or hashlib.sha256(data).hexdigest() != entry['sha256']:
        raise Fail('indexed bytes or hash mismatch')
    if check_png(data) != (width, height):
        raise Fail('indexed PNG dimensions mismatch')


def main():
    parser = argparse.ArgumentParser(description='Validate indexed screenshot evidence (read-only).')
    parser.add_argument('--root', required=True)
    root = parser.parse_args().root
    try:
        index = read_index(root)
        pngs = sorted(p for p in index if p.lower().endswith('.png'))
        if MANIFEST not in index:
            if pngs:
                raise Fail('indexed PNGs require an indexed manifest')
            print(json.dumps({'screenshots': []}))
            return 0
        doc = json.loads(read_blob(root, index[MANIFEST], MANIFEST).decode('utf-8'),
                         object_pairs_hook=no_dups, parse_constant=bad_constant)
        if not isinstance(doc, dict) or set(doc) != {'schema_version', 'screenshots'}:
            raise Fail('manifest must contain exactly schema_version and screenshots')
        if not is_int(doc['schema_version']) or doc['schema_version'] != 1:
            raise Fail('schema_version must be integer 1')
        if not isinstance(doc['screenshots'], list):
            raise Fail('screenshots must be an array')
        ids, paths = set(), set()
        for entry in doc['screenshots']:
            check_entry(entry, index, root)
            if entry['id'] in ids or entry['path'] in paths:
                raise Fail('duplicate id or path')
            ids.add(entry['id'])
            paths.add(entry['path'])
        if paths != set(pngs):
            raise Fail('indexed PNGs and manifest registrations differ')
        print(json.dumps({'screenshots': pngs}))
        return 0
    except (Fail, ValueError, UnicodeError, OSError) as exc:
        print('check_screenshots: FAIL: ' + ascii(str(exc)), file=sys.stderr)
        return 1


if __name__ == '__main__':
    sys.exit(main())
