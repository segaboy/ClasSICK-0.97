# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Exercise source-type guards using isolated copies of the Git index.

The real index and working files are never changed. Original fixture blobs may
be added to Git's object store but are never referenced by the real index.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    out = args.output.resolve()
    if out == ROOT or ROOT in out.parents:
        parser.error('output must be outside repository')
    shell = shutil.which('pwsh') or shutil.which('powershell')
    if not shell:
        parser.error('PowerShell is required')
    index = Path(subprocess.check_output(['git', 'rev-parse', '--git-path', 'index'], cwd=ROOT, text=True).strip())
    if not index.is_absolute():
        index = ROOT / index
    before = sha(index)
    out.mkdir(parents=True, exist_ok=False)
    cases = []
    fixtures = (
        ('source', 'tests/source-type-control.asm', b'; SPDX-License-Identifier: GPL-3.0-or-later\n; Original source-type fixture.\n', None),
        ('binary', 'tests/source-type-control.asm', b'original\0fixture', 'NUL in source/text blob'),
        ('executable', 'tests/source-type-control.exe', b'original text fixture\n', 'Non-source/text payload needs explicit policy review:'),
    )
    for name, path, payload, error in fixtures:
        isolated = out / (name + '.index'); shutil.copyfile(index, isolated)
        env = os.environ.copy(); env['GIT_INDEX_FILE'] = str(isolated)
        blob = subprocess.check_output(['git', 'hash-object', '-w', '--stdin'], input=payload, cwd=ROOT).decode().strip()
        subprocess.run(['git', 'update-index', '--add', '--cacheinfo', '100644', blob, path],
                       cwd=ROOT, env=env, check=True, timeout=20)
        cmd = [shell, '-NoProfile', '-NonInteractive', '-File', str(ROOT/'scripts/Test-Repository.ps1'), '-Python', sys.executable]
        result = subprocess.run(cmd, cwd=ROOT, env=env, capture_output=True, text=True, timeout=45,
                                creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        log = result.stdout + result.stderr
        (out / (name + '.log')).write_text(log, encoding='utf-8')
        if (error is None and result.returncode != 0) or (error is not None and (result.returncode == 0 or error not in log)):
            raise ValueError('unexpected guard result: ' + name)
        cases.append({'name': name, 'path': path, 'blob': blob, 'returncode': result.returncode,
                      'expected_error': error, 'command': cmd})
    if sha(index) != before:
        raise ValueError('real index changed during guard tests')
    report = {'status': 'pass', 'real_index_unchanged': True, 'cases': cases,
              'script_sha256': sha(Path(__file__)), 'guard_sha256': sha(ROOT/'scripts/Test-Repository.ps1')}
    (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
