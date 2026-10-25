# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright 2026 Dean Howell. License: GPL-3.0-or-later.
import importlib.util, json, os, subprocess, tempfile, unittest
from unittest import mock

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location("phase", os.path.join(HERE, "..", "scripts", "Observe-InputPhase.py"))
phase = importlib.util.module_from_spec(spec)
spec.loader.exec_module(phase)
UUID = "01234567-89ab-cdef-0123-456789abcdef"


class Clock:
    def __init__(self):
        self.t = 0.0

    def __call__(self):
        return self.t

    def sleep(self, s):
        self.t += s


class Fake:
    def __init__(self, cfg, states=("running",), fail_snap=False, name=None, cleanup_uuid=None, cleanup_err=False):
        self.cfg, self.states, self.fail_snap, self.name = cfg, list(states), fail_snap, name
        self.cleanup_uuid, self.cleanup_err, self.calls = cleanup_uuid, cleanup_err, []

    def info(self, uuid):
        self.calls.append("info")
        if len(self.calls) > 1 and self.cleanup_err:
            raise RuntimeError("api down")
        st = self.states.pop(0) if len(self.states) > 1 else self.states[0]
        u = self.cleanup_uuid if self.cleanup_uuid and self.calls.count("info") > 1 else uuid
        return {"uuid": u, "name": self.name or "ClasSICK-097-a", "state": st, "cfg_file": self.cfg}

    def snapshot(self, uuid, path):
        self.calls.append("snapshot")
        if self.fail_snap:
            raise RuntimeError("snap failed")
        with open(path, "wb") as f:
            f.write(b"placeholder")

    def pause(self, uuid):
        self.calls.append("pause")
        self.states = ["paused"]


class T(unittest.TestCase):
    def setUp(self):
        self.td = tempfile.TemporaryDirectory()
        self.addCleanup(self.td.cleanup)
        self.root = os.path.realpath(self.td.name)
        self.cfg = os.path.join(self.root, "vm.vbox")
        with open(self.cfg, "w") as f:
            f.write("<VirtualBox/>")
        self.rec = {"uuid": UUID, "name": "ClasSICK-097-a", "trial_root": self.root}
        self.out = os.path.join(self.root, "out")
        self.clk = Clock()

    def run_it(self, fake, seconds=5, out=None):
        return phase.run_phase(self.rec, out or self.out, seconds, fake, self.clk, self.clk.sleep)

    def test_normal_deadline_single_pause(self):
        f = Fake(self.cfg)
        m = self.run_it(f)
        self.assertEqual(f.calls.count("pause"), 1)
        self.assertEqual((m["exit_code"], m["cleanup"]), (0, "paused"))
        self.assertTrue(0 < m["capture_count"] <= 15)
        self.assertTrue(os.path.exists(os.path.join(self.out, "manifest.json")))

    def test_refusals_make_no_calls_beyond_info(self):
        outside = tempfile.NamedTemporaryFile(suffix=".vbox", delete=False)
        outside.close()
        self.addCleanup(os.unlink, outside.name)
        for f in (Fake(self.cfg, ["paused"]), Fake(self.cfg, name="ClasSICK-097-b"),
                  Fake(os.path.join(self.root, "none.vbox")), Fake(outside.name)):
            with self.assertRaises(phase.PhaseError):
                self.run_it(f)
            self.assertEqual(f.calls, ["info"])
            self.assertFalse(os.path.exists(self.out))

    def test_first_snapshot_failure_still_pauses(self):
        f = Fake(self.cfg, fail_snap=True)
        m = self.run_it(f)
        self.assertEqual(f.calls, ["info", "snapshot", "info", "pause", "info"])
        self.assertEqual(m["exit_code"], 1)
        self.assertIn("snap failed", m["error"])

    def test_cleanup_identity_change_never_pauses(self):
        f = Fake(self.cfg, cleanup_uuid="11111111-89ab-cdef-0123-456789abcdef")
        m = self.run_it(f)
        self.assertNotIn("pause", f.calls)
        self.assertTrue(m["cleanup"].startswith("failed"))
        self.assertEqual(m["exit_code"], 1)

    def test_already_paused_no_duplicate(self):
        f = Fake(self.cfg, ["running", "paused"])
        m = self.run_it(f)
        self.assertNotIn("pause", f.calls)
        self.assertEqual((m["cleanup"], m["exit_code"]), ("already_paused", 0))

    def test_cleanup_error_retained(self):
        f = Fake(self.cfg, cleanup_err=True)
        m = self.run_it(f)
        self.assertIn("api down", m["cleanup"])
        self.assertEqual(m["exit_code"], 1)

    def test_invalid_inputs_precede_backend(self):
        link = os.path.join(self.root, "link")
        try:
            os.symlink(self.root, link, target_is_directory=True)
        except (OSError, NotImplementedError):
            link = None
        os.mkdir(os.path.join(self.root, "exists"))
        cases = [("5", self.out), (True, self.out), (4, self.out), (56, self.out),
                 (5, os.path.join(self.root, "exists")), (5, "relative"),
                 (5, os.path.join(os.path.dirname(self.root), "elsewhere"))]
        if link:
            cases.append((5, os.path.join(link, "o2")))
        for secs, out in cases:
            f = Fake(self.cfg)
            with self.assertRaises(phase.PhaseError):
                self.run_it(f, secs, out)
            self.assertEqual(f.calls, [])

    def test_mkdir_failure_still_pauses(self):
        f = Fake(self.cfg)
        with mock.patch.object(phase.os, "mkdir", side_effect=OSError("disk unavailable")):
            m = self.run_it(f)
        self.assertEqual(m["exit_code"], 1)
        self.assertIn("disk unavailable", m["error"])
        self.assertEqual(f.calls, ["info", "info", "pause", "info"])
        self.assertFalse(os.path.exists(self.out))

    def test_pause_must_be_observed(self):
        f = Fake(self.cfg)
        f.pause = lambda uuid: f.calls.append("pause")
        m = self.run_it(f)
        self.assertEqual(m["exit_code"], 1)
        self.assertIn("pause not observed", m["cleanup"])

    def test_manifest_contains_exit_status(self):
        m = self.run_it(Fake(self.cfg))
        with open(os.path.join(self.out, "manifest.json")) as stream:
            self.assertEqual(json.load(stream)["exit_code"], m["exit_code"])

    def test_cli_dry_run_no_subprocess(self):
        rp = os.path.join(self.root, "prepared-record.json")
        with open(rp, "w") as f:
            json.dump(self.rec, f)
        with mock.patch.object(subprocess, "run") as r, mock.patch.object(subprocess, "Popen") as p:
            rc = phase.main(["--record", rp, "--output", self.out], backend_factory=lambda: self.fail("backend"))
        self.assertEqual(rc, 0)
        r.assert_not_called()
        p.assert_not_called()
        self.assertFalse(os.path.exists(self.out))


if __name__ == "__main__":
    unittest.main()
