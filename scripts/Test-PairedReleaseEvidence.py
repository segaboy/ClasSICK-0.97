#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell
"""Regression test for the configurable evidence locator (schema 1 and 2).

Uses only temporary synthetic inputs; the output is not a native package.
"""
import argparse
import hashlib
import io
import json
import os
import shutil
import subprocess
import sys
import tempfile
import zipfile

REPO = 'https://github.com/segaboy/ClasSICK-0.97'
SRC, EV, VER = 'a' * 40, 'b' * 40, '1.0.0'
V1P = 'docs/development/preview-session-evidence.json'
V2P = 'docs/evidence/Run_1-a.b.json'
HERE = os.path.dirname(os.path.abspath(__file__))
NL, BS, TAB = chr(10), chr(92), chr(9)
checks = 0
failures = []


def check(cond, msg):
    global checks
    checks += 1
    if not cond:
        failures.append(msg)


def write(path, data):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'wb') as handle:
        handle.write(data)


def make_inputs(root):
    buf = io.BytesIO()
    with zipfile.ZipFile(buf, 'w') as archive:
        member = zipfile.ZipInfo('src/readme.txt', (2026, 10, 19, 0, 0, 0))
        member.create_system = 3
        member.external_attr = 0o100644 << 16
        archive.writestr(member, 'synthetic source text' + NL)
    files = {'rom.rom': b'synthetic-rom-bytes', 'BOOTX64.EFI': b'synthetic-efi-bytes',
             'pc.img': b'synthetic-img-bytes', 'COPYING': b'synthetic license text\n',
             'source.zip': buf.getvalue()}
    for name, data in files.items():
        write(os.path.join(root, name), data)
    return {n: {'bytes': len(d), 'sha256': hashlib.sha256(d).hexdigest()} for n, d in files.items()}


def make_docs(root):
    for key in ('mac', 'pc'):
        for doc in ('README', 'RELEASE'):
            text = '# ' + key + ' ' + doc + ' ' + VER + NL + NL + 'Synthetic. See https://example.com/x' + NL
            write(os.path.join(root, key, doc + '.md'), text.encode())


def make_config(meta, schema, path=None):
    def comp(key, name, tag, binary, standalone, payloads):
        return {'name': name, 'version': VER, 'tag': tag, 'binary_input': binary,
                'standalone': standalone, 'archive': key + '-' + VER + '.zip',
                'source_archive': key + '-' + VER + '-source.zip', 'payloads': payloads,
                'documents': {'README.txt': key + '/README.md', 'RELEASE.txt': key + '/RELEASE.md'}}
    cfg = {'schema_version': schema, 'compiled_source_revision': SRC, 'evidence_revision': EV,
           'ci_url': REPO + '/actions/runs/1', 'license': 'GPL-3.0-or-later', 'inputs': meta,
           'components': {'mac': comp('mac', 'Synthetic Mac', 'rom-v' + VER, 'rom.rom',
                                      'mac-' + VER + '.rom', {'rom.rom': 'rom.rom'}),
                          'pc': comp('pc', 'Synthetic PC', 'pc-v' + VER, 'pc.img', 'pc-' + VER + '.img',
                                     {'BOOTX64.EFI': 'BOOTX64.EFI', 'pc.img': 'pc.img'})},
           'unqualified': ['synthetic test only']}
    if path is not None:
        cfg['evidence_path'] = path
    return cfg


def run(script, *args):
    proc = subprocess.run([sys.executable, script] + list(args), capture_output=True, text=True, timeout=120)
    try:
        result = json.loads(proc.stdout)
    except ValueError:
        result = {}
    return proc.returncode, result


def tree(root):
    out = {}
    for base, _, names in os.walk(root):
        for name in names:
            full = os.path.join(base, name)
            with open(full, 'rb') as handle:
                out[os.path.relpath(full, root).replace(os.sep, '/')] = handle.read()
    return out


def source_literal(path):
    return ('Compiled source: ' + SRC + NL + 'Qualification evidence: ' + EV + NL
            + 'Source: ' + REPO + '/tree/' + SRC + NL + 'Evidence: ' + REPO + '/blob/' + EV + '/' + path + NL
            + 'CI: ' + REPO + '/actions/runs/1' + NL + 'License: GPL-3.0-or-later' + NL)


