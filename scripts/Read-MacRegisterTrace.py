#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Strict decoder for the MWR1 register snapshot (diagnostic data only)."""
import json
import re
import sys

REGS = tuple("D%d" % i for i in range(8)) + tuple("A%d" % i for i in range(7))
MAGIC = 0x4D575231
NAMES = {
    "D0": "magic", "D1": "services", "D2": "losses", "D3": "deliveries",
    "D4": "loss_reason", "D5": "loss_rr3", "D6": "loss_previous",
    "D7": "loss_phase", "A0": "loss_snap0", "A1": "loss_snap1",
    "A2": "loss_snap2", "A3": "pointer_x", "A4": "pointer_y",
    "A5": "fault", "A6": "trace_address",
}
SIGNED = ("A3", "A4")
_HEX = re.compile(r"(?:0[xX])?([0-9A-Fa-f]{8})")


class DecodeError(ValueError):
    pass


def _word(name, value):
    if not isinstance(value, str):
        raise DecodeError("%s must be a string" % name)
    m = _HEX.fullmatch(value)
    if not m:
        raise DecodeError("%s must be exactly 8 hex digits" % name)
    return int(m.group(1), 16)


def decode(regs):
    if not isinstance(regs, dict):
        raise DecodeError("registers must be an object")
    extra = sorted(str(k) for k in regs if k not in REGS)
    if extra:
        raise DecodeError("unknown fields: " + ",".join(extra))
    missing = [r for r in REGS if r not in regs]
    if missing:
        raise DecodeError("missing fields: " + ",".join(missing))
    vals = {r: _word(r, regs[r]) for r in REGS}
    if vals["D0"] != MAGIC:
        raise DecodeError("D0 is not MWR1")
    mapping = {}
    for r in REGS:
        v = vals[r]
        if r in SIGNED and v & 0x80000000:
            v -= 1 << 32
        mapping[NAMES[r]] = v
    return {"format": "MWR1", "address_registers_are_data": True,
            "mapping": mapping}


def _no_dups(pairs):
    out = {}
    for k, v in pairs:
        if k in out:
            raise DecodeError("duplicate field: %s" % k)
        out[k] = v
    return out


def main(argv):
    if len(argv) != 2:
        sys.stderr.write("usage: Read-MacRegisterTrace.py FILE.json\n")
        return 2
    try:
        with open(argv[1], "r", encoding="utf-8") as f:
            regs = json.load(f, object_pairs_hook=_no_dups)
        print(json.dumps(decode(regs), indent=2))
    except (OSError, ValueError) as e:
        sys.stderr.write("error: %s\n" % e)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
