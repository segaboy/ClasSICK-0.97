# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Literal CSPTIM01 vectors; no emulator or target execution."""
import importlib.util
import unittest
from pathlib import Path

p = Path(__file__).resolve().parents[1] / 'scripts' / 'Read-PCInputTimeline.py'
spec = importlib.util.spec_from_file_location('timeline_reader', p)
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)


def lines(words):
    return '\n'.join(f'r{i+8}={v:016x}' for i, v in enumerate(words))


class TimelineReader(unittest.TestCase):
    def test_partial_marker(self):
        w = [0x43535054494D3031, 0x0014000A02020001,
             0x03020100000100C8, 0, 0, 0, 0, 0]
        result = mod.decode(lines(w))
        self.assertEqual(result['pointer'], [10, 20])
        self.assertEqual(result['stop_reason'], 'drained-marker')
        self.assertEqual(result['entries'][0], {
            'byte': 0xc8, 'used_before': 0, 'used_after': 1,
            'delivery_kind': 0, 'error_changed': 0, 'session_phase': 1,
            'owner_phase': 2, 'owner_stage': 3})

    def test_empty_fault_and_invalid_coordinates(self):
        w = [0x43535054494D3031, 0xFFFFFFFF03020000, 0, 0, 0, 0, 0, 0]
        result = mod.decode(lines(w))
        self.assertEqual(result['stop_reason'], 'session-fault')
        self.assertEqual(result['pointer'], [None, None])
        self.assertEqual(result['entries'], [])

    def test_refusals(self):
        good = [0x43535054494D3031, 0x0014000A02020000, 0, 0, 0, 0, 0, 0]
        for index, value in [(0, 0), (1, 0x0014000A01020000),
                             (1, 0x0014000A02040000), (2, 1)]:
            w = good.copy(); w[index] = value
            with self.subTest(index=index, value=value), self.assertRaises(ValueError):
                mod.decode(lines(w))
        with self.assertRaises(ValueError):
            mod.decode(lines(good) + '\nr8=0')
        with self.assertRaises(ValueError):
            mod.decode(lines(good[:-1]))


if __name__ == '__main__':
    unittest.main()
