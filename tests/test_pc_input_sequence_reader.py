# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Literal-vector API tests for scripts/Read-PCInputSequence.py (SPEC-0085).

Register words and expectations are hand-derived from SPEC-0085 text, not
from the candidate reader.
"""
import contextlib
import importlib.util
import io
import json
import os
import tempfile
import unittest
from pathlib import Path

_PATH = Path(__file__).resolve().parents[1] / "scripts" / "Read-PCInputSequence.py"
_SPEC = importlib.util.spec_from_file_location("read_pc_input_sequence", _PATH)
rd = importlib.util.module_from_spec(_SPEC)
_SPEC.loader.exec_module(rd)

MAGIC = "4353505345513031"


def regs(r9, r10="0", r11="0", r12="0", r13="0", r14="0", r15="0", r8=MAGIC):
    vals = [r8, r9, r10, r11, r12, r13, r14, r15]
    return "\n".join(f"r{8 + i}=0x{v}" for i, v in enumerate(vals)) + "\n"


# 08 05 03 | 39 FF FE: packets (+5,+3,b0) and (-1,-2,b1); no flags, no gaps.
CLEAN = regs("0600000000000000", "0000FEFF39030508")
# 00E | C8 12 34E | 08 07E | 38 00 00 | 09 ; flags bits 0,3,5; count 10.
LOSSES = regs("0A00000000000029", "003807083412C800", "0000000000000900")
# 00 | 48 01 02 ; no error steps, gaps 3.
MISMATCH = regs("0403000000000000", "0000000002014800")


def names(d):
    return [e["event"] for e in d["events"]]


class ReplayVectors(unittest.TestCase):
    def test_clean_signed_packets(self):
        d = rd.decode(CLEAN)
        self.assertEqual(d["bytes"], [0x08, 0x05, 0x03, 0x39, 0xFF, 0xFE])
        self.assertEqual(names(d), ["header", "x", "packet", "header", "x", "packet"])
        self.assertEqual([e["kind"] for e in d["events"]], [0, 0, 1, 0, 0, 1])
        p1, p2 = d["events"][2], d["events"][5]
        self.assertEqual((p1["dx"], p1["dy"], p1["buttons"]), (5, 3, 0))
        self.assertEqual((p2["dx"], p2["dy"], p2["buttons"]), (-1, -2, 1))
        s = d["summary"]
        self.assertEqual((s["packets"], s["sum_dx"], s["sum_dy"]), (2, 4, 1))
        self.assertEqual((s["max_abs_dx"], s["max_abs_dy"]), (5, 3))
        self.assertEqual(s["button_states"], [0, 1])
        self.assertEqual((s["trailing_partial_bytes"], s["gaps"]), (0, 0))
        self.assertIs(s["alignment_qualified"], True)
        self.assertEqual((s["overflow_compatible"], s["unsynced_compatible"],
                          s["status_compatible"], s["model_mismatches"]), (0, 0, 0, 0))
        self.assertFalse(any("proportional" in n.lower() for n in s["notes"]))
        self.assertIn("scope", s)

    def test_compatible_losses_and_9bit_extreme(self):
        d = rd.decode(LOSSES)
        self.assertEqual(names(d), [
            "loss-unsynced-compatible", "header", "x", "loss-overflow-compatible",
            "header", "lost-status-compatible", "header", "x", "packet", "header"])
        self.assertEqual([e["kind"] for e in d["events"]], [2, 0, 0, 2, 0, 2, 0, 0, 1, 0])
        self.assertEqual([int(e["error"]) for e in d["events"]], [1, 0, 0, 1, 0, 1, 0, 0, 0, 0])
        ov = d["events"][3]
        self.assertEqual((ov["header"], ov["raw_x"], ov["raw_y"]), (0xC8, 0x12, 0x34))
        self.assertEqual((ov["x_overflow"], ov["y_overflow"]), (True, True))
        p = d["events"][8]
        self.assertEqual((p["dx"], p["dy"], p["buttons"]), (-256, -256, 0))
        s = d["summary"]
        self.assertEqual((s["packets"], s["overflow_compatible"], s["unsynced_compatible"],
                          s["status_compatible"], s["model_mismatches"]), (1, 1, 1, 1, 0))
        self.assertEqual(s["trailing_partial_bytes"], 1)
        self.assertEqual((s["max_abs_dx"], s["max_abs_dy"]), (256, 256))
        for key in s:
            self.assertNotIn("dropped", key)

    def test_unflagged_model_mismatch_with_gaps(self):
        d = rd.decode(MISMATCH)
        self.assertEqual(names(d), ["model-mismatch-drop-unsynced", "header", "x",
                                    "model-mismatch-drop-overflow"])
        self.assertEqual([e["kind"] for e in d["events"]], [2, 0, 0, 2])
        ev = d["events"][3]
        self.assertEqual((ev["header"], ev["raw_x"], ev["raw_y"]), (0x48, 0x01, 0x02))
        self.assertEqual((ev["x_overflow"], ev["y_overflow"]), (True, False))
        s = d["summary"]
        self.assertEqual((s["model_mismatches"], s["overflow_compatible"],
                          s["unsynced_compatible"], s["packets"]), (2, 0, 0, 0))
        self.assertEqual(s["gaps"], 3)
        self.assertIs(s["alignment_qualified"], False)
        self.assertTrue(any("alignment" in n.lower() for n in s["notes"]))

    def test_saturated_gaps_single_byte(self):
        d = rd.decode(regs("01FF000000000000", "0000000000000008"))
        s = d["summary"]
        self.assertEqual((d["count"], s["gaps"], s["trailing_partial_bytes"], s["packets"]), (1, 255, 1, 0))
        self.assertIs(s["alignment_qualified"], False)

    def test_full_48_bytes_last_word(self):
        words = [int(MAGIC, 16), 0x30 << 56] + [0x0808080808080808] * 6
        data, errs, gaps = rd.unpack(words)
        self.assertEqual((len(data), set(data), sum(errs), gaps), (48, {8}, 0, 0))


class Refusals(unittest.TestCase):
    def refused(self, text):
        with self.assertRaises(rd.FormatError):
            rd.decode(text)

    def test_record_limits(self):
        self.refused(regs("0600000000000000", "0000FEFF39030508", r8="4353505345513030"))
        self.refused(regs("0000000000000000"))
        self.refused(regs("3100000000000000"))
        self.refused(regs("0100000000000000", "0000000000000108"))
        self.refused(regs("0100000000000002", "0000000000000008"))
        self.refused(regs("0600000000000000", "0000FEFF39030508", r15="1"))

    def test_malformed_known_registers(self):
        good = "r10=0x0000FEFF39030508"
        for bad in ("r10=0xZZ", "r10=", "r10 = 0x", "r10=0x" + "1" * 17,
                    "r10=0x12 extra", "r10 -5", "R10"):
            with self.subTest(bad=bad):
                self.refused(CLEAN.replace(good, bad))
        self.refused(CLEAN + "r12=0\n")
        self.refused(CLEAN.replace("r15=0x0\n", ""))

    def test_accepted_forms_and_unrelated_lines(self):
        text = ("\npc 0x1234\nrip=5\nR8: 0x4353505345513031\nr9 0600000000000000\n"
                "r10=FEFF39030508\n  r11 : 0\nR12 0x0\nr13=0\nr14:0X0\nr15 0\n")
        self.assertEqual(rd.parse_registers(text),
                         [0x4353505345513031, 0x0600000000000000, 0xFEFF39030508, 0, 0, 0, 0, 0])

    def test_malformed_addition_to_complete_record(self):
        # A missing-register failure cannot detect silently ignored extra data.
        for extra in ("r8=ZZ\n", "r12\n", "R15=0x1 comment\n"):
            with self.subTest(extra=extra):
                self.refused(CLEAN + extra)

    def test_unpack_rejects_invalid_uint64_lists(self):
        base = [int(MAGIC, 16), 0x0600000000000000, 0x0000FEFF39030508, 0, 0, 0, 0, 0]
        self.assertEqual(rd.unpack(base)[0], [0x08, 0x05, 0x03, 0x39, 0xFF, 0xFE])
        for bad in (base[:7], base + [0], base[:3] + [True] + base[4:],
                    base[:3] + [-1] + base[4:], base[:3] + [1 << 64] + base[4:],
                    base[:3] + ["0"] + base[4:], base[:3] + [0.0] + base[4:], None):
            with self.subTest(bad=bad), self.assertRaises(rd.FormatError):
                rd.unpack(bad)


class Cli(unittest.TestCase):
    def run_cli(self, text, *extra):
        with tempfile.TemporaryDirectory() as tmp:
            path = os.path.join(tmp, "regs.txt")
            with open(path, "w", encoding="utf-8") as f:
                f.write(text)
            out, err = io.StringIO(), io.StringIO()
            with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
                rc = rd.main([path, *extra])
        return rc, out.getvalue(), err.getvalue()

    def test_json_success(self):
        rc, out, _ = self.run_cli(LOSSES, "--json")
        self.assertEqual(rc, 0)
        self.assertEqual(json.loads(out)["summary"]["status_compatible"], 1)

    def test_text_success_and_refusal(self):
        self.assertEqual(self.run_cli(CLEAN)[0], 0)
        rc, _, err = self.run_cli(CLEAN.replace("r9=", "r9=zz"))
        self.assertEqual(rc, 2)
        self.assertIn("refused", err)


if __name__ == "__main__":
    unittest.main()
