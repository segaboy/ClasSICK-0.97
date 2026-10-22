# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Decode only the exact SPEC-0062 CWP1 diagnostic record; no legacy guessing."""
import argparse
import json
import struct
from pathlib import Path


def decode(data):
    if len(data) != 160:
        raise ValueError('Window trace must contain exactly 160 bytes')
    words = struct.unpack('<40I', data)
    if words[:3] != (0x31505743, 1, 160) or words[39] != 0:
        raise ValueError('Unsupported window trace identity/version/reserved field')
    names = ('phase', 'result', 'module', 'cause', 'ready', 'handoff_stage',
             'bus_result', 'session_phase', 'session_stage', 'session_result',
             'session_cause', 'delivery', 'consumed', 'pointer_x', 'pointer_y',
             'pointer_down', 'suppressed', 'window_x', 'window_y', 'dragging',
             'pattern', 'inputs', 'ram_frames', 'published_frames',
             'publication_phase', 'publication_rows', 'publication_required',
             'publication_cause', 'drained')
    result = dict(zip(names, words[3:32]))
    for name in ('cause', 'session_cause', 'pointer_x', 'pointer_y', 'window_x',
                 'window_y', 'publication_cause'):
        if result[name] >= 0x80000000:
            result[name] -= 0x100000000
    result.update(polls=struct.unpack_from('<Q', data, 128)[0],
                  ticks=struct.unpack_from('<Q', data, 136)[0],
                  arena_used=words[36], timer_bits=words[37], timer_port=words[38])
    return result


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('record', type=Path)
    args = parser.parse_args()
    try:
        print(json.dumps(decode(args.record.read_bytes()), indent=2))
    except (OSError, ValueError) as error:
        parser.exit(1, str(error) + '\n')
