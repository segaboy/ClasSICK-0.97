# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
import copy
import json
from pathlib import Path
import sys
import unittest
root=Path(__file__).resolve().parents[1];sys.path.insert(0,str(root/'scripts'))
import check_animations as gate
class Admission(unittest.TestCase):
    def setUp(self):
        self.row=json.loads((root/gate.MANIFEST).read_text())['animations'][0];self.data=(root/gate.PATH).read_bytes()
    def test_reviewed_original(self):gate.verify_blob(gate.PATH,self.data,self.row)
    def test_native_device_sequence(self):
        row=next(x for x in json.loads((root/gate.MANIFEST).read_text())['animations'] if x['path']==gate.NATIVE_PATH)
        data=(root/gate.NATIVE_PATH).read_bytes();gate.verify_blob(gate.NATIVE_PATH,data,row)
        for key,value in [('width',512),('duration_ms',4200),('frames',9),('captured_on','2026-10-21')]:
            bad=copy.deepcopy(row);bad[key]=value
            with self.assertRaises(gate.Fail):gate.verify_blob(gate.NATIVE_PATH,data,bad)
        for at in (0,100,len(data)-1):
            bad=bytearray(data);bad[at]^=1
            with self.assertRaises(gate.Fail):gate.verify_blob(gate.NATIVE_PATH,bytes(bad),row)
    def test_pixel_payload_tamper(self):
        for index in (0,100,len(self.data)-1):
            with self.subTest(index=index):
                bad=bytearray(self.data);bad[index]^=1
                with self.assertRaises(gate.Fail):gate.verify_blob(gate.PATH,bytes(bad),self.row)
    def test_registration_and_provenance(self):
        with self.assertRaises(gate.Fail):gate.verify_blob('other.gif',self.data,self.row)
        for key,value in [('sha256','0'*64),('frames',50),('duration_ms',True),('input','human-confirmed'),('source_revision','short')]:
            with self.subTest(key=key):
                row=copy.deepcopy(self.row);row[key]=value
                with self.assertRaises(gate.Fail):gate.verify_blob(gate.PATH,self.data,row)
if __name__=='__main__':unittest.main()
