# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
import importlib.util, json, os, subprocess, sys, tempfile, unittest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCRIPT = os.path.join(ROOT, "scripts", "Read-MacPhaseWitness.py")
spec = importlib.util.spec_from_file_location("rmpw", SCRIPT)
R = importlib.util.module_from_spec(spec)
spec.loader.exec_module(R)

NAMES = ("tag version bytes source_irq vbl scc combined services baselines losses "
         "queued peak deliveries pointer_x pointer_y window_x window_y pattern phase "
         "motion_stage raw_a raw_b buttons motion_cause cause loss_reason loss_rr3 "
         "loss_snap0 loss_snap1 loss_snap2 loss_previous loss_phase").split()


def words(**kw):
    w = [0] * 32
    w[0], w[1], w[2] = 0x4D575031, 3, 128
    for k, v in kw.items():
        w[NAMES.index(k)] = v
    return ["%08X" % (x & 0xFFFFFFFF) for x in w]


def dec(**kw):
    return R.decode({"words": words(**kw)})


def prev(x, y, b=0):
    return 0x81000000 | x << 28 | y << 26 | (x & 1) << 16 | (y & 1) << 8 | b


FROZEN = [0x81000000, 0x85000100, 0x89000000, 0x8D000100, 0x91010000, 0x95010100,
          0x99010000, 0x9D010100, 0xA1000000, 0xA5000100, 0xA9000000, 0xAD000100,
          0xB1010000, 0xB5010100, 0xB9010000, 0xBD010100]


class Witness(unittest.TestCase):
    def refused(self, obj):
        with self.assertRaises(R.DecodeError):
            R.decode(obj)

    def test_frozen_words_and_all_pairs(self):
        for x in range(4):
            for y in range(4):
                for b in (0, 1):
                    p = prev(x, y, b)
                    self.assertEqual(p, FROZEN[x * 4 + y] + b)
                    r = dec(loss_reason=2, loss_previous=p, loss_snap1=0xAF4C4C)
                    self.assertEqual(r["previous"], {"known": True, "x_phase": x, "y_phase": y})
                    self.assertIsNone(r["consistent_with_gray_loss"])
                    self.assertEqual(r["current"], {"x_phase": 1, "y_phase": 3})

    def test_four_gray_cases(self):
        cases = [((0, 0), ["y"]), ((2, 0), ["x", "y"]), ((0, 2), []), ((2, 2), ["x"])]
        for (x, y), axes in cases:
            r = dec(loss_reason=6, loss_previous=prev(x, y), loss_snap1=0xAF4C4C)
            self.assertEqual(r["format"], "MWP1-v3-phase-witness")
            self.assertEqual(r["model_gray_jump_axes"], axes)
            self.assertIs(r["consistent_with_gray_loss"], bool(axes))
            self.assertEqual(r["current"], {"x_phase": 1, "y_phase": 3})

    def test_unknown_predecessor(self):
        r = dec(loss_reason=6, loss_previous=0x80000000, loss_snap1=0xAF4C4C)
        self.assertEqual(r["previous"], {"known": False, "x_phase": None, "y_phase": None})
        self.assertIsNone(r["consistent_with_gray_loss"])
        self.assertIsNone(r["model_gray_jump_axes"])
        r = dec(loss_reason=6, loss_previous=0x80010101, loss_snap1=0xAF4C4C)
        self.assertEqual(r["previous"], {"known": False, "x_phase": None, "y_phase": None})

    def test_no_loss(self):
        r = dec()
        self.assertFalse(r["previous"]["known"])
        self.assertIsNone(r["current"])
        self.assertIsNone(r["model_gray_jump_axes"])
        self.assertIsNone(r["consistent_with_gray_loss"])

    def test_mapping_and_signed(self):
        kw = {n: i + 1 for i, n in enumerate(NAMES) if 3 <= i <= 24}
        r = dec(**kw)
        self.assertEqual(sorted(r["mapping"]), sorted(NAMES))
        for i, n in enumerate(NAMES):
            if 3 <= i <= 24:
                self.assertEqual(r["mapping"][n], i + 1)
        r = dec(pointer_x=0xFFFFFFFF, window_y=0x80000000, motion_cause=-5, cause=-1)
        self.assertEqual(r["mapping"]["pointer_x"], -1)
        self.assertEqual(r["mapping"]["window_y"], -2 ** 31)
        self.assertEqual(r["mapping"]["motion_cause"], -5)
        self.assertEqual(r["mapping"]["cause"], -1)
        self.assertEqual(r["mapping"]["tag"], 0x4D575031)
        fields = ("pointer_x", "pointer_y", "window_x", "window_y", "motion_cause", "cause")
        r = dec(**{n: 0xFFFFFFFF for n in fields})
        for n in fields:
            self.assertEqual(r["mapping"][n], -1)

    def test_prefix_accepted(self):
        w = words()
        w[3] = "0x00000007"
        self.assertEqual(R.decode({"words": w})["mapping"]["source_irq"], 7)
        q = words(loss_reason=6, loss_previous=0xBD010101, loss_snap1=0xAF4C4C)
        self.assertEqual(R.decode({"words": [x.lower() for x in q]})["previous"],
                         {"known": True, "x_phase": 3, "y_phase": 3})

    def test_previous_word_refusals(self):
        bad = [(2, 0x01000000), (2, 0xC1000000), (2, 0x83000000), (2, 0x81000002),
               (2, 0x81010000), (2, 0x81000100), (2, 0xA0000000), (2, 0x90000000),
               (2, 0x40000000), (6, 0x01010101)]
        for reason, p in bad:
            with self.subTest(p=hex(p)):
                self.refused({"words": words(loss_reason=reason, loss_previous=p)})

    def test_header_refusals(self):
        for kw in ({"tag": 0x4D575032}, {"version": 2}, {"version": 4}, {"bytes": 127}):
            self.refused({"words": words(**kw)})

    def test_malformed(self):
        self.refused({"words": words(loss_previous=0x80000000)})
        w = words()
        bad = [tuple(w), [True] * 32, [0.0] * 32, [], w[:31], w + ["00000000"], "x", None, [1] * 32,
               w[:3] + ["GGGGGGGG"] + w[4:], w[:3] + ["1234567"] + w[4:],
               w[:3] + ["123456789"] + w[4:], w[:3] + ["-0000001"] + w[4:]]
        for b in bad:
            self.refused({"words": b})
        for o in ([], None, {}, {"words": w, "extra": 1}, {"word": w}):
            self.refused(o)

    def run_cli(self, text):
        fd, path = tempfile.mkstemp(suffix=".json")
        try:
            with os.fdopen(fd, "w") as f:
                f.write(text)
            return subprocess.run([sys.executable, SCRIPT, path], capture_output=True,
                                  timeout=20).returncode
        finally:
            os.remove(path)

    def test_cli(self):
        self.assertEqual(self.run_cli(json.dumps({"words": words()})), 0)
        self.assertEqual(self.run_cli(json.dumps({"words": words(version=2)})), 2)
        self.assertEqual(self.run_cli('{"words": [], "words": []}'), 2)
        dup = json.dumps({"words": words()})[:-1] + ', "words": []}'
        self.assertEqual(self.run_cli(dup), 2)
        self.assertEqual(self.run_cli("not json"), 2)


if __name__ == "__main__":
    unittest.main()
