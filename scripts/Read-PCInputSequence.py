#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Decode a CSPSEQ01 PC input-sequence register readout (r8..r15).

Offline and read-only (SPEC-0085). The register text must come from the exact
paused own-code spin of a raw-sequence image; this tool checks only the record
format and cannot establish that identity. Accepted lines: NAME=VALUE,
NAME: VALUE or NAME VALUE, case-insensitive, hex with or without 0x.

The recorded bytes (1..48) are replayed through a MODEL of the strict SPEC-0040
report decoder (header bit 3 required at sync; an overflowed axis drops the
report; 9-bit two's-complement deltas). An error step means only that the
aggregate bus error counter changed; loss labels are compatibility
classifications, never proven controller or host causes.
"""
import argparse
import json
import re
import sys

MAGIC = 0x4353505345513031
BYTES = 48
U64 = (1 << 64) - 1
REGS = [f"r{n}" for n in range(8, 16)]
_KNOWN = re.compile(r"\s*(r(?:8|9|1[0-5]))\b", re.I)
_LINE = re.compile(r"\s*(r(?:8|9|1[0-5]))\s*(?:[=:]\s*|\s+)(?:0x)?([0-9a-f]{1,16})\s*$", re.I)
SCOPE = ("Modeled bytes delivered to this decoder; not physical origin, host intent, "
         "controller status or GUI acceptance.")


class FormatError(ValueError):
    pass


def parse_registers(text):
    regs = {}
    for line in text.splitlines():
        if not _KNOWN.match(line):
            continue
        m = _LINE.match(line)
        if not m:
            raise FormatError(f"malformed register line {line.strip()!r}")
        name = m.group(1).lower()
        if name in regs:
            raise FormatError(f"duplicate {name}")
        regs[name] = int(m.group(2), 16)
    missing = [r for r in REGS if r not in regs]
    if missing:
        raise FormatError("missing " + ",".join(missing))
    return [regs[r] for r in REGS]


def unpack(words):
    try:
        words = list(words)
    except TypeError:
        raise FormatError("expected exactly eight uint64 integers") from None
    if len(words) != 8 or any(type(w) is not int or not 0 <= w <= U64 for w in words):
        raise FormatError("expected exactly eight uint64 integers")
    if words[0] != MAGIC:
        raise FormatError(f"magic {words[0]:016x} is not CSPSEQ01")
    count = words[1] >> 56
    gaps = (words[1] >> 48) & 0xFF
    flags = words[1] & ((1 << 48) - 1)
    if not 1 <= count <= BYTES:
        raise FormatError(f"count {count} outside 1..{BYTES}")
    data = [(words[2 + i // 8] >> ((i % 8) * 8)) & 0xFF for i in range(BYTES)]
    if any(data[count:]) or flags >> count:
        raise FormatError("nonzero bytes or flags beyond count")
    return data[:count], [(flags >> i) & 1 for i in range(count)], gaps


def replay(data, errs):
    """Per byte: dict(index, byte, error, event, kind[0 partial|1 modeled packet|2 modeled loss])."""
    used, header, xbyte = 0, 0, 0
    out = []
    for i, (b, e) in enumerate(zip(data, errs)):
        ev = {"index": i, "byte": b, "error": e, "kind": 0}
        if used == 0:
            model = "drop-unsynced" if (b & 8) == 0 else "header"
        elif used == 1:
            model = "x"
        else:
            model = "drop-overflow" if (header & 0xC0) else "packet"
        if model.startswith("drop"):
            if e:
                name = "loss-unsynced-compatible" if model == "drop-unsynced" else "loss-overflow-compatible"
            else:
                name = "model-mismatch-" + model
            ev.update(event=name, kind=2)
            if model == "drop-overflow":
                ev.update(header=header, x_overflow=bool(header & 0x40), y_overflow=bool(header & 0x80),
                          raw_x=xbyte, raw_y=b)
            used = 0
        elif e:
            ev.update(event="lost-status-compatible", kind=2)  # error step: reset modeled state
            used = 0
        elif model == "header":
            ev["event"] = "header"; header, used = b, 1
        elif model == "x":
            ev["event"] = "x"; xbyte, used = b, 2
        else:
            dx = xbyte - (256 if header & 0x10 else 0)
            dy = b - (256 if header & 0x20 else 0)
            ev.update(event="packet", kind=1, header=header, dx=dx, dy=dy, buttons=header & 7)
            used = 0
        out.append(ev)
    return out, used


def summarize(events, trailing, gaps):
    packets = [e for e in events if e["event"] == "packet"]
    s = {
        "packets": len(packets),
        "overflow_compatible": sum(e["event"] == "loss-overflow-compatible" for e in events),
        "unsynced_compatible": sum(e["event"] == "loss-unsynced-compatible" for e in events),
        "status_compatible": sum(e["event"] == "lost-status-compatible" for e in events),
        "model_mismatches": sum(e["event"].startswith("model-mismatch") for e in events),
        "trailing_partial_bytes": trailing,
        "gaps": gaps,
        "alignment_qualified": gaps == 0,
        "sum_dx": sum(e["dx"] for e in packets),
        "sum_dy": sum(e["dy"] for e in packets),
        "max_abs_dx": max((abs(e["dx"]) for e in packets), default=0),
        "max_abs_dy": max((abs(e["dy"]) for e in packets), default=0),
        "button_states": sorted({e["buttons"] for e in packets}),
    }
    notes = []
    if gaps:
        notes.append(f"Recorder gaps ({gaps}): bytes were consumed between observations; modeled alignment is not qualified.")
    if s["model_mismatches"]:
        notes.append("Model mismatch: a byte the model would drop carried no error step; check image identity before interpreting.")
    if s["overflow_compatible"] or s["unsynced_compatible"] or s["status_compatible"]:
        notes.append("Compatible losses are classifications under model/history assumptions, not proven controller or host causes.")
    notes.append("Image identity, counter/non-reset assumptions and error-counter saturation remain external limits.")
    s["notes"] = notes
    s["scope"] = SCOPE
    return s


def decode(text):
    data, errs, gaps = unpack(parse_registers(text))
    events, trailing = replay(data, errs)
    return {"format": "CSPSEQ01", "count": len(data), "bytes": data, "events": events,
            "summary": summarize(events, trailing, gaps)}


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("registers", help="register readout text file")
    ap.add_argument("--json", action="store_true", help="emit JSON")
    a = ap.parse_args(argv)
    with open(a.registers, encoding="utf-8") as f:
        try:
            r = decode(f.read())
        except FormatError as exc:
            print(f"refused: {exc}", file=sys.stderr)
            return 2
    if a.json:
        print(json.dumps(r, indent=1))
        return 0
    for e in r["events"]:
        extra = ""
        if e["kind"] == 1:
            extra = f" dx={e['dx']:+d} dy={e['dy']:+d} buttons={e['buttons']}"
        elif "raw_x" in e:
            extra = f" header={e['header']:02x} xo={int(e['x_overflow'])} yo={int(e['y_overflow'])}"
        print(f"{e['index']:2d} {e['byte']:02x}{' E' if e['error'] else '  '} {e['event']}{extra}")
    print(json.dumps(r["summary"], indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
