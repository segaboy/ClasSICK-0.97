#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell
'''Build approval-pending paired ClasSICK release candidates from admitted inputs.

Python standard library only. Never runs, extracts or publishes anything.
'''
import argparse
import hashlib
import io
import json
import os
import re
import stat
import sys
import zipfile

REPO = 'https://github.com/segaboy/ClasSICK-0.97'
INPUT_NAMES = {'rom.rom', 'BOOTX64.EFI', 'pc.img', 'source.zip', 'COPYING'}
BINARY_INPUTS = INPUT_NAMES - {'source.zip', 'COPYING'}
TAG_PREFIX = {'mac': 'rom-v', 'pc': 'pc-v'}
TOP_KEYS = {'schema_version', 'compiled_source_revision', 'evidence_revision', 'ci_url',
            'license', 'inputs', 'components', 'unqualified'}
COMPONENT_KEYS = {'name', 'version', 'tag', 'binary_input', 'standalone', 'archive',
                  'source_archive', 'payloads', 'documents'}
DOC_NAMES = {'README.txt', 'RELEASE.txt'}
SUMS = 'SHA256SUMS.txt'
FIXED_MEMBERS = ['COPYING', 'README.txt', 'RELEASE.txt', SUMS, 'SOURCE.txt']
ZIP_TIME = (2026, 10, 19, 0, 0, 0)
REVISION = re.compile(r'[0-9a-f]{40}')
DIGEST = re.compile(r'[0-9a-f]{64}')
VERSION = re.compile(r'[0-9]+\.[0-9]+\.[0-9]+(-[0-9A-Za-z.]+)?')
CI_URL = re.compile(re.escape(REPO) + r'/actions/runs/[0-9]+')
PINNED = re.compile(re.escape(REPO) + r'/((tree|blob)/[0-9a-f]{40}(/\S*)?|actions/runs/[0-9]+)')
SAFE_PART = re.compile(r'[A-Za-z0-9][A-Za-z0-9._-]*')
INLINE_LINK = re.compile(r'\]\(\s*<?([^)\s>]*)')
REFERENCE_LINK = re.compile(r'^ {0,3}\[[^\]]+\]:\s*<?([^\s>]*)', re.M)
BAD_CHARS = set('\\:*?<>|' + chr(34)) | {chr(c) for c in range(32)}
RESERVED = {'CON', 'PRN', 'AUX', 'NUL'} | {p + str(i) for p in ('COM', 'LPT') for i in range(1, 10)}
SOURCE_TEMPLATE = ('Compiled source: {src}\nQualification evidence: {ev}\n'
                   'Source: ' + REPO + '/tree/{src}\n'
                   'Evidence: ' + REPO + '/blob/{ev}/docs/development/preview-session-evidence.json\n'
                   'CI: {ci}\nLicense: GPL-3.0-or-later\n')


class Refusal(Exception):
    '''A validation failure.'''


def need(condition, message):
    if not condition:
        raise Refusal(message)


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def check_name(name, what, strict=True):
    need(isinstance(name, str) and name and not name.startswith('/'), f'{what}: empty or absolute name')
    for part in name.split('/'):
        need(part not in ('', '.', '..') and not (BAD_CHARS & set(part)), f'{what}: unsafe name {name!r}')
        if strict:
            need(SAFE_PART.fullmatch(part) and not part.endswith('.')
                 and part.split('.')[0].upper() not in RESERVED, f'{what}: unsafe name {name!r}')
    return name


def check_unique(names, what):
    keys = [name.casefold() for name in names]
    need(len(set(keys)) == len(keys), f'{what}: duplicate or case-colliding names')
    for key in keys:
        need(not any(other.startswith(key + '/') for other in keys), f'{what}: {key!r} is both file and directory')


def inside(path, root):
    path = os.path.normcase(os.path.realpath(path))
    root = os.path.normcase(os.path.realpath(root))
    try:
        return os.path.commonpath([path, root]) == root
    except ValueError:
        return False


def read_regular(path, what):
    try:
        info = os.lstat(path)
    except OSError:
        raise Refusal(f'{what}: missing or unreadable')
    need(stat.S_ISREG(info.st_mode), f'{what}: not a regular file (symlinks refused)')
    with open(path, 'rb') as handle:
        return handle.read()


def no_alias(path):
    path = os.path.abspath(path)
    while True:
        need(not os.path.islink(path) and not getattr(os.path, 'isjunction', lambda p: False)(path),
             'symlink or junction in path')
        parent = os.path.dirname(path)
        if parent == path:
            break
        path = parent


def read_under(root, rel, what):
    no_alias(root)
    need(os.path.isdir(root), f'{what}: root directory missing')
    path = root
    for part in rel.split('/'):
        path = os.path.join(path, part)
        no_alias(path)
    need(inside(path, root), f'{what}: path escapes its root')
    return read_regular(path, what)


def no_duplicate_keys(pairs):
    keys = [key for key, _ in pairs]
    need(len(keys) == len(set(keys)), 'config: duplicate JSON key')
    return dict(pairs)


