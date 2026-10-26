# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
import importlib.util
import json
import os
import subprocess
import sys
import tempfile
import unittest

_p = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                  "Read-MacRegisterTrace.py")
_s = importlib.util.spec_from_file_location("rmrt", _p)
m = importlib.util.module_from_spec(_s)
_s.loader.exec_module(m)


def good():
    d = {r: "%08X" % i for i, r in enumerate(m.REGS)}
    d["D0"] = "0x4D575231"
    return d


class T(unittest.TestCase):
    def test_frozen_complete_mapping(self):
        # Literal format fixture; not a claimed native/hardware observation.
        regs = {"D0":"4D575231","D1":"00000001","D2":"00000002",
                "D3":"00000003","D4":"00000004","D5":"00000005",
                "D6":"00000006","D7":"00000007","A0":"11223344",
                "A1":"55667788","A2":"99AABBCC","A3":"FFFFFFFF",
                "A4":"00000011","A5":"00000000","A6":"00000B78"}
        self.assertEqual(m.decode(regs), {"format":"MWR1",
            "address_registers_are_data":True,"mapping":{
                "magic":0x4D575231,"services":1,"losses":2,"deliveries":3,
                "loss_reason":4,"loss_rr3":5,"loss_previous":6,"loss_phase":7,
                "loss_snap0":0x11223344,"loss_snap1":0x55667788,
                "loss_snap2":0x99AABBCC,"pointer_x":-1,"pointer_y":17,
                "fault":0,"trace_address":2936}})

    def test_cli(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = os.path.join(tmp, "registers.json")
            valid = json.dumps(good())
            cases = [(valid, 0), (valid[:-1]+',"D1":"00000000"}', 1),
                     ('[]', 1), ('null', 1), ('{}', 1), ('{', 1),
                     (valid.replace('"00000003"', 'NaN'), 1),
                     (valid.replace('0x4D575231', '4D575232'), 1)]
            for text, expected in cases:
                with open(path, 'w', encoding='utf-8') as f:
                    f.write(text)
                p = subprocess.run([sys.executable,'-B',_p,path],
                    capture_output=True, timeout=10)
                self.assertEqual(p.returncode, expected, (text,p.stderr))
                if expected:
                    self.assertEqual(p.stdout, b'')
                else:
                    self.assertEqual(json.loads(p.stdout), m.decode(good()))
            p = subprocess.run([sys.executable,'-B',_p],capture_output=True,timeout=10)
            self.assertEqual(p.returncode, 2)
            p = subprocess.run([sys.executable,'-B',_p,path+'.missing'],
                capture_output=True,timeout=10)
            self.assertEqual(p.returncode, 1)

    def test_valid(self):
        d = good()
        d["A3"] = "FFFFFFFF"
        d["A4"] = "0x80000000"
        d["D1"] = "ffffffff"
        d["A0"] = "80000000"
        r = m.decode(d)
        self.assertEqual(r["format"], "MWR1")
        self.assertTrue(r["address_registers_are_data"])
        x = r["mapping"]
        self.assertEqual(x["pointer_x"], -1)
        self.assertEqual(x["pointer_y"], -2147483648)
        self.assertEqual(x["services"], 4294967295)
        self.assertEqual(x["loss_snap0"], 0x80000000)
        d["A3"] = "7FFFFFFF"
        self.assertEqual(m.decode(d)["mapping"]["pointer_x"], 2147483647)

    def test_missing_unknown(self):
        for r in m.REGS:
            d = good()
            del d[r]
            with self.assertRaises(m.DecodeError):
                m.decode(d)
        d = good()
        d["A7"] = "00000000"
        with self.assertRaises(m.DecodeError):
            m.decode(d)

    def test_malformed(self):
        for bad in ("1234567", "123456789", "0x1234567", "GGGGGGGG", "",
                    " 00000000", "00000000\n", "-0000001", 5, None, [], True):
            d = good()
            d["D3"] = bad
            with self.assertRaises(m.DecodeError):
                m.decode(d)
        with self.assertRaises(m.DecodeError):
            m.decode([])

    def test_magic(self):
        d = good()
        d["D0"] = "4D575232"
        with self.assertRaises(m.DecodeError):
            m.decode(d)

    def test_duplicate(self):
        text = json.dumps(good())[:-1] + ', "D1": "00000001"}'
        with self.assertRaises(m.DecodeError):
            json.loads(text, object_pairs_hook=m._no_dups)


if __name__ == "__main__":
    unittest.main()
