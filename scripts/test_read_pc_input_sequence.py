# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Reader conformance: the frozen readout and truth below were produced by
tests/pc-sequence-bus.c over the real bus and decoder. Set CLASSICK_SEQ_DIR to
a fresh directory written by that test to check a new run as well."""
import importlib.util
import os
import pathlib
import unittest

HERE = pathlib.Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("reader", HERE / "Read-PCInputSequence.py")
reader = importlib.util.module_from_spec(spec)
spec.loader.exec_module(reader)

REGS = """r8=4353505345513031
r9=3000000000080620
r10=f01800ff48030508
r11=7f28000009080102
r12=0800010801018881
r13=0001080001080001
r14=0108000108000108
r15=0800010800010800
"""
TRUTH = """08 0 0 0 0 0|05 0 0 0 0 0|03 1 5 3 0 0|48 0 0 0 0 0|ff 0 0 0 0 0|00 2 0 0 0 1|18 0 0 0 0 0|f0 0 0 0 0 0|02 1 -16 2 0 0|01 2 0 0 0 1|08 2 0 0 0 1|09 0 0 0 0 0|00 0 0 0 0 0|00 1 0 0 1 0|28 0 0 0 0 0|7f 0 0 0 0 0|81 1 127 -127 0 0|88 0 0 0 0 0|01 0 0 0 0 0|01 2 0 0 0 1|08 0 0 0 0 0|01 0 0 0 0 0|00 1 1 0 0 0|08 0 0 0 0 0|01 0 0 0 0 0|00 1 1 0 0 0|08 0 0 0 0 0|01 0 0 0 0 0|00 1 1 0 0 0|08 0 0 0 0 0|01 0 0 0 0 0|00 1 1 0 0 0|08 0 0 0 0 0|01 0 0 0 0 0|00 1 1 0 0 0|08 0 0 0 0 0|01 0 0 0 0 0|00 1 1 0 0 0|08 0 0 0 0 0|01 0 0 0 0 0|00 1 1 0 0 0|08 0 0 0 0 0|01 0 0 0 0 0|00 1 1 0 0 0|08 0 0 0 0 0|01 0 0 0 0 0|00 1 1 0 0 0|08 0 0 0 0 0"""


def truth_rows(text):
    rows = []
    for line in text.replace("|", "\n").split("\n"):
        if line.strip():
            b, kind, dx, dy, buttons, err = line.split()
            rows.append((int(b, 16), int(kind), int(dx), int(dy), int(buttons), int(err)))
    return rows


class Conformance(unittest.TestCase):
    def check(self, regs, truth):
        r = reader.decode(regs)
        rows = truth_rows(truth)
        self.assertEqual(len(rows), 48)
        for ev, (b, kind, dx, dy, buttons, err) in zip(r["events"], rows):
            self.assertEqual(ev["byte"], b)
            self.assertEqual(ev["error"], 1 if err else 0)
            self.assertEqual(ev["kind"], kind, ev)
            if kind == 1:
                self.assertEqual((ev["dx"], ev["dy"], ev["buttons"]), (dx, dy, buttons))
        return r

    def test_frozen(self):
        s = self.check(REGS, TRUTH)["summary"]
        self.assertEqual(s["packets"], 13)
        self.assertEqual(s["overflow_compatible"], 2)
        self.assertEqual(s["unsynced_compatible"], 1)
        self.assertEqual(s["status_compatible"], 1)
        self.assertEqual(s["model_mismatches"], 0)
        self.assertEqual(s["trailing_partial_bytes"], 1)
        self.assertEqual((s["max_abs_dx"], s["max_abs_dy"]), (127, 127))
        ov = [e for e in reader.decode(REGS)["events"] if e["event"] == "loss-overflow-compatible"]
        self.assertEqual([(e["x_overflow"], e["y_overflow"]) for e in ov], [(True, False), (False, True)])

    @unittest.skipUnless(os.environ.get("CLASSICK_SEQ_DIR"), "fresh run not provided")
    def test_fresh_run(self):
        d = pathlib.Path(os.environ["CLASSICK_SEQ_DIR"])
        self.check((d / "registers.txt").read_text(), (d / "truth.txt").read_text())


# Readout shape of the 3-byte raw-sequence-first build (tests/pc-sequence-short.c).
SHORT = "r8=4353505345513031\nr9=0300000000000002\nr10=0000000000cac9c8\n" + \
    "".join(f"r{n}=0\n" for n in range(11, 16))


class Short(unittest.TestCase):
    def test_short(self):
        r = reader.decode(SHORT)
        self.assertEqual(r["count"], 3)
        self.assertEqual([e["event"] for e in r["events"]], ["header", "lost-status-compatible", "header"])
        self.assertEqual(r["summary"]["trailing_partial_bytes"], 1)


class Refusals(unittest.TestCase):
    def test_formats(self):
        self.assertEqual(reader.parse_registers(REGS.replace("=", ": 0x")), reader.parse_registers(REGS))
        self.assertEqual(reader.parse_registers(REGS.upper().replace("=", " ")), reader.parse_registers(REGS))

    def test_refusals(self):
        for bad, why in [
            (REGS.replace("r15=0800010800010800\n", ""), "missing"),
            (REGS + "r9=0\n", "duplicate"),
            (REGS.replace("4353505345513031", "4353505250543031"), "magic"),
            (REGS.replace("r9=3000", "r9=3100"), "count"),
            (SHORT.replace("r10=0000000000cac9c8", "r10=0000000001cac9c8"), "beyond count"),
        ]:
            with self.assertRaises(reader.FormatError, msg=why) as cm:
                reader.decode(bad)
            self.assertIn(why, str(cm.exception))

    def test_mismatch_and_gaps_are_reported(self):
        # Clear byte 5's error flag: the model's overflow drop becomes a mismatch.
        r = reader.decode(REGS.replace("r9=3000000000080620", "r9=3001000000080600"))
        s = r["summary"]
        self.assertEqual(s["model_mismatches"], 1)
        self.assertEqual(s["gaps"], 1)
        self.assertTrue(any("gaps" in n for n in s["notes"]))
        self.assertTrue(any("mismatch" in n.lower() for n in s["notes"]))


if __name__ == "__main__":
    unittest.main()