def check_component(key, comp):
    need(isinstance(comp, dict) and set(comp) == COMPONENT_KEYS, f'{key}: unexpected component keys')
    version = comp['version']
    need(isinstance(comp['name'], str) and comp['name'].strip(), f'{key}: name missing')
    need(isinstance(version, str) and VERSION.fullmatch(version), f'{key}: bad version')
    need(comp['tag'] == TAG_PREFIX[key] + version, f'{key}: tag must be {TAG_PREFIX[key]}{version}')
    need(isinstance(comp['binary_input'], str) and comp['binary_input'] in BINARY_INPUTS, f'{key}: bad binary_input')
    uploads = [comp['standalone'], comp['archive'], comp['source_archive']]
    for name in uploads:
        check_name(name, f'{key} upload')
        need('/' not in name and version in name, f'{key}: bad upload name {name!r}')
    need(comp['archive'].endswith('.zip') and comp['source_archive'].endswith('-source.zip'),
         f'{key}: archive names must end in .zip and -source.zip')
    check_unique(uploads + [SUMS], f'{key} uploads')
    payloads = comp['payloads']
    need(isinstance(payloads, dict) and payloads, f'{key}: payloads missing')
    for member, source in payloads.items():
        check_name(member, f'{key} payload')
        need(isinstance(source, str) and source in BINARY_INPUTS, f'{key}: payload {member!r} has bad input')
    need(comp['binary_input'] in payloads.values(), f'{key}: binary_input is not packaged')
    check_unique(list(payloads) + FIXED_MEMBERS, f'{key} archive')
    documents = comp['documents']
    need(isinstance(documents, dict) and set(documents) == DOC_NAMES, f'{key}: documents must be README.txt and RELEASE.txt')
    for rel in documents.values():
        check_name(rel, f'{key} document', strict=False)
        need(rel.endswith('.md'), f'{key}: document {rel!r} is not Markdown')


def load_config(path):
    raw = read_regular(path, 'config')
    try:
        config = json.loads(raw.decode('utf-8'), object_pairs_hook=no_duplicate_keys)
    except ValueError as exc:
        raise Refusal(f'config: invalid JSON ({exc})')
    need(isinstance(config, dict) and set(config) == TOP_KEYS, 'config: unexpected top-level keys')
    need(type(config['schema_version']) is int and config['schema_version'] == 1, 'config: schema_version must be 1')
    for key in ('compiled_source_revision', 'evidence_revision'):
        need(isinstance(config[key], str) and REVISION.fullmatch(config[key]), f'config: bad {key}')
    need(config['compiled_source_revision'] != config['evidence_revision'], 'config: source and evidence pins must stay distinct')
    need(isinstance(config['ci_url'], str) and CI_URL.fullmatch(config['ci_url']), 'config: bad ci_url')
    need(config['license'] == 'GPL-3.0-or-later', 'config: license must be GPL-3.0-or-later')
    inputs = config['inputs']
    need(isinstance(inputs, dict) and set(inputs) == INPUT_NAMES, 'config: inputs must be exactly ' + ', '.join(sorted(INPUT_NAMES)))
    for name, meta in inputs.items():
        need(isinstance(meta, dict) and set(meta) == {'bytes', 'sha256'} and type(meta['bytes']) is int
             and meta['bytes'] > 0 and isinstance(meta['sha256'], str) and DIGEST.fullmatch(meta['sha256']),
             f'config: bad input entry {name}')
    unqualified = config['unqualified']
    need(isinstance(unqualified, list) and unqualified and all(isinstance(i, str) and i.strip() for i in unqualified),
         'config: unqualified must list limitations')
    components = config['components']
    need(isinstance(components, dict) and set(components) == set(TAG_PREFIX), 'config: components must be exactly mac and pc')
    for key in sorted(components):
        check_component(key, components[key])
    used = set().union(*(comp['payloads'].values() for comp in components.values()))
    need(used == BINARY_INPUTS, 'config: every binary input must be packaged')
    return raw, config


def load_document(root, rel, version, what):
    raw = read_under(root, rel, what)
    need(not raw.startswith(b'\xef\xbb\xbf'), f'{what}: UTF-8 BOM refused')
    try:
        text = raw.decode('utf-8')
    except UnicodeDecodeError:
        raise Refusal(f'{what}: not UTF-8')
    text = text.replace('\r\n', '\n')
    need('\r' not in text and text.endswith('\n'), f'{what}: stray CR or missing final newline')
    need(version in text, f'{what}: version {version} missing')
    for target in INLINE_LINK.findall(text) + REFERENCE_LINK.findall(text):
        need(target.startswith('#') or (target.startswith('https://')
             and (not target.startswith(REPO) or PINNED.fullmatch(target))),
             f'{what}: relative, insecure or unpinned link {target!r}')
    return text.encode('utf-8')


