# SPDX-License-Identifier: GPL-3.0-or-later
# Original fake-backend contract checks, not host or guest input tests.
import json
import subprocess
import sys

cases = [json.loads(subprocess.check_output([sys.argv[1], str(i)], text=True)) for i in range(8)]
assert cases[0]['window_pid'] == 42
assert cases[0]['cursor_screen']['x'] == 12 and cases[0]['cursor_physical']['x'] == 24
assert cases[0]['clip_rect']['right'] == 3 and cases[0]['cursor_info']['showing']
assert cases[0]['gui_capture']['pid_match']
for i in range(1, 5):
    assert cases[i]['refused'] and cases[i]['refusal_code'] == i
    assert cases[i]['cursor_screen'] is None and cases[i]['gui_capture'] is None
assert all(not cases[5][k]['ok'] for k in ('cursor_screen', 'cursor_physical', 'clip_rect', 'cursor_info'))
assert cases[5]['gui_capture']['query_ok'] is False
assert cases[6]['gui_capture']['capture_present'] is False
assert cases[7]['gui_capture']['capture_pid'] == 99 and cases[7]['gui_capture']['pid_match'] is False
print('PASS capture-context fake backend: 8 cases')