def source_members(pkg):
    found = {}
    for key in ('mac', 'pc'):
        with zipfile.ZipFile(os.path.join(pkg, key, key + '-' + VER + '.zip')) as archive:
            found[key] = archive.read('SOURCE.txt').decode('ascii')
    return found


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--scripts', default=HERE)
    parser.add_argument('--old-builder')
    args = parser.parse_args()
    builder = os.path.join(args.scripts, 'Prepare-PairedRelease.py')
    auditor = os.path.join(args.scripts, 'Test-PairedRelease.py')
    with tempfile.TemporaryDirectory() as tmp:
        inputs, docs = os.path.join(tmp, 'inputs'), os.path.join(tmp, 'docs')
        meta = make_inputs(inputs)
        make_docs(docs)
        counter = [0]

        def cfg_file(obj):
            counter[0] += 1
            path = os.path.join(tmp, 'cfg%d.json' % counter[0])
            text = obj if isinstance(obj, str) else json.dumps(obj, indent=2, sort_keys=True)
            write(path, text.encode('utf-8'))
            return path

        def build(script, cfg, out, inp=inputs):
            return run(script, '--config', cfg, '--inputs', inp, '--docs', docs, '--output', out)

        def audit(cfg, pkg, inp=inputs):
            return run(auditor, '--config', cfg, '--inputs', inp, '--docs', docs, '--package', pkg)

        c1, c2 = cfg_file(make_config(meta, 1)), cfg_file(make_config(meta, 2, V2P))
        out1, out2a, out2b = (os.path.join(tmp, n) for n in ('o1', 'o2a', 'o2b'))
        for cfg, out in ((c1, out1), (c2, out2a), (c2, out2b)):
            rc, res = build(builder, cfg, out)
            check(rc == 0 and res.get('result') == 'built', 'build failed for ' + out)
        for cfg, out in ((c1, out1), (c2, out2a)):
            rc, res = audit(cfg, out)
            check(rc == 0 and res.get('result') == 'pass', 'audit failed for ' + out + str(res))
        if failures:
            return finish()
        src1, src2 = source_members(out1), source_members(out2a)
        for key in ('mac', 'pc'):
            check(src1[key] == source_literal(V1P), key + ' v1 SOURCE.txt literal mismatch')
            check(src2[key] == source_literal(V2P), key + ' v2 SOURCE.txt literal mismatch')
            check(src1[key] != src2[key], key + ' v2 locator equals v1')
            check('Evidence: ' + REPO + '/blob/' + EV + '/' + V2P + NL in src2[key], key + ' pinned line')
        check(tree(out2a) == tree(out2b), 'v2 twin trees differ')
        r1 = json.loads(tree(out1)['review.json'])
        r2 = json.loads(tree(out2a)['review.json'])
        check(r1['schema_version'] == 1 and 'evidence_path' not in r1, 'v1 review.json changed')
        check(r2['schema_version'] == 1 and r2.get('evidence_path') == V2P, 'v2 review.json schema/path wrong')
        check(audit(c1, out2a)[1].get('result') == 'fail', 'auditor accepted v2 package under v1 locator')
        check(audit(c2, out1)[1].get('result') == 'fail', 'auditor accepted v1 package under v2 locator')
        # Change only the locator; reseal both checksum layers and review metadata.
        # An independent content check must still reject each component.
        for key in ('mac', 'pc'):
            altered = os.path.join(tmp, 'resealed-' + key)
            shutil.copytree(out2a, altered)
            zpath = os.path.join(altered, key, key + '-' + VER + '.zip')
            with zipfile.ZipFile(zpath) as archive:
                infos = archive.infolist()
                members = {i.filename: archive.read(i) for i in infos}
            members['SOURCE.txt'] = source_literal(V2P.replace('Run_1', 'Run_2')).encode('ascii')
            def sums(items):
                return ''.join(hashlib.sha256(data).hexdigest() + '  ' + name + NL
                               for name, data in sorted(items.items())).encode('ascii')
            members['SHA256SUMS.txt'] = sums({n: b for n, b in members.items() if n != 'SHA256SUMS.txt'})
            with zipfile.ZipFile(zpath, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
                for info in infos:
                    archive.writestr(info, members[info.filename])
            contents = tree(os.path.join(altered, key))
            write(os.path.join(altered, key, 'SHA256SUMS.txt'),
                  sums({n: b for n, b in contents.items() if n != 'SHA256SUMS.txt'}))
            contents = tree(os.path.join(altered, key))
            review = json.loads(tree(altered)['review.json'])
            review['components'][key]['assets'] = {
                n: {'bytes': len(b), 'sha256': hashlib.sha256(b).hexdigest()} for n, b in contents.items()}
            write(os.path.join(altered, 'review.json'), (json.dumps(review, indent=2, sort_keys=True) + NL).encode())
            rc, result = audit(c2, altered)
            check(rc == 1 and key + ': member SOURCE.txt content mismatch' in result.get('errors', []),
                  key + ' resealed wrong evidence locator escaped content audit')
        if args.old_builder:
            old = os.path.join(tmp, 'old')
            rc, res = build(args.old_builder, c1, old)
            check(rc == 0 and tree(old) == tree(out1), 'v1 tree differs from old builder')

        base2 = make_config(meta, 2, V2P)
        cases = {}
        v1extra = make_config(meta, 1)
        v1extra['evidence_path'] = V1P
        cases['v1 with evidence_path'] = v1extra
        missing = dict(base2)
        del missing['evidence_path']
        cases['v2 missing evidence_path'] = missing
        for label, value in (('int', 5), ('null', None), ('list', [V2P])):
            cases['v2 evidence_path ' + label] = dict(base2, evidence_path=value)
        for label, value in (('bool', True), ('string', '2'), ('three', 3), ('zero', 0), ('float', 2.0), ('null', None)):
            cases['schema ' + label] = dict(base2, schema_version=value)
        cases['extra top key'] = dict(base2, extra=1)
        cases['v1 extra top key'] = dict(make_config(meta, 1), extra=1)
        text = json.dumps(base2)
        cases['duplicate key'] = text[:-1] + ', "license": "GPL-3.0-or-later"}'
        cases['duplicate schema'] = text[:-1] + ', "schema_version": 2}'
        bad_paths = ['/docs/a.json', 'docs/a.txt', 'docs/a.json' + NL, 'docs/../a.json', 'docs/./a.json',
                     'docs//a.json', 'docs/a./b.json', 'https://x/docs/a.json', 'C:/docs/a.json',
                     'docs' + BS + 'a.json', 'docs/a%2e.json', 'docs/a b.json', 'docs/a#.json',
                     'docs/a?.json', 'docs/a:b.json', 'docs/\u00e9.json', 'docs/.json', 'docs/-a.json',
                     'docs/_a.json', 'docs/a.json/', 'docs/a.JSON', 'evidence.json', 'docs/',
                     'docs/a' + TAB + '.json', 'docs/a.json ', ' docs/a.json', 'docs/a.', 'docs/a/..',
                     'docs/a.json#x', 'docs/a.json?x', 'file:///docs/a.json', 'docs/a/b./c.json']
        for path in bad_paths:
            cases['bad path ' + ascii(path)] = dict(base2, evidence_path=path)
        for label, obj in sorted(cases.items()):
            cfg = cfg_file(obj)
            out = os.path.join(tmp, 'bad%d' % counter[0])
            missing_inputs = os.path.join(tmp, 'no-such-inputs')
            rc, res = build(builder, cfg, out, missing_inputs)
            check(rc == 1 and res.get('result') == 'refused' and res.get('error', '').startswith('config:')
                  and not os.path.exists(out), 'builder did not refuse at config: ' + label)
            rc, res = audit(cfg, out1, missing_inputs)
            errs = res.get('errors') or ['']
            check(rc == 1 and res.get('result') == 'fail' and errs[0].startswith('config:'),
                  'auditor did not refuse at config: ' + label)
    return finish()


def finish():
    print(json.dumps({'result': 'fail' if failures else 'pass', 'checks': checks, 'failures': failures[:20]}))
    return 1 if failures else 0


if __name__ == '__main__':
    sys.exit(main())