def inspect_source_zip(data):
    try:
        archive = zipfile.ZipFile(io.BytesIO(data))
    except zipfile.BadZipFile:
        raise Refusal('source.zip: not a ZIP archive')
    with archive:
        infos = archive.infolist()
    need(infos, 'source.zip: empty')
    for info in infos:
        name = info.orig_filename
        check_name(name[:-1] if name.endswith('/') else name, 'source.zip member', strict=False)
        need(not stat.S_ISLNK(info.external_attr >> 16) and not info.flag_bits & 1,
             f'source.zip: symlink or encrypted member {name!r}')
    check_unique([info.orig_filename for info in infos], 'source.zip')


def build_zip(members):
    buffer = io.BytesIO()
    with zipfile.ZipFile(buffer, 'w') as archive:
        for name in sorted(members):
            info = zipfile.ZipInfo(name, ZIP_TIME)
            info.create_system = 3
            info.external_attr = (stat.S_IFREG | 0o644) << 16
            info.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(info, members[name], compress_type=zipfile.ZIP_DEFLATED, compresslevel=9)
    return buffer.getvalue()


def checksums(files):
    return ''.join(f'{sha256(files[name])}  {name}\n' for name in sorted(files)).encode('ascii')


def marker(config):
    lines = ['ClasSICK paired-native-preview-1: APPROVAL PENDING', '',
             'These candidates are UNPUBLISHED; approval pending; NOT UPLOADED, tagged or released.',
             'Published versions remain ROM 0.2.0-alpha.1 and PC unreleased until the owner explicitly accepts them.',
             '', 'Proposed tags:']
    for key in sorted(config['components']):
        comp = config['components'][key]
        lines.append('- ' + comp['tag'] + ' (' + comp['name'] + ' ' + comp['version'] + ')')
    lines += ['', 'Unqualified limitations needing explicit owner acceptance (not passed):']
    lines += ['- ' + item for item in config['unqualified']]
    return ('\n'.join(lines) + '\n').encode('utf-8')


def plan(args):
    raw, config = load_config(args.config)
    inputs = {}
    for name, meta in sorted(config['inputs'].items()):
        data = read_under(args.inputs, name, f'input {name}')
        need(len(data) == meta['bytes'] and sha256(data) == meta['sha256'], f'input {name}: size or SHA-256 mismatch')
        inputs[name] = data
    inspect_source_zip(inputs['source.zip'])
    source_text = SOURCE_TEMPLATE.format(src=config['compiled_source_revision'],
                                         ev=config['evidence_revision'], ci=config['ci_url']).encode('ascii')
    tree, components = {}, {}
    for key in sorted(config['components']):
        comp = config['components'][key]
        members = {member: inputs[source] for member, source in comp['payloads'].items()}
        members['COPYING'] = inputs['COPYING']
        for out, rel in sorted(comp['documents'].items()):
            members[out] = load_document(args.docs, rel, comp['version'], f'{key} {out}')
        members['SOURCE.txt'] = source_text
        members[SUMS] = checksums(members)
        uploads = {comp['standalone']: inputs[comp['binary_input']],
                   comp['archive']: build_zip(members),
                   comp['source_archive']: inputs['source.zip']}
        uploads[SUMS] = checksums(uploads)
        for name, data in uploads.items():
            tree[key + '/' + name] = data
        components[key] = {'tag': comp['tag'], 'version': comp['version'],
                           'assets': {n: {'bytes': len(d), 'sha256': sha256(d)} for n, d in sorted(uploads.items())}}
    review = {'schema_version': 1, 'state': 'approval-pending', 'published': False,
              'compiled_source_revision': config['compiled_source_revision'],
              'evidence_revision': config['evidence_revision'],
              'config_sha256': sha256(raw), 'components': components}
    tree['review.json'] = (json.dumps(review, indent=2, sort_keys=True) + '\n').encode('ascii')
    tree['APPROVAL-PENDING.txt'] = marker(config)
    return tree


def check_output(args):
    output = os.path.abspath(args.output)
    no_alias(output)
    need(not os.path.lexists(output), 'output already exists; refusing to touch it')
    need(os.path.isdir(os.path.dirname(output)), 'output parent directory missing')
    for root in (args.inputs, args.docs):
        need(not inside(output, root) and not inside(root, output), 'output overlaps the inputs or docs tree')
    return output


def write_tree(output, tree):
    os.mkdir(output)
    for sub in sorted(TAG_PREFIX):
        os.mkdir(os.path.join(output, sub))
    for rel, data in sorted(tree.items()):
        with open(os.path.join(output, *rel.split('/')), 'xb') as handle:
            handle.write(data)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('--config', '--inputs', '--docs', '--output'):
        parser.add_argument(option, required=True)
    args = parser.parse_args(argv)
    try:
        check_output(args)
        tree = plan(args)
        write_tree(check_output(args), tree)
    except (Refusal, OSError, TypeError, KeyError, ValueError, zipfile.BadZipFile) as exc:
        print(json.dumps({'result': 'refused', 'error': str(exc)}))
        return 1
    print(json.dumps({'result': 'built', 'state': 'approval-pending', 'published': False, 'files': sorted(tree)}, indent=2))
    return 0


if __name__ == '__main__':
    sys.exit(main())
