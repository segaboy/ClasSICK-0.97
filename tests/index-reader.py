#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Real Git-index controls for the batch reader, without repository mutations."""
from importlib.util import module_from_spec,spec_from_file_location
from pathlib import Path
import subprocess
import tempfile
import unittest
import os
spec=spec_from_file_location('index_text',Path(__file__).resolve().parents[1]/'scripts/read_index_text.py')
mod=module_from_spec(spec);spec.loader.exec_module(mod)
class IndexTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory(dir=os.environ.get('CLASSICK_TEST_TEMP'));self.root=Path(self.tmp.name)
        self.git('init','-q');self.git('config','core.autocrlf','false')
    def tearDown(self):
        parent=Path(os.environ.get('CLASSICK_TEST_TEMP') or tempfile.gettempdir()).resolve()
        if self.root.resolve().parent!=parent:raise ValueError('Unexpected cleanup target')
        self.tmp.cleanup()
    def git(self,*args):return subprocess.check_output(['git','-C',str(self.root),*args],stderr=subprocess.STDOUT)
    def stage(self,name,data):
        (self.root/name).write_bytes(data);self.git('add','--',name)
    def test_index_authority_and_exact_unicode(self):
        self.stage('own.md','Original café\r\nwith final newline\n'.encode())
        (self.root/'own.md').write_text('different working tree')
        self.assertEqual(mod.read(self.root),[{'path':'own.md','size':35,'content':'Original café\r\nwith final newline\n'}])
    def test_binary_skip_is_only_extension_not_admission(self):
        self.stage('test.png',b'\0\xff');self.stage('test.gif',b'\xff')
        self.stage('own.c',b'own\n');self.assertEqual([x['path'] for x in mod.read(self.root)],['own.c'])
    def test_bad_utf8(self):
        self.stage('own.c',b'\xff')
        with self.assertRaises(UnicodeDecodeError):mod.read(self.root)
    def test_nul(self):
        self.stage('own.c',b'own\0text')
        with self.assertRaises(ValueError):mod.read(self.root)
    def test_bound(self):
        self.stage('own.md',b'x'*1048577)
        with self.assertRaises(ValueError):mod.read(self.root)
    def test_unapproved_path(self):
        self.stage('own text.md',b'own')
        with self.assertRaises(ValueError):mod.read(self.root)
    def test_empty(self):self.assertEqual(mod.read(self.root),[])
if __name__=='__main__':unittest.main()
