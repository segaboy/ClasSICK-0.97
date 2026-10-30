# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright 2026 Dean Howell. Synthetic Windows caller checks; no live input.
import json
import os
import subprocess
import sys

CASES = {
    'good': (0, (2, 3, 2, 1)), 'open': (1, (0, 0, 0, 0)),
    'hash_first': (1, (1, 0, 1, 1)), 'hash_final': (1, (2, 2, 2, 1)),
    'close_first': (1, (1, 2, 1, 1)), 'close_final': (1, (2, 3, 2, 1)),
    'close_process': (1, (2, 3, 2, 1)), 'snapshot_refuse': (1, (1, 2, 1, 1)),
    'snapshot_error': (1, (1, 2, 1, 1)), 'window_changed': (1, (1, 2, 1, 1)),
    'final_window_changed': (1, (2, 3, 2, 1)), 'late_dead': (1, (2, 2, 2, 1)),
    'path': (1, (1, 0, 1, 1)), 'desktop': (1, (1, 0, 1, 1)),
    'count': (1, (1, 1, 1, 1)), 'pid': (1, (1, 1, 1, 1)),
}
for name, (code, counts) in CASES.items():
    env = os.environ.copy()
    env['WIH_CASE'] = name
    run = subprocess.run([sys.argv[1]], env=env, capture_output=True, text=True, timeout=5, check=False)
    rows = [line for line in run.stderr.splitlines() if line.startswith('COUNTS ')]
    if run.returncode != code or len(rows) != 1 or tuple(map(int, rows[0].split()[1:])) != counts:
        raise AssertionError((name, run.returncode, run.stderr))
    if name == 'good' and json.loads(run.stdout) != {'synthetic': True}:
        raise AssertionError((name, run.stdout))
print('PASS 16 owned-context synthetic caller cases; no native input')
