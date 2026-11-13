#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Headless window-message and replay checks for the anchored Mac ROM.

This pins the candidate and checks four guest checkpoints. Injected window
messages are not physical mouse input, hardware qualification or a release.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys


ROM_SHA256 = "faf37c6f96f8eb2ee6cf7a1e58a8b1b9538055d2fc89493c7419eeea816f6c8c"
ROOT = Path(__file__).resolve().parents[1]
READY_CONDITION = "0xc48:4:1"
EVENT_ORIGIN = 100000000
EVENTS = (
    "100000000:256:171:0", "108000000:100:80:0", "120000000:100:80:1",
    "128000000:132:80:2", "136000000:132:80:3", "144000000:148:80:0",
    "152000000:148:80:1", "160000000:148:56:2", "168000000:148:56:3",
    "176000000:148:64:0", "184000000:150:110:0", "192000000:150:110:1",
    "192000000:150:110:3",
)
CHECKPOINTS = (
    (142000000, (132, 80), (112, 70), 1, 0),
    (150000000, (148, 80), (112, 70), 1, 0),
    (174000000, (148, 56), (112, 46), 2, 0),
    (205000000, (150, 110), (112, 46), 3, 1),
)


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(args, log):
    result = subprocess.run([str(x) for x in args], stdin=subprocess.DEVNULL,
                            capture_output=True, timeout=30, check=False,
                            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
    log.write_bytes(result.stdout + b"\n--- stderr ---\n" + result.stderr)
    if result.returncode:
        raise ValueError(f"BootHush exited {result.returncode}; see {log}")
    return json.loads(result.stdout)


def decode(record):
    memory = record.get("memory", {})
    if "memory_ranges" in record:
        ranges = record["memory_ranges"]
        if (len(ranges) != 2 or ranges[1].get("address") != 0x8000 or
                len(bytes.fromhex(ranges[1].get("bytes", ""))) != 512):
            raise ValueError("unexpected additional diagnostic span")
        memory = ranges[0]
    if memory.get("address") != 0xB00:
        raise ValueError("unexpected diagnostic address")
    data = bytes.fromhex(memory.get("bytes", ""))
    if len(data) != 1024:
        raise ValueError("unexpected diagnostic span")
    def word(address):
        offset = address - 0xB00
        return int.from_bytes(data[offset:offset + 4], "big")
    if (word(0xC04) != 0x4D4C4331 or word(0xC18) != 0x4D4D4431 or
            word(0xC28) != 0x4D575031 or word(0xC2C) != 2 or
            word(0xC30) != 128 or word(0xCA8) != 0x43533039):
        raise ValueError("unrecognized candidate diagnostics")
    return {"pointer": [word(0xC5C), word(0xC60)],
            "window": [word(0xC64), word(0xC68)], "presses": word(0xCB4),
            "pattern": word(0xC6C), "held": word(0xC80),
            "hard_losses": [word(0xC04 + 4 * i) for i in range(1, 5)],
            "omissions": word(0xC1C), "stage": word(0xCAC),
            "fault": word(0xCC0)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--emulator", required=True, type=Path)
    parser.add_argument("--rom", required=True, type=Path)
    parser.add_argument("--after-ready", action="store_true",
                        help="schedule input after the guest readiness flag; requires multi-range inspection")
    parser.add_argument("--output", required=True, type=Path,
                        help="new scratch directory outside the repository")
    args = parser.parse_args()
    try:
        emulator = args.emulator.resolve(strict=True)
        rom = args.rom.resolve(strict=True)
        out = args.output.resolve()
        if out == ROOT or ROOT in out.parents:
            raise ValueError("output must be outside the repository")
        if rom.stat().st_size != 65536 or sha(rom) != ROM_SHA256:
            raise ValueError("anchored ROM identity mismatch")
        out.mkdir(parents=True, exist_ok=False)
        ready_cycle = None
        inspect = ["--inspect", "0xb00:1024"]
        if args.after_ready:
            calibration = run([emulator, "--machine", "mac128k", "--rom", rom,
                               "--cycles", 80000000, "--timeout-ms", 20000,
                               "--until", READY_CONDITION, "--inspect", "0xb00:1024"],
                              out / "readiness.log")
            ready_cycle = calibration.get("cycles")
            if (calibration.get("status") != "condition_met" or
                    not isinstance(ready_cycle, int) or ready_cycle <= 0):
                raise ValueError("guest readiness was not observed")
            inspect += ["--inspect", "0x8000:512"]
        observations = []
        for index, (cycles, pointer, window, presses, pattern) in enumerate(CHECKPOINTS):
            frame = out / f"checkpoint-{index}.pbm"
            trace = out / f"checkpoint-{index}.bhmouse"
            limit = cycles if ready_cycle is None else ready_cycle + cycles - EVENT_ORIGIN
            command = [emulator, "--machine", "mac128k", "--rom", rom,
                       "--cycles", limit, "--timeout-ms", 20000,
                       "--mouse-dcd-invert", 3, "--mouse-record", trace,
                       *inspect, "--frame", frame]
            if args.after_ready:
                command.extend(("--input-after", READY_CONDITION))
            event_count = 0
            for event in EVENTS:
                event_cycle, rest = event.split(":", 1)
                if int(event_cycle) <= cycles:
                    if args.after_ready:
                        event = f"{int(event_cycle) - EVENT_ORIGIN}:{rest}"
                    command.extend(("--window-mouse", event))
                    event_count += 1
            record = run(command, out / f"checkpoint-{index}.log")
            actual = decode(record)
            expected = {"pointer": list(pointer), "window": list(window),
                        "presses": presses, "pattern": pattern, "held": 0,
                        "hard_losses": [0, 0, 0, 0], "omissions": 0,
                        "stage": 16, "fault": 0}
            if (record.get("status") != "cycle_limit" or
                    record.get("input_source") != "injected_window_messages" or
                    record.get("mouse", {}).get("pending") != 0 or actual != expected):
                raise ValueError(f"checkpoint {index}: expected {expected}, observed {actual}; see log")
            if args.after_ready:
                schedule = record.get("input_schedule", {})
                if (schedule.get("ready") is not True or schedule.get("ready_cycle") != ready_cycle or
                        schedule.get("scheduled") != event_count or schedule.get("accepted") != event_count):
                    raise ValueError(f"checkpoint {index}: readiness or input schedule mismatch")
            if not frame.read_bytes().startswith(b"P4\n512 342\n"):
                raise ValueError(f"checkpoint {index}: unexpected frame format")
            observations.append({"requested_cycles": limit, "actual_cycles": record["cycles"],
                                 "guest": actual, "mouse": record["mouse"],
                                 "frame_sha256": sha(frame), "trace_sha256": sha(trace)})
        replay_frame = out / "replay.pbm"
        replay = run([emulator, "--machine", "mac128k", "--rom", rom,
                      "--cycles", limit, "--timeout-ms", 20000,
                      "--mouse-replay", out / "checkpoint-3.bhmouse",
                      *inspect, "--frame", replay_frame], out / "replay.log")
        memory_key = "memory_ranges" if args.after_ready else "memory"
        if (replay.get("status") != "cycle_limit" or
                replay.get("cycles") != record.get("cycles") or
                replay.get(memory_key) != record.get(memory_key) or
                replay_frame.read_bytes() != (out / "checkpoint-3.pbm").read_bytes()):
            raise ValueError("record/replay cycle, memory or framebuffer mismatch")
        report = {"schema_version": 1, "emulator_sha256": sha(emulator),
                  "rom_sha256": ROM_SHA256, "checkpoints": observations,
                  "replay_cycle_memory_frame_match": True,
                  "input_class": "injected_window_messages",
                  "limits": "No physical mouse, actual OS mouse delivery, hardware or release claim."}
        if args.after_ready:
            report["readiness"] = {"condition": READY_CONDITION, "cycle": ready_cycle,
                                   "inspection_ranges": 2}
        (out / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(json.dumps(report, indent=2))
        return 0
    except (OSError, ValueError, json.JSONDecodeError, subprocess.TimeoutExpired) as exc:
        print(f"Mac window diagnostic failed: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
