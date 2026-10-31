#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Decode own CSPTIM01 registers; caller separately verifies the exact spin."""
import argparse
import json
import re
import sys

MAGIC = 0x43535054494D3031
REG = re.compile(r"\s*(r(?:8|9|1[0-5]))\s*(?:[=:]\s*|\s+)(?:0x)?([0-9a-f]{1,16})\s*$", re.I)
KNOWN = re.compile(r"\s*r(?:8|9|1[0-5])\b", re.I)


def decode(text):
    regs = {}
    for line in text.splitlines():
        if not KNOWN.match(line):
            continue
        m = REG.fullmatch(line)
        if not m or m.group(1).lower() in regs:
            raise ValueError("malformed or duplicate register")
        regs[m.group(1).lower()] = int(m.group(2), 16)
    if set(regs) != {f"r{i}" for i in range(8, 16)}:
        raise ValueError("missing registers")
    if regs["r8"] != MAGIC:
        raise ValueError("wrong timeline magic")
    h = regs["r9"]
    count, gaps, flags, reason = h & 255, (h >> 8) & 255, (h >> 16) & 255, (h >> 24) & 255
    x, y = (h >> 32) & 65535, (h >> 48) & 65535
    if count > 6 or flags & ~3 or reason not in (1, 2, 3):
        raise ValueError("invalid header")
    if (reason == 1) != (count == 6):
        raise ValueError("inconsistent stop reason")
    if (x != 65535 and x > 511) or (y != 65535 and y > 341):
        raise ValueError("invalid pointer")
    entries = []
    names = ("byte", "used_before", "used_after", "delivery_kind", "error_changed",
             "session_phase", "owner_phase", "owner_stage")
    for i in range(6):
        word = regs[f"r{i+10}"]
        if i >= count:
            if word:
                raise ValueError("nonzero unused entry")
            continue
        values = [(word >> (8 * j)) & 255 for j in range(8)]
        if values[4] > 1:
            raise ValueError("invalid error-change flag")
        entries.append(dict(zip(names, values)))
    return {"magic": "CSPTIM01", "count": count, "gaps": gaps,
            "unreliable": bool(flags & 1), "ready_baseline": bool(flags & 2),
            "stop_reason": {1: "full", 2: "drained-marker", 3: "session-fault"}[reason],
            "pointer": [None if x == 65535 else x, None if y == 65535 else y],
            "entries": entries,
            "scope": "Observed decoder state only; neither stream alignment nor host delivery is proven."}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("registers")
    a = p.parse_args()
    try:
        with open(a.registers, encoding="utf-8-sig") as f:
            result = decode(f.read())
    except (OSError, ValueError) as exc:
        print(str(exc), file=sys.stderr)
        return 2
    print(json.dumps(result, indent=2))
    return 0


if __name__ == "__main__":
    sys.exit(main())
