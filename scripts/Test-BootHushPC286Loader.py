#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Verify the original diagnostic floppy loader across its two tracks.

Runs original generated payloads in BootHush, not a ClasSICK OS or installer.
"""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--emulator", required=True, type=Path)
    p.add_argument("--output", required=True, type=Path)
    args = p.parse_args()
    try:
        spec = importlib.util.spec_from_file_location("helpers", ROOT / "scripts/Test-BootHushPC286BankPlan.py")
        h = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(h)
        emulator, out = args.emulator.resolve(strict=True), args.output.resolve()
        if out == ROOT or ROOT in out.parents:
            raise ValueError("output must be outside the repository")
        out.mkdir(parents=True, exist_ok=False)
        # Original six-byte program: emit A4 to POST, then loop forever.
        code = bytes.fromhex("b0a4e680ebfe")
        observations = []
        for size in (6, 7168, 7169, 14848):
            payload = code + bytes((i * 13 + 7) & 255 for i in range(size - len(code)))
            image = out / f"payload-{size}.img"
            with image.open("xb") as stream:
                stream.write(h.floppy(payload, 0))
            before = h.digest(image)
            chunks = [(0x10000 + off, payload[off:off + 4096]) for off in range(0, size, 4096)]
            inspect = []
            for address, data in chunks:
                inspect.extend(("--inspect", f"{hex(address)}:{len(data)}"))
            stdout = h.run([emulator, "--machine", "pc286", "--floppy", image,
                            "--cycles", "8000000", "--timeout-ms", "20000", "--until-post", "0xa4", *inspect],
                           out, os.environ.copy(), out / f"payload-{size}.log")
            result = json.loads(stdout)
            memories = [result.pop("memory", {})] if len(chunks) == 1 else result.pop("memory_ranges", [])
            if (result.get("status") != "condition_met" or result.get("post") != 0xa4 or
                    result.get("cs") != 0x1000 or len(memories) != len(chunks) or h.digest(image) != before):
                raise ValueError(f"payload {size}: bad stop, inspection count or changed disk")
            for memory, (address, data) in zip(memories, chunks):
                if memory.get("address") != address or bytes.fromhex(memory.get("bytes", "")) != data:
                    raise ValueError(f"payload {size}: loaded bytes differ at {hex(address)}")
            observations.append({"payload_bytes": size, "result": result, "floppy_sha256": before,
                                 "floppy_unchanged": True, "complete_payload_matches": True})
        for payload, entry in ((b"", 0), (bytes(14849), 0), (code, -1), (code, len(code))):
            try:
                h.floppy(payload, entry)
            except ValueError:
                continue
            raise ValueError("invalid loader input was accepted")
        # The payload crosses the track boundary. Each injection matches the
        # starting CHS of one loader read, not the firmware bootstrap read.
        payload = code + bytes((i * 13 + 7) & 255 for i in range(7169-len(code)))
        failure_cases = []
        for name, chs, expected in (
            ("first-track", "0:0:2", bytes(7169)),
            ("second-track", "0:1:1", payload[:7168] + b"\0"),
        ):
            image = out / f"failure-{name}.img"
            with image.open("xb") as stream:
                stream.write(h.floppy(payload, 0))
            before = h.digest(image)
            args = [emulator, "--machine", "pc286", "--floppy", image,
                    "--fail-floppy-read", chs, "--cycles", "8000000",
                    "--timeout-ms", "20000", "--until-post", "0xee",
                    "--inspect", "0x10000:4096", "--inspect", "0x11000:3073"]
            result = json.loads(h.run(args, out, os.environ.copy(), out / f"failure-{name}.log"))
            ranges = result.pop("memory_ranges", [])
            fault = result.get("floppy_read_failure", {})
            actual = b"".join(bytes.fromhex(row.get("bytes", "")) for row in ranges)
            selected = [int(x) for x in chs.split(":")]
            if (result.get("status") != "condition_met" or result.get("post") != 0xee or
                    len(ranges) != 2 or [x.get("address") for x in ranges] != [0x10000,0x11000] or
                    actual != expected or fault.get("injected") != 1 or fault.get("bios_status") != 32 or
                    [fault.get(x) for x in ("cylinder","head","sector")] != selected or
                    h.digest(image) != before):
                raise ValueError(f"{name}: injection, stopped state, loaded bytes or disk mismatch")
            failure_cases.append({"name": name, "chs": chs, "result": result,
                                  "floppy_sha256": before, "floppy_unchanged": True,
                                  "memory_matches_expected": True})
        report = {"schema_version": 1, "emulator_sha256": h.digest(emulator), "cases": observations,
                  "invalid_inputs_rejected": 4, "injected_failures": failure_cases,
                  "limits": "Original diagnostic floppy only; no system installation or hardware claim."}
        (out / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(json.dumps(report, indent=2))
        return 0
    except (OSError, ValueError, json.JSONDecodeError, subprocess.TimeoutExpired) as exc:
        print(f"286 diagnostic loader failed: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
