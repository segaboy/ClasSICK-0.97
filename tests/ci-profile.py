#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell
"""Synthetic tests for scripts/Profile-WindowsCI.py."""
import contextlib
import hashlib
import importlib.util
import io
import json
import pathlib
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "profile_windows_ci", ROOT / "scripts" / "Profile-WindowsCI.py")
MOD = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MOD)

PASS = [(1, "a.one", "Passed", "1.00"), (2, "a.two", "Passed", "0.50")]


def ts(n, frac="5000000"):
    return f"2026-10-27T21:{n // 60:02d}:{n % 60:02d}.{frac}Z"


def gh(n, text, step="UNKNOWN STEP"):
    return f"windows\t{step}\t{ts(n)} {text}"


def case(index, of, number, name, outcome, secs):
    mark = "   " if outcome == "Passed" else "***"
    return f"{index:>3}/{of} Test {'#' + str(number):>4}: {name} {'.' * 10}{mark}{outcome} {secs:>7} sec"


def profile(t0, path, cases, real, of=None, summary=None, close=True):
    of = len(cases) if of is None else of
    failed = sum(1 for c in cases if c[2] not in ("Passed", "Not Run (Disabled)"))
    if summary is None:
        pct = 100 if not failed else (100 * (of - failed)) // of
        summary = f"{pct}% tests passed, {failed} tests failed out of {of}"
    lines = [gh(t0, f"Test project {path}"), gh(t0, "    Start  1: setup")]
    lines += [gh(t0 + 1 + i, case(i + 1, of, *c)) for i, c in enumerate(cases)]
    end = t0 + 1 + len(cases)
    if summary:
        lines += [gh(end, ""), gh(end, summary)]
    if close:
        lines.append(gh(end, f"Total Test time (real) = {real:>6} sec"))
    return lines


class ProfileTests(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.dir = pathlib.Path(tmp.name)
        self.out = self.dir / "profile.json"

    def run_cli(self, lines, start=ts(0), end=ts(100)):
        log = self.dir / "job.log"
        log.write_text("\n".join(lines) + "\n", encoding="utf-8")
        err = io.StringIO()
        with contextlib.redirect_stderr(err), contextlib.redirect_stdout(io.StringIO()):
            code = MOD.main(["--log", str(log), "--start", start, "--end", end,
                             "--output", str(self.out)])
        self.stderr = err.getvalue()
        return code

    def assert_refused(self, lines, text, **bounds):
        self.assertEqual(self.run_cli(lines, **bounds), 2)
        self.assertIn(text, self.stderr)
        self.assertFalse(self.out.exists())

    def report(self):
        return json.loads(self.out.read_text(encoding="utf-8"))

    def test_two_profiles_math_and_noise(self):
        parallel = [(3, "b.three", "Passed", "2.00"), (1, "b.one", "Passed", "1.50"),
                    (2, "b.two", "Passed", "1.00")]
        lines = ["", "garbage without timestamp", "\ufeff",
                 gh(0, "-- Configuring done", step="Build and verify"),
                 f"windows\tUNKNOWN STEP\t{ts(5, '0000001')} noise",
                 *profile(10, "D:\\a\\ClasSICK\\build\\gcc-a", PASS, "3.00"),
                 gh(30, "Sanitizer detection validated: address"),
                 *profile(40, "D:/a/ClasSICK/build/clang/release", parallel, "2.00"),
                 *profile(100, "D:/a/ClasSICK/build/after", PASS, "1.00")]
        self.assertEqual(self.run_cli(lines), 0)
        r = self.report()
        self.assertEqual(r["schema_version"], 1)
        self.assertEqual([p["name"] for p in r["profiles"]], ["gcc-a", "clang/release"])
        b = r["profiles"][1]
        self.assertEqual(b["counts"], {"expected": 3, "executed": 3, "passed": 3,
                                       "failed": 0, "skipped_or_not_run": 0})
        self.assertEqual(b["summed_case_seconds"], 4.5)
        self.assertEqual(b["reported_real_seconds"], 2.0)
        self.assertEqual(b["status"], "pass")
        self.assertEqual(r["total_reported_ctest_wall_seconds"], 5.0)
        self.assertEqual(r["selected_interval_seconds"], 100.0)
        self.assertEqual(r["unattributed_seconds"], 95.0)
        self.assertEqual(b["identity_sha256"], hashlib.sha256(
            b"b.one\tPassed\nb.three\tPassed\nb.two\tPassed\n").hexdigest())

    def test_complete_failure_remains_nonpassing(self):
        bad = [(1, "crash", "Failed", "0.50")]
        self.assertEqual(self.run_cli(profile(10, "build/fail", bad, "1.00")), 1)
        r = self.report()
        self.assertEqual(r["status"], "nonpassing")
        self.assertEqual(r["profiles"][0]["counts"]["failed"], 1)
        self.assertEqual(r["profiles"][0]["nonpassing_cases"][0]["name"], "crash")

    def test_skip_does_not_count_as_executed_or_passing(self):
        skipped = [(1, "disabled", "Not Run (Disabled)", "0.00")]
        self.assertEqual(self.run_cli(profile(10, "build/skip", skipped, "1.00")), 1)
        self.assertEqual(self.report()["profiles"][0]["counts"]["executed"], 0)

    def test_incomplete_and_inconsistent_groups_refuse(self):
        base = profile(10, "build/a", PASS, "2.00")
        cases = [
            (base[:-1], "incomplete"),
            (base[:-2] + [base[-1]], "without matching summary"),
            (profile(10, "build/a", PASS[:1], "2.00", of=2), "case lines"),
            (profile(10, "build/a", [PASS[0], PASS[0]], "2.00"), "duplicate"),
            (profile(10, "build/a", PASS, "2.00", summary="50% tests passed, 1 tests failed out of 2"), "failure count"),
            (base[:2] + [gh(1, "backwards")] + base[2:], "precedes previous"),
        ]
        for lines, why in cases:
            with self.subTest(why=why):
                self.assert_refused(lines, why)

    def test_invalid_empty_bounds_and_no_groups(self):
        for start, end in [("bad", ts(10)), (ts(10), ts(10)), (ts(11), ts(10)),
                           ("2026-13-01T00:00:00Z", ts(10))]:
            with self.subTest(start=start, end=end):
                self.assert_refused([], "timestamp" if start in ["bad", "2026-13-01T00:00:00Z"] else "earlier", start=start, end=end)
        self.assert_refused([], "no timestamped lines")
        self.assert_refused([gh(5, "exit0 SUCCESS")], "no complete CTest profile")

    def test_existing_output_is_preserved(self):
        self.out.write_bytes(b"keep me")
        self.assertEqual(self.run_cli(profile(10, "build/a", PASS, "2.00")), 2)
        self.assertEqual(self.out.read_bytes(), b"keep me")

    def test_repeated_paths_are_distinct_complete_occurrences(self):
        lines = profile(10, "D:/build/a", PASS, "2.00") + profile(20, "D:\\build\\a", PASS, "2.00")
        self.assertEqual(self.run_cli(lines), 0)
        self.assertEqual([x["occurrence"] for x in self.report()["profiles"]], [1, 2])

if __name__ == "__main__":
    unittest.main()
