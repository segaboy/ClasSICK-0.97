#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Strict decoder for the SPEC-0084 MWP1 v3 phase witness (diagnostic data only)."""
import json
import re
import sys

NAMES = ("tag", "version", "bytes", "source_irq", "vbl", "scc", "combined",
         "services", "baselines", "losses", "queued", "peak", "deliveries",
         "pointer_x", "pointer_y", "window_x", "window_y", "pattern", "phase",
         "motion_stage", "raw_a", "raw_b", "buttons", "motion_cause", "cause",
         "loss_reason", "loss_rr3", "loss_snap0", "loss_snap1", "loss_snap2",
         "loss_previous", "loss_phase")
SIGNED = ("pointer_x", "pointer_y", "window_x", "window_y", "motion_cause",
          "cause")
TAG = 0x4D575031
ALLOWED = 0xBD010101
GRAY_REASON = 6
POS = {0: 0, 1: 1, 3: 2, 2: 3}
_HEX = re.compile(r"(?:0[xX])?([0-9A-Fa-f]{8})")


class DecodeError(ValueError):
    pass


def _word(i, value):
    if not isinstance(value, str):
        raise DecodeError("word %d must be a string" % i)
    m = _HEX.fullmatch(value)
    if not m:
        raise DecodeError("word %d must be exactly 8 hex digits" % i)
    return int(m.group(1), 16)


def _illegal(prev, cur):
    return (POS[cur] - POS[prev]) % 4 == 2


def decode(obj):
    if not isinstance(obj, dict):
        raise DecodeError("input must be an object")
    if set(obj) != {"words"}:
        raise DecodeError("input must contain exactly 'words'")
    words = obj["words"]
    if not isinstance(words, list) or len(words) != len(NAMES):
        raise DecodeError("words must be a list of 32 strings")
    raw = [_word(i, w) for i, w in enumerate(words)]
    v = dict(zip(NAMES, raw))
    if v["tag"] != TAG:
        raise DecodeError("tag is not MWP1")
    if v["version"] == 2:
        raise DecodeError("version 2 has no phase witness; refused")
    if v["version"] != 3:
        raise DecodeError("unsupported version")
    if v["bytes"] != 128:
        raise DecodeError("bytes must be 128")
    mapping = {}
    for n in NAMES:
        x = v[n]
        if n in SIGNED and x & 0x80000000:
            x -= 1 << 32
        mapping[n] = x
    reason, prev = v["loss_reason"], v["loss_previous"]
    result = {"format": "MWP1-v3-phase-witness", "mapping": mapping,
              "previous": {"known": False, "x_phase": None, "y_phase": None},
              "current": None, "model_gray_jump_axes": None,
              "consistent_with_gray_loss": None}
    if not reason:
        if prev:
            raise DecodeError("loss_previous must be 0 without a recorded loss")
        return result
    if not prev & 0x80000000:
        raise DecodeError("recorded loss lacks phase-witness marker")
    if prev & ~ALLOWED & 0xFFFFFFFF:
        raise DecodeError("reserved loss_previous bits set")
    known = bool(prev >> 24 & 1)
    px, py = prev >> 28 & 3, prev >> 26 & 3
    if not known and (px or py):
        raise DecodeError("unknown predecessor with nonzero phases")
    if known and ((px & 1) != (prev >> 16 & 1) or (py & 1) != (prev >> 8 & 1)):
        raise DecodeError("predecessor phase contradicts retained DCD bit")
    s = v["loss_snap1"]
    pb, ra, rb = s >> 16 & 0xFF, s >> 8 & 0xFF, s & 0xFF
    cx = (pb >> 4 & 1) << 1 | (ra >> 3 & 1)
    cy = (pb >> 5 & 1) << 1 | (rb >> 3 & 1)
    result["current"] = {"x_phase": cx, "y_phase": cy}
    if known:
        result["previous"] = {"known": True, "x_phase": px, "y_phase": py}
        axes = [n for n, a, b in (("x", px, cx), ("y", py, cy)) if _illegal(a, b)]
        result["model_gray_jump_axes"] = axes
        if reason == GRAY_REASON:
            result["consistent_with_gray_loss"] = bool(axes)
    return result


def _no_dups(pairs):
    out = {}
    for k, val in pairs:
        if k in out:
            raise DecodeError("duplicate field: %s" % k)
        out[k] = val
    return out


def main(argv):
    if len(argv) != 2:
        sys.stderr.write("usage: Read-MacPhaseWitness.py FILE.json\n")
        return 2
    try:
        with open(argv[1], "r", encoding="utf-8") as f:
            obj = json.load(f, object_pairs_hook=_no_dups)
        print(json.dumps(decode(obj), indent=2))
    except (OSError, ValueError) as e:
        sys.stderr.write("error: %s\n" % e)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
