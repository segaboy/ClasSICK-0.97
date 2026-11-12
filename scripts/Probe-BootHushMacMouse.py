#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Run a bounded, original-input Mac mouse timing probe in BootHush.

This measures this ROM on this emulator. It does not establish physical Mac
timing or accept a release. The diagnostic layout must match the verified
MWP1 v2 fixture identified by --rom-sha256.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def word(data, address):
    offset = address - 0xB00
    if offset < 0 or offset + 4 > len(data):
        raise ValueError("diagnostic word outside inspected memory")
    return int.from_bytes(data[offset:offset + 4], "big")


def probe(executable, rom, edge, guard):
    args = [str(executable), "--machine", "mac128k", "--rom", str(rom),
            "--cycles", "65000000", "--timeout-ms", "20000",
            "--mouse-dcd-invert", "3", "--mouse-motion", "60000000:32:0",
            "--mouse-edge-cycles", str(edge), "--mouse-vbl-guard", str(guard),
            "--inspect", "0xb00:1024"]
    result = subprocess.run(args, capture_output=True, text=True, timeout=30,
                            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
                            check=False)
    if result.returncode != 0:
        raise ValueError(f"BootHush exited {result.returncode}: {result.stdout[:1000]} {result.stderr[:1000]}")
    record = json.loads(result.stdout)
    if record.get("status") != "cycle_limit":
        raise ValueError(f"unexpected BootHush status: {record.get('status')}")
    memory = record.get("memory", {})
    if memory.get("address") != 0xB00:
        raise ValueError("unexpected diagnostic memory address")
    data = bytes.fromhex(memory.get("bytes", ""))
    if (len(data) != 1024 or word(data, 0xC04) != 0x4D4C4331 or
            word(data, 0xC18) != 0x4D4D4431 or
            word(data, 0xC20) != 0x4D434D31 or
            word(data, 0xC28) != 0x4D575031 or word(data, 0xC2C) != 2 or
            word(data, 0xC30) != 128):
        raise ValueError("unrecognized MWP1 v2 diagnostic layout")
    mouse = record.get("mouse", {})
    actual = {"edge_cycles": edge, "vbl_guard_cycles": guard,
              "stop_cycles": record.get("cycles"),
              "mouse": mouse,
              "pointer": [word(data, 0xC5C), word(data, 0xC60)],
              "window": [word(data, 0xC64), word(data, 0xC68)],
              "hard_losses": [word(data, 0xC04 + 4 * i) for i in range(1, 5)],
              "omitted_passes": word(data, 0xC1C),
              "merges": word(data, 0xC24)}
    actual["passed"] = (actual["pointer"] == [288, 171] and
                        actual["window"] == [80, 70] and
                        actual["hard_losses"] == [0, 0, 0, 0] and
                        actual["omitted_passes"] == 0 and
                        mouse.get("samples") == 1 and
                        mouse.get("steps") == 64 and
                        mouse.get("pending") == 0)
    return actual


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--emulator", required=True, type=Path)
    parser.add_argument("--rom", required=True, type=Path)
    parser.add_argument("--rom-sha256", required=True)
    parser.add_argument("--output", required=True, type=Path,
                        help="new JSON report path outside the source tree")
    args = parser.parse_args()
    try:
        if args.output.exists():
            raise ValueError("report already exists")
        if len(args.rom_sha256) != 64 or any(c not in "0123456789abcdef" for c in args.rom_sha256):
            raise ValueError("expected ROM SHA-256 must be lowercase hexadecimal")
        if args.rom.stat().st_size != 65536 or sha256(args.rom) != args.rom_sha256:
            raise ValueError("ROM size or SHA-256 mismatch")
        emulator_hash = sha256(args.emulator)
        results = [probe(args.emulator, args.rom, edge, guard)
                   for edge, guard in [(8192, 4096), (4096, 4096),
                                       (2048, 4096), (8192, 0)]]
        report = {"schema_version": 1, "emulator_sha256": emulator_hash,
                  "rom_sha256": args.rom_sha256,
                  "input": {"cycle": 60000000, "dx": 32, "dy": 0,
                            "dcd_invert": 3},
                  "results": results,
                  "limitations": "Synthetic BootHush input only; no physical or human qualification."}
        with args.output.open("x", encoding="utf-8", newline="\n") as stream:
            json.dump(report, stream, indent=2)
            stream.write("\n")
        print(json.dumps({"output": str(args.output), "results": results}, indent=2))
        return 0 if all(item["passed"] for item in results) else 1
    except (OSError, ValueError, json.JSONDecodeError, subprocess.TimeoutExpired) as exc:
        print(f"BootHush probe error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
