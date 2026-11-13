#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Run the original 16-bit far-span planner and binder in BootHush.

The diagnostic uses fixed test windows, not a production memory provider.
"""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
PARTS = ("bank-plan", "bank-catalog", "span-build", "span-bind", "span-access")
HARNESS = ROOT / "probes/pc286/boot-span-build.c"


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--tool-root", required=True, type=Path)
    p.add_argument("--emulator", required=True, type=Path)
    p.add_argument("--output", required=True, type=Path)
    p.add_argument("--capability-chain", action="store_true",
                   help="exercise planned far spans through the capability layer")
    args = p.parse_args()
    try:
        harness = ROOT / "probes/pc286/boot-memory-chain.c" if args.capability_chain else HARNESS
        parts = ("bank-plan", "bank-catalog", "span-build", "span-bind", "span-cap") if args.capability_chain else PARTS
        cases = (("baseline", None, 0xa9), ("bad-permission", "CS_CHAIN_MUTATE_PERMISSION", 0xc6),
                 ("bad-stale", "CS_CHAIN_MUTATE_STALE", 0xc9)) if args.capability_chain else (
                 ("baseline", None, 0xa8), ("bad-boundary", "CS_BUILD_MUTATE_BOUNDARY", 0xb5),
                 ("bad-atomic", "CS_BUILD_MUTATE_ATOMIC", 0xb7), ("bad-epoch", "CS_BUILD_MUTATE_EPOCH", 0xb8))
        spec = importlib.util.spec_from_file_location("helpers", ROOT / "scripts/Test-BootHushPC286BankPlan.py")
        helpers = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(helpers)
        tool, emulator, out = args.tool_root.resolve(strict=True), args.emulator.resolve(strict=True), args.output.resolve()
        if out == ROOT or ROOT in out.parents:
            raise ValueError("output must be outside the repository")
        lock = json.loads((ROOT / "scripts/pc286-toolchain-lock.json").read_text(encoding="utf-8"))
        for member in lock["selected_members"]:
            path = tool / member["path"]
            if path.stat().st_size != member["bytes"] or helpers.digest(path) != member["sha256"]:
                raise ValueError("pinned compiler identity mismatch: " + member["path"])
        out.mkdir(parents=True, exist_ok=False)
        env = os.environ.copy()
        for key in ("WCC", "WATCOM", "INCLUDE", "LIB", "WCL", "WLINK"):
            env.pop(key, None)
        wcc, wlink = tool / "binnt64/wcc.exe", tool / "binnt64/wlink.exe"
        flags = ["-zq", "-2", "-mc", "-zl", "-s", "-os", "-zastd=c99",
                 "-dCS_C16_REQUIRE_16BIT", "-i=" + str(tool / "h")]
        near_flags = ["-ms" if flag == "-mc" else flag for flag in flags]
        near = subprocess.run([str(x) for x in (wcc, *near_flags,
                              "-fo=" + str(out / "near.obj"), harness)], cwd=out, env=env,
                              stdin=subprocess.DEVNULL, capture_output=True, timeout=30, check=False,
                              creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        near_log = near.stdout + b"\n--- stderr ---\n" + near.stderr
        (out / "near.log").write_bytes(near_log)
        if near.returncode == 0 or re.search(re.escape(harness.name.encode()) + rb"\(\d+\): Error! E1020", near_log) is None:
            raise ValueError("near-data pointer guard did not reject the target")
        objects = []
        for part in parts:
            obj = out / (part + ".obj")
            helpers.run([wcc, *flags, "-fo=" + str(obj), ROOT / ("probes/pc286/" + part + ".c")],
                        out, env, out / (part + "-compile.log"))
            objects.append(obj)
        observations = []
        for name, mutant, expected in cases:
            obj, binary, mapfile = (out / (name + suffix) for suffix in (".obj", ".bin", ".map"))
            helpers.run([wcc, *flags, *(["-d" + mutant] if mutant else []), "-fo=" + str(obj), harness],
                        out, env, out / (name + "-compile.log"))
            helpers.run([wlink, "format", "raw", "bin", "option", "start=start_", "option", "nodefaultlibs",
                         "option", "map=" + str(mapfile), "name", binary, "file",
                         ",".join(str(path) for path in (*objects, obj))], out, env, out / (name + "-link.log"))
            match = re.search(r"Entry point address:\s*([0-9a-fA-F]+)",
                              mapfile.read_text(encoding="utf-8", errors="replace"))
            if not match:
                raise ValueError("missing linked entry address")
            payload = binary.read_bytes()
            image = out / (name + ".img")
            with image.open("xb") as stream:
                stream.write(helpers.floppy(payload, int(match.group(1), 16)))
            before = helpers.digest(image)
            spans = [(0x10000 + off, payload[off:off + 4096]) for off in range(0, len(payload), 4096)]
            spans.extend(((0x27ffd, bytes.fromhex("44aabb00")),
                          (0x30010, bytes.fromhex("ccdd88000000"))))
            inspect = []
            for address, data in spans:
                inspect.extend(("--inspect", f"{hex(address)}:{len(data)}"))
            stdout = helpers.run([emulator, "--machine", "pc286", "--floppy", image,
                                  "--cycles", "8000000", "--timeout-ms", "20000", "--until-post", hex(expected),
                                  *inspect], out, env, out / (name + "-boot.log"))
            result = json.loads(stdout)
            memories = result.pop("memory_ranges", [])
            if (result.get("status") != "condition_met" or result.get("post") != expected or
                    result.get("cs") != 0x1000 or len(memories) != len(spans) or helpers.digest(image) != before):
                raise ValueError(f"{name}: unexpected stop, inspection count or changed floppy")
            for memory, (address, data) in zip(memories, spans):
                if memory.get("address") != address or bytes.fromhex(memory.get("bytes", "")) != data:
                    raise ValueError(f"{name}: inspected bytes differ at {hex(address)}")
            observations.append({"case": name, "result": result, "payload_bytes": len(payload),
                                 "payload_sha256": helpers.digest(binary), "floppy_sha256": before,
                                 "floppy_unchanged": True, "complete_payload_and_backing_match": True})
        report = {"schema_version": 1, "compiler": "pinned Open Watcom 16-bit -2 -mc",
                  "emulator_sha256": helpers.digest(emulator), "near_data_guard_rejected": True,
                  "source_sha256": {part: helpers.digest(ROOT / ("probes/pc286/" + part + ".c")) for part in parts},
                  "harness_sha256": helpers.digest(harness), "cases": observations,
                  "limits": "Fixed test windows; no production provider, OS, physical hardware or release claim."}
        (out / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(json.dumps(report, indent=2))
        return 0
    except (OSError, ValueError, json.JSONDecodeError, subprocess.TimeoutExpired) as exc:
        print(f"286 span construction diagnostic failed: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
