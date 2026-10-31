# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Synthetic workspace checks; no ROM is executed."""
import copy
import hashlib
import importlib.util
import json
import os
import tempfile
import unittest
from pathlib import Path

p = Path(__file__).resolve().parents[1] / 'scripts' / 'Verify-SnowWorkspace.py'
spec = importlib.util.spec_from_file_location('snow_workspace', p)
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)


class SnowWorkspace(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(dir=os.environ.get('CLASSICK_TEST_TMP'))
        self.addCleanup(self.tmp.cleanup)
        root = Path(self.tmp.name)
        self.rom = root / 'own.rom'
        self.rom.write_bytes(bytes(65536))
        self.sha = hashlib.sha256(bytes(65536)).hexdigest()
        self.ws = root / 'test.snoww'
        self.cfg = {'rom_path': str(self.rom), 'model': 'Early128K',
                    'init_args': {'mouse_mode': 'RelativeHw', 'audio_disabled': True},
                    'shared_dir': None, 'pram_path': None, 'extension_rom_path': None,
                    'display_card_rom_path': None, 'floppy_images': [],
                    'scsi_targets': ['None'] * 7}

    def run_cfg(self, cfg):
        self.ws.write_text(json.dumps(cfg), encoding='utf-8')
        return mod.verify(str(self.ws), str(self.rom), self.sha)

    def test_valid_and_mismatched_workspace_rom(self):
        self.assertTrue(self.run_cfg(self.cfg)['workspace_path_matches'])
        cfg = copy.deepcopy(self.cfg)
        cfg['rom_path'] = str(self.rom.parent / 'different.rom')
        with self.assertRaisesRegex(ValueError, 'path differs'):
            self.run_cfg(cfg)

    def test_isolation_refusals(self):
        for key, val in [('shared_dir', 'elsewhere'), ('pram_path', 'x'),
                         ('extension_rom_path', 'x'), ('display_card_rom_path', 'x'),
                         ('floppy_images', ['x']), ('scsi_targets', ['x'] * 7),
                         ('model', 'other')]:
            cfg = copy.deepcopy(self.cfg); cfg[key] = val
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.run_cfg(cfg)
        cfg = copy.deepcopy(self.cfg); cfg['init_args']['mouse_mode'] = 'other'
        with self.assertRaises(ValueError):
            self.run_cfg(cfg)

    def test_identity_and_duplicate_keys(self):
        self.ws.write_text(json.dumps(self.cfg), encoding='utf-8')
        with self.assertRaises(ValueError):
            mod.verify(str(self.ws), str(self.rom), '0' * 64)
        self.rom.write_bytes(bytes(65537))
        with self.assertRaises(ValueError):
            mod.verify(str(self.ws), str(self.rom), self.sha)
        self.ws.write_text('{"rom_path":"x","rom_path":"y"}', encoding='utf-8')
        with self.assertRaisesRegex(ValueError, 'duplicate'):
            mod.verify(str(self.ws), str(self.rom), self.sha)


if __name__ == '__main__':
    unittest.main()
