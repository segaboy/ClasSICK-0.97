#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Compile and run an original 16-bit bank-plan diagnostic in BootHush.

This is a bounded emulator test of the unbound model, not a 286 OS, a native
backing provider, physical-hardware proof, or a release qualification.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]
PLAN = ROOT / "probes/pc286/bank-plan.c"
HARNESS = ROOT / "probes/pc286/boot-bank-plan.c"


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(args, cwd, env, log, timeout=30):
    result = subprocess.run([str(x) for x in args], cwd=cwd, env=env,
                            stdin=subprocess.DEVNULL, capture_output=True,
                            timeout=timeout, check=False,
                            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
    log.write_bytes(result.stdout + b"\n--- stderr ---\n" + result.stderr)
    if result.returncode:
        raise ValueError(f"command exited {result.returncode}; see {log}")
    return result.stdout


def floppy(payload, entry):
    sectors = (len(payload) + 511) // 512
    if not 1 <= sectors <= 29 or not 0 <= entry < len(payload):
        raise ValueError("payload or entry does not fit the original two-track loader")
    # Original diagnostic loader. Read through the firmware's documented disk
    # interface at 1000:0000, set DS=SS=1000 and jump to the linked entry.
    # On either read failure emit EE to the diagnostic POST port.
    boot = bytearray.fromhex("fa31c08ed0bc007c8ed8fbb800108ec031db")
    boot += bytes([0xb8, min(sectors, 14), 0x02]) + bytes.fromhex("b9020030f6cd1372")
    branches = [len(boot)]
    boot.append(0)
    if sectors > 14:
        # Continue at cylinder 0, head 1, sector 1 into 1000:1c00. The
        # first 14 sectors occupy 1c00h bytes, and at most 15 remain.
        boot += bytes([0xb8, sectors - 14, 0x02])
        boot += bytes.fromhex("bb001cb90100ba0001cd1372")
        branches.append(len(boot))
        boot.append(0)
    boot += bytes.fromhex("fab800108ed88ed0bcfefffbea") + struct.pack("<HH", entry, 0x1000)
    failure = len(boot)
    boot += bytes.fromhex("b0eee680ebfe")
    for branch in branches:
        displacement = failure - (branch + 1)
        if not -128 <= displacement <= 127:
            raise ValueError("loader branch invalid")
        boot[branch] = displacement & 0xff
    if len(boot) > 510:
        raise ValueError("loader size invalid")
    boot += bytes(510 - len(boot)) + b"\x55\xaa"
    return bytes(boot) + payload + bytes(1228800 - 512 - len(payload))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tool-root", required=True, type=Path)
    parser.add_argument("--emulator", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path,
                        help="new scratch directory outside the repository")
    args = parser.parse_args()
    try:
        root = args.tool_root.resolve(strict=True)
        emulator = args.emulator.resolve(strict=True)
        out = args.output.resolve()
        if out == ROOT or ROOT in out.parents:
            raise ValueError("output must be outside the repository")
        lock = json.loads((ROOT / "scripts/pc286-toolchain-lock.json").read_text(encoding="utf-8"))
        for member in lock["selected_members"]:
            path = root / member["path"]
            if path.stat().st_size != member["bytes"] or digest(path) != member["sha256"]:
                raise ValueError("pinned compiler identity mismatch: " + member["path"])
        out.mkdir(parents=True, exist_ok=False)
        env = os.environ.copy()
        for key in ("WCC", "WATCOM", "INCLUDE", "LIB", "WCL", "WLINK"):
            env.pop(key, None)
        wcc = root / "binnt64/wcc.exe"
        wlink = root / "binnt64/wlink.exe"
        flags = ["-zq", "-2", "-ms", "-zl", "-s", "-os", "-zastd=c99",
                 "-dCS_C16_REQUIRE_16BIT", "-i=" + str(root / "h")]
        plan_object = out / "bank-plan.obj"
        run([wcc, *flags, "-fo=" + str(plan_object), PLAN], out, env, out / "plan-compile.log")
        cases = [("baseline", None, 0xa5), ("bad-plan", "CS_P286_MUTATE_PLAN", 0xe3),
                 ("bad-odd", "CS_P286_MUTATE_ODD", 0xe4)]
        observations = []
        for name, mutant, expected in cases:
            obj, binary, mapfile = (out / (name + suffix) for suffix in (".obj", ".bin", ".map"))
            compile_flags = flags + (["-d" + mutant] if mutant else [])
            run([wcc, *compile_flags, "-fo=" + str(obj), HARNESS], out, env, out / (name + "-compile.log"))
            run([wlink, "format", "raw", "bin", "option", "start=start_", "option", "nodefaultlibs",
                 "option", "map=" + str(mapfile), "name", binary,
                 "file", str(plan_object) + "," + str(obj)], out, env, out / (name + "-link.log"))
            match = re.search(r"Entry point address:\s*([0-9a-fA-F]+)",
                              mapfile.read_text(encoding="utf-8", errors="replace"))
            if not match:
                raise ValueError("missing linked entry address")
            entry = int(match.group(1), 16)
            payload = binary.read_bytes()
            if len(payload) > 4096:
                raise ValueError("payload exceeds the complete guest inspection limit")
            image = out / (name + ".img")
            with image.open("xb") as stream:
                stream.write(floppy(payload, entry))
            before = digest(image)
            stdout = run([emulator, "--machine", "pc286", "--floppy", image,
                          "--cycles", "8000000", "--timeout-ms", "20000",
                          "--until-post", hex(expected), "--inspect", f"0x10000:{len(payload)}"],
                         out, env, out / (name + "-boot.log"))
            response = json.loads(stdout)
            after = digest(image)
            memory = response.pop("memory", {})
            if (response.get("status") != "condition_met" or response.get("post") != expected or
                    response.get("cs") != 0x1000 or before != after or
                    memory.get("address") != 0x10000 or
                    bytes.fromhex(memory.get("bytes", "")) != payload):
                raise ValueError(f"{name}: unexpected guest result, loaded bytes or input image")
            observations.append({"case": name, "expected_post": expected, "entry": entry,
                                 "payload_sha256": digest(binary), "floppy_sha256": before,
                                 "result": response, "loaded_payload_matches": True,
                                 "floppy_unchanged": before == after})
        report = {"schema_version": 1, "compiler": "pinned Open Watcom 16-bit -2 -ms",
                  "emulator_sha256": digest(emulator), "bank_plan_sha256": digest(PLAN),
                  "harness_sha256": digest(HARNESS), "cases": observations,
                  "limits": "Original unbound bank planner only; no native backing, OS boot, hardware or release claim."}
        (out / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(json.dumps(report, indent=2))
        return 0
    except (OSError, ValueError, json.JSONDecodeError, subprocess.TimeoutExpired) as exc:
        print(f"286 diagnostic failed: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
