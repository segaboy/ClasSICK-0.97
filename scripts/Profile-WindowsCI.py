#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell
"""Profile complete CTest groups in a bounded slice of a raw GitHub Actions job log.

The report restates what the log says. Unattributed seconds cover configure,
build, object audits and all other work outside reported CTest wall time; they
are not proven waste. No speedup, quota or compatibility claim is made.
Exit status: 0 all profiles passed, 1 nonpassing evidence reported, 2 refused.
"""
import argparse
import hashlib
import json
import os
import re
import sys
from datetime import datetime, timezone
from decimal import Decimal

SCHEMA_VERSION = 1
TOP_CASES = 10
TICKS = 10_000_000
STAMP = re.compile(r"(\d{4})-(\d\d)-(\d\d)T(\d\d):(\d\d):(\d\d)(?:\.(\d{1,7}))?Z")
LINE = re.compile(r"^(?:[^\t]*\t[^\t]*\t)?\ufeff?(\d{4}-\d\d-\d\dT[0-9:.]+Z)(?: (.*))?$")
PROJECT = re.compile(r"^Test project (\S.*)$")
CASE = re.compile(
    r"^(\d+)/(\d+) Test +#(\d+): (\S+) \.*\s*(\*{0,3}[A-Za-z].*?)\s+(\d+(?:\.\d+)?) sec$")
SUMMARY = re.compile(r"^(\d{1,3})% tests passed(?:, (\d+) tests? failed)? out of (\d+)$")
TOTAL = re.compile(r"^Total Test time \(real\) =\s*(\d+(?:\.\d+)?) sec$")
SKIPPED = ("Skipped", "Not Run (Disabled)")
NOTES = [
    "summed_case_seconds adds per-case durations; under parallel CTest it may exceed "
    "reported_real_seconds and is not wall time.",
    "unattributed_seconds = selected interval minus reported CTest real time; it includes "
    "configure, build, object audits and other step work and is not proven waste.",
    "No speedup, duplicate-check, quota or historical compatibility claim is made.",
]


class Refusal(Exception):
    """Input cannot support a trustworthy profile."""


def parse_ticks(text):
    m = STAMP.fullmatch(text or "")
    if not m:
        raise Refusal(f"invalid UTC timestamp {text!r}; expected YYYY-MM-DDTHH:MM:SS[.fffffff]Z")
    try:
        dt = datetime(*(int(g) for g in m.groups()[:6]), tzinfo=timezone.utc)
    except ValueError as exc:
        raise Refusal(f"invalid UTC timestamp {text!r}: {exc}") from None
    return int(dt.timestamp()) * TICKS + int((m.group(7) or "").ljust(7, "0"))


