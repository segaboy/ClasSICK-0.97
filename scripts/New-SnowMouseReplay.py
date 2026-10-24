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


def parse_args(argv):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--output", required=True)
    p.add_argument("--start", type=int, required=True)
    p.add_argument("--gap", type=int, required=True)
    p.add_argument("--case", required=True, choices=["single-x", "slow-x"])
    return p.parse_args(argv)


def build(case, start, gap):
    count = 1 if case == "single-x" else 8
    events = []
    for i in range(count):
        events.append([start + i * gap,
                       {"MouseUpdateRelative": {"relx": 1, "rely": 0, "btn": None}}])
    return events


def main(argv=None):
    a = parse_args(argv)
    if a.start < 0:
        sys.exit("error: --start must be >= 0")
    if a.gap <= 0:
        sys.exit("error: --gap must be > 0")
    count = 1 if a.case == "single-x" else 8
    if a.start + (count - 1) * a.gap > MAX_TS:
        sys.exit("error: last timestamp exceeds 2**63-1")
    out = os.path.abspath(a.output)
    parent = os.path.dirname(out)
    if not os.path.isdir(parent):
        sys.exit("error: output parent directory does not exist")
    if os.path.lexists(out):
        sys.exit("error: output already exists; refusing to overwrite")
    events = build(a.case, a.start, a.gap)
    data = (json.dumps(events, separators=(",", ":")) + "\n").encode("utf-8")
    try:
        with open(out, "xb") as f:
            f.write(data)
    except FileExistsError:
        sys.exit("error: output already exists; refusing to overwrite")
    receipt = {
        "case": a.case,
        "start": a.start,
        "gap": a.gap,
        "event_count": len(events),
        "last_timestamp": events[-1][0],
        "sha256": hashlib.sha256(data).hexdigest(),
        "bytes": len(data),
        "original_data": True,
        "timestamp_semantics_verified": False,
        "tests_executed": [],
    }
    print(json.dumps(receipt, separators=(",", ":")))
    return 0


if __name__ == "__main__":
    sys.exit(main())
