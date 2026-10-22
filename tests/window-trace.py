# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Original exact-layout parser fixtures, not a boot-observed trace."""
import importlib.util
import struct
import sys
from pathlib import Path

root = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('window_trace', root / 'scripts/Read-WindowTrace.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
data = bytearray(160)
struct.pack_into('<3I', data, 0, 0x31505743, 1, 160)
for i in range(3, 39):
    struct.pack_into('<I', data, i * 4, 0x100 + i)
for offset in (24, 52, 64, 68, 80, 84, 120):
    struct.pack_into('<i', data, offset, -offset)
struct.pack_into('<Q', data, 128, 0x1234567890ABCDEF)
struct.pack_into('<Q', data, 136, 0xFEDCBA0987654321)
d = module.decode(data)
assert d['phase'] == 0x103 and d['result'] == 0x104
assert d['cause'] == -24 and d['session_cause'] == -52
assert d['pointer_x'] == -64 and d['pointer_y'] == -68
assert d['window_x'] == -80 and d['window_y'] == -84
assert d['publication_cause'] == -120 and d['drained'] == 0x11F
assert d['polls'] == 0x1234567890ABCDEF and d['ticks'] == 0xFEDCBA0987654321
assert d['arena_used'] == 0x124 and d['timer_bits'] == 0x125 and d['timer_port'] == 0x126
rejections = 0
for n in range(161):
    if n == 160:
        continue
    try:
        module.decode(data[:n])
    except ValueError:
        rejections += 1
    else:
        raise AssertionError(n)
for offset in (0, 4, 8, 156):
    bad = bytearray(data)
    bad[offset] ^= 1
    try:
        module.decode(bad)
    except ValueError:
        rejections += 1
    else:
        raise AssertionError(offset)
try:
    module.decode(data + b'\0')
except ValueError:
    rejections += 1
else:
    raise AssertionError('overlong')
print(f'TEST-0073 trace reader: exact offsets/signed/64-bit PASS; {rejections} refusals')
if len(sys.argv) == 2:
    actual = module.decode(Path(sys.argv[1]).read_bytes())
    assert actual['phase'] == 3 and actual['result'] == 1
    assert (actual['pointer_x'], actual['pointer_y']) == (212, 160)
    assert (actual['window_x'], actual['window_y'], actual['pattern']) == (100, 80, 1)
    assert actual['inputs'] == 2 and actual['published_frames'] == 11
    assert actual['publication_phase'] == 0 and actual['publication_rows'] == 0
    assert actual['ticks'] >= 62 * 3579545
    print('TEST-0073 native RAM trace roundtrip PASS; no image executed')
