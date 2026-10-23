#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Synthetic audit controls and SR model; no privileged instruction execution."""
import importlib.util
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('exclusion_audit', ROOT/'scripts/Verify-M68kExclusion.py')
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)
GOOD = '''00000000 <cs_m68k_foreground_enter>:
   0:\t40c0 \tmovew %sr,%d0
   2:\t007c 0700 \toriw #1792,%sr
   6:\t4e75 \trts

00000008 <cs_m68k_foreground_leave>:
   8:\t302f 000a \tmovew %sp@(10),%d0
   c:\t46c0 \tmovew %d0,%sr
   e:\t4e75 \trts
'''

class Controls(unittest.TestCase):
    def refused(self, text):
        with self.assertRaises(ValueError):
            audit.check(audit.parse(text))

    def test_valid_register_and_hex_immediate(self):
        audit.check(audit.parse(GOOD))
        audit.check(audit.parse(GOOD.replace('#1792', '#0x0700')))

    def test_wrong_mask(self):
        self.refused(GOOD.replace('007c 0700', '007c 0200').replace('#1792', '#512'))

    def test_missing_snapshot(self):
        self.refused(GOOD.replace('   0:\t40c0 \tmovew %sr,%d0\n', ''))

    def test_reversed_operations(self):
        text=GOOD.replace('   0:\t40c0 \tmovew %sr,%d0\n', '')
        self.refused(text.replace('   6:', '   4:\t40c0 \tmovew %sr,%d0\n   6:'))

    def test_constant_restore(self):
        self.refused(GOOD.replace('46c0 \tmovew %d0,%sr', '46fc 2000 \tmovew #8192,%sr'))

    def test_extra_sr_after_blank(self):
        self.refused(GOOD.replace('   e:', '\n   d:\t007c 0700 \toriw #1792,%sr\n   e:'))

    def test_missing_api_and_bad_syntax(self):
        self.refused(GOOD.split('00000008')[0])
        self.refused(GOOD.replace('   e:\t4e75 \trts', 'unexpected text'))

    def test_clobbers_not_comments(self):
        source=(ROOT/'arch/m68k/exclusion.c').read_text()
        self.assertTrue(audit.clobbers_ok(source))
        self.assertFalse(audit.clobbers_ok(source.replace('"memory"', '/* "memory" */')))
        self.assertFalse(audit.clobbers_ok(source.replace('"cc"', '/* "cc" */')))
        self.assertFalse(audit.clobbers_ok(source.replace('__asm__ volatile', '__asm__')))

    def test_snapshot_model(self):
        # A mathematical model of the selected protocol, NOT the CPU callbacks.
        for priority in range(8):
            for ccr in range(32):
                original=0x2000 | (priority << 8) | ccr
                saved=original & 0xffff
                held=original | 0x0700
                self.assertEqual(held & 0x0700, 0x0700)
                self.assertEqual(held & ~0x0700, original & ~0x0700)
                self.assertEqual(saved, original)
                inner_saved=held
                self.assertEqual(inner_saved, held)  # inner LIFO restore
                self.assertEqual(saved & 0x0700, priority << 8)

if __name__ == '__main__':
    unittest.main()
