# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
#!/usr/bin/env python3
"""Generate original mouse-replay input DATA (timestamp semantics unverified)."""
import argparse
import hashlib
import json
import os
import sys

MAX_TS = 2**63 - 1
MAX_REC_BYTES = 4096


def parse_args(argv):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--output", required=True)
    g = p.add_mutually_exclusive_group(required=True)
    g.add_argument("--start", type=int)
    g.add_argument("--paused-recording")
    p.add_argument("--offset", type=int)
    p.add_argument("--gap", type=int, required=True)
    p.add_argument("--case", required=True, choices=["single-x", "slow-x"])
    a = p.parse_args(argv)
    if a.paused_recording is None:
        if a.offset is not None:
            p.error("--offset is only valid with --paused-recording")
    elif a.offset is None or a.offset <= 0:
        p.error("--offset is required and must be > 0 with --paused-recording")
    return a


def build(case, start, gap):
    count = 1 if case == "single-x" else 8
    events = []
    for i in range(count):
        events.append([start + i * gap,
                       {"MouseUpdateRelative": {"relx": 1, "rely": 0, "btn": None}}])
    return events


def reject_constant(_):
    raise ValueError("non-finite JSON constant")


def load_anchor(path):
    """Return (anchor timestamp, marker count, sha256) or exit."""
    try:
        with open(path, "rb") as f:
            raw = f.read(MAX_REC_BYTES + 1)
    except OSError as e:
        sys.exit("error: cannot read paused recording: %s" % e.__class__.__name__)
    if not 1 <= len(raw) <= MAX_REC_BYTES:
        sys.exit("error: paused recording must be 1..4096 bytes")
    try:
        rec = json.loads(raw.decode("utf-8"), parse_constant=reject_constant)
    except (ValueError, RecursionError):
        sys.exit("error: paused recording is not valid UTF-8 JSON")
    if not isinstance(rec, list) or not rec:
        sys.exit("error: paused recording must be a nonempty array")
    anchor = None
    for item in rec:
        if not isinstance(item, list) or len(item) != 2:
            sys.exit("error: each marker must be [timestamp,payload]")
        ts, payload = item
        if type(ts) is not int or not 0 <= ts <= MAX_TS:
            sys.exit("error: marker timestamp must be an int in 0..2**63-1")
        if anchor is None:
            anchor = ts
        elif ts != anchor:
            sys.exit("error: marker timestamps must all be identical")
        if not isinstance(payload, dict) or len(payload) != 1:
            sys.exit("error: marker payload must be a one-key object")
        kind = next(iter(payload))
        if not isinstance(kind, str) or not kind:
            sys.exit("error: marker kind must be a nonempty string")
        if kind.lower().startswith("mouse"):
            sys.exit("error: mouse markers are not allowed in recording")
    return anchor, len(rec), hashlib.sha256(raw).hexdigest()


def main(argv=None):
    a = parse_args(argv)
    paused = a.paused_recording is not None
    if paused:
        anchor, markers, rec_sha = load_anchor(a.paused_recording)
        start = anchor + a.offset
        if start > MAX_TS:
            sys.exit("error: anchor plus offset exceeds 2**63-1")
    else:
        start = a.start
    if start < 0:
        sys.exit("error: --start must be >= 0")
    if a.gap <= 0:
        sys.exit("error: --gap must be > 0")
    count = 1 if a.case == "single-x" else 8
    if start + (count - 1) * a.gap > MAX_TS:
        sys.exit("error: last timestamp exceeds 2**63-1")
    out = os.path.abspath(a.output)
    parent = os.path.dirname(out)
    if not os.path.isdir(parent):
        sys.exit("error: output parent directory does not exist")
    if os.path.lexists(out):
        sys.exit("error: output already exists; refusing to overwrite")
    events = build(a.case, start, a.gap)
    data = (json.dumps(events, separators=(",", ":")) + "\n").encode("utf-8")
    try:
        with open(out, "xb") as f:
            f.write(data)
    except FileExistsError:
        sys.exit("error: output already exists; refusing to overwrite")
    receipt = {
        "case": a.case,
        "start": start,
        "gap": a.gap,
        "event_count": len(events),
        "last_timestamp": events[-1][0],
        "sha256": hashlib.sha256(data).hexdigest(),
        "bytes": len(data),
        "original_data": True,
        "timestamp_semantics_verified": False,
        "tests_executed": [],
    }
    if paused:
        receipt["paused_clock_anchor"] = anchor
        receipt["paused_recording_sha256"] = rec_sha
        receipt["paused_marker_count"] = markers
        receipt["offset"] = a.offset
    print(json.dumps(receipt, separators=(",", ":")))
    return 0


if __name__ == "__main__":
    sys.exit(main())