def utc(ticks):
    whole = datetime.fromtimestamp(ticks // TICKS, timezone.utc)
    return whole.strftime("%Y-%m-%dT%H:%M:%S") + f".{ticks % TICKS:07d}Z"


def seconds(ticks):
    return float(Decimal(ticks) / TICKS)


def path_key(path):
    return path.replace("\\", "/").rstrip("/").casefold()


def profile_name(path):
    norm = path.replace("\\", "/").rstrip("/")
    at = norm.casefold().rfind("/build/")
    return norm[at + 7:] if at >= 0 else norm.rsplit("/", 1)[-1]


def evidence(c):
    return {"number": c["number"], "name": c["name"], "outcome": c["outcome"],
            "seconds": float(c["seconds"]), "line": c["line"], "utc": utc(c["ticks"]),
            "text": c["text"]}


def close_profile(cur, lineno, ticks, real):
    pct, failed_text, expected = cur["summary"]
    where = f"profile {cur['path']!r} opened at line {cur['open_line']}"
    cases = cur["cases"]
    if expected == 0:
        raise Refusal(f"{where}: summary reports no executed cases")
    if len(cases) != expected:
        raise Refusal(f"{where}: {len(cases)} case lines but summary says out of {expected}")
    indexes = sorted(c["index"] for c in cases)
    if any(c["of"] != expected for c in cases) or indexes != list(range(1, expected + 1)):
        raise Refusal(f"{where}: case completion indexes do not form 1..{expected}")
    failed = [c for c in cases if c["outcome"] != "Passed" and c["outcome"] not in SKIPPED]
    skipped = [c for c in cases if c["outcome"] in SKIPPED]
    nonpassing = [c for c in cases if c["outcome"] != "Passed"]
    reported = int(failed_text) if failed_text is not None else (0 if pct == 100 else None)
    if reported != len(failed) or (pct == 100) != (not failed):
        raise Refusal(f"{where}: summary failure count disagrees with {len(failed)} failed case lines")
    identity = "".join(f"{n}\t{o}\n" for n, o in sorted((c["name"], c["outcome"]) for c in cases))
    return {
        "name": profile_name(cur["path"]),
        "path": cur["path"],
        "path_key": path_key(cur["path"]),
        "status": "nonpassing" if nonpassing else "pass",
        "open_line": cur["open_line"],
        "close_line": lineno,
        "open_utc": utc(cur["open"]),
        "close_utc": utc(ticks),
        "log_span_seconds": seconds(ticks - cur["open"]),
        "reported_real_seconds": float(real),
        "summed_case_seconds": float(sum((c["seconds"] for c in cases), Decimal(0))),
        "counts": {"expected": expected, "executed": len(cases) - len([c for c in cases if c["outcome"] in SKIPPED or c["outcome"].startswith("Not Run")]),
                   "passed": len(cases) - len(nonpassing), "failed": len(failed),
                   "skipped_or_not_run": len(skipped)},
        "identity_sha256": hashlib.sha256(identity.encode("utf-8")).hexdigest(),
        "nonpassing_cases": [evidence(c) for c in nonpassing],
        "_real": real,
        "_cases": cases,
    }


def analyze(data, start, end):
    profiles, cur, prev, selected = [], None, None, 0
    for lineno, line in enumerate(data.decode("utf-8", "replace").splitlines(), 1):
        m = LINE.match(line)
        if not m:
            continue
        ticks = parse_ticks(m.group(1))
        if prev is not None and ticks < prev:
            raise Refusal(f"line {lineno}: timestamp {m.group(1)} precedes previous line")
        prev = ticks
        if not start <= ticks < end:
            continue
        selected += 1
        body = (m.group(2) or "").strip()
        if mm := PROJECT.match(body):
            if cur:
                raise Refusal(f"line {lineno}: Test project before profile at line "
                              f"{cur['open_line']} completed")
            cur = {"path": mm.group(1), "open_line": lineno, "open": ticks,
                   "cases": [], "summary": None}
        elif mm := CASE.match(body):
            if not cur or cur["summary"]:
                raise Refusal(f"line {lineno}: test case line outside an open profile")
            name, number = mm.group(4), int(mm.group(3))
            if any(c["name"] == name or c["number"] == number for c in cur["cases"]):
                raise Refusal(f"line {lineno}: duplicate test case {name!r} #{number} "
                              f"in profile {cur['path']!r}")
            cur["cases"].append({
                "index": int(mm.group(1)), "of": int(mm.group(2)), "number": number,
                "name": name, "outcome": mm.group(5).lstrip("*").strip(),
                "seconds": Decimal(mm.group(6)), "line": lineno, "ticks": ticks, "text": body})
        elif mm := SUMMARY.match(body):
            if not cur or cur["summary"]:
                raise Refusal(f"line {lineno}: CTest summary outside an open profile")
            cur["summary"] = (int(mm.group(1)), mm.group(2), int(mm.group(3)))
        elif mm := TOTAL.match(body):
            if not cur or not cur["summary"]:
                raise Refusal(f"line {lineno}: Total Test time without matching summary")
            profiles.append(close_profile(cur, lineno, ticks, Decimal(mm.group(1))))
            cur = None
    if not selected:
        raise Refusal("no timestamped lines inside the selected interval")
    if cur:
        raise Refusal(f"profile {cur['path']!r} opened at line {cur['open_line']} "
                      "is incomplete at end of selection")
    if not profiles:
        raise Refusal("no complete CTest profile inside the selected interval")

    seen, slow = {}, []
    for p in profiles:
        seen[p["path_key"]] = seen.get(p["path_key"], 0) + 1
        p["occurrence"] = seen[p["path_key"]]
        p["id"] = f"{p['name']}#{p['occurrence']}"
        slow += [{"profile": p["id"], **evidence(c)} for c in p.pop("_cases")]
    slow.sort(key=lambda c: (-c["seconds"], c["profile"], c["name"]))
    total_real = sum((p.pop("_real") for p in profiles), Decimal(0))
    interval = Decimal(end - start) / TICKS
    return {
        "schema_version": SCHEMA_VERSION,
        "source": {"sha256": hashlib.sha256(data).hexdigest(), "bytes": len(data)},
        "bounds": {"start": utc(start), "end": utc(end), "interval": "half-open [start, end)"},
        "status": "pass" if all(p["status"] == "pass" for p in profiles) else "nonpassing",
        "profiles": profiles,
        "top_slow_cases": slow[:TOP_CASES],
        "total_reported_ctest_wall_seconds": float(total_real),
        "selected_interval_seconds": float(interval),
        "unattributed_seconds": float(interval - total_real),
        "notes": NOTES,
    }


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--log", required=True, help="raw GH CLI job log")
    parser.add_argument("--start", required=True, help="inclusive UTC bound, ...Z")
    parser.add_argument("--end", required=True, help="exclusive UTC bound, ...Z")
    parser.add_argument("--output", required=True, help="new JSON report path")
    args = parser.parse_args(argv)
    try:
        start, end = parse_ticks(args.start), parse_ticks(args.end)
        if start >= end:
            raise Refusal("--start must be earlier than --end")
        if os.path.lexists(args.output):
            raise Refusal(f"output {args.output!r} already exists")
        with open(args.log, "rb") as fh:
            data = fh.read()
        report = analyze(data, start, end)
        with open(args.output, "x", encoding="utf-8", newline="\n") as fh:
            json.dump(report, fh, indent=2)
            fh.write("\n")
    except (Refusal, OSError) as exc:
        print(f"refused: {exc}", file=sys.stderr)
        return 2
    print(f"{report['status']}: {len(report['profiles'])} profiles, "
          f"{report['total_reported_ctest_wall_seconds']}s reported CTest real of "
          f"{report['selected_interval_seconds']}s selected")
    return 0 if report["status"] == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
