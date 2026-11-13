#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Run the original experimental key-reply parser as 16-bit guest code.

This exercises parser semantics, not a physical or emulated keyboard driver.
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
PARSER = ROOT / "core/input/m0110-reply.c"
HARNESS = ROOT / "probes/pc286/boot-m0110-reply.c"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tool-root", required=True, type=Path)
    parser.add_argument("--emulator", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    try:
        spec = importlib.util.spec_from_file_location("pc286_helpers", ROOT / "scripts/Test-BootHushPC286BankPlan.py")
        h = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(h)
        tool, emulator, out = args.tool_root.resolve(strict=True), args.emulator.resolve(strict=True), args.output.resolve()
        if out == ROOT or ROOT in out.parents:
            raise ValueError("output must be outside the repository")
        lock = json.loads((ROOT / "scripts/pc286-toolchain-lock.json").read_text(encoding="utf-8"))
        for member in lock["selected_members"]:
            path = tool / member["path"]
            if path.stat().st_size != member["bytes"] or h.digest(path) != member["sha256"]:
                raise ValueError("pinned compiler identity mismatch: " + member["path"])
        out.mkdir(parents=True, exist_ok=False)
        env = os.environ.copy()
        for key in ("WCC", "WATCOM", "INCLUDE", "LIB", "WCL", "WLINK"):
            env.pop(key, None)
        wcc, wlink = tool / "binnt64/wcc.exe", tool / "binnt64/wlink.exe"
        flags = ["-zq", "-2", "-ms", "-zl", "-s", "-os", "-zastd=c99",
                 "-dCS_C16_REQUIRE_16BIT", "-i=" + str(tool / "h")]
        parser_obj = out / "parser.obj"
        h.run([wcc, *flags, "-fo=" + str(parser_obj), PARSER], out, env, out / "parser-compile.log")
        observations = []
        for name, mutant, expected in (("baseline", None, 0xa3),
                                       ("bad-release", "CS_KEY_MUTATE_RELEASE", 0xe3),
                                       ("bad-invalid", "CS_KEY_MUTATE_INVALID", 0xe4)):
            obj, binary, mapfile = (out / (name + suffix) for suffix in (".obj", ".bin", ".map"))
            h.run([wcc, *flags, *(["-d" + mutant] if mutant else []), "-fo=" + str(obj), HARNESS],
                  out, env, out / (name + "-compile.log"))
            h.run([wlink, "format", "raw", "bin", "option", "start=start_", "option", "nodefaultlibs",
                   "option", "map=" + str(mapfile), "name", binary, "file", str(parser_obj) + "," + str(obj)],
                  out, env, out / (name + "-link.log"))
            match = re.search(r"Entry point address:\s*([0-9a-fA-F]+)",
                              mapfile.read_text(encoding="utf-8", errors="replace"))
            if not match:
                raise ValueError("missing linked entry address")
            payload = binary.read_bytes()
            if len(payload) > 4096:
                raise ValueError("payload exceeds single inspection limit")
            image = out / (name + ".img")
            with image.open("xb") as stream:
                stream.write(h.floppy(payload, int(match.group(1), 16)))
            before = h.digest(image)
            stdout = h.run([emulator, "--machine", "pc286", "--floppy", image,
                            "--cycles", "8000000", "--timeout-ms", "20000", "--until-post", hex(expected),
                            "--inspect", f"0x10000:{len(payload)}"], out, env, out / (name + "-boot.log"))
            result = json.loads(stdout)
            memory = result.pop("memory", {})
            if (result.get("status") != "condition_met" or result.get("post") != expected or
                    result.get("cs") != 0x1000 or memory.get("address") != 0x10000 or
                    bytes.fromhex(memory.get("bytes", "")) != payload or h.digest(image) != before):
                raise ValueError(f"{name}: unexpected guest result, payload or changed disk")
            observations.append({"case": name, "result": result, "payload_sha256": h.digest(binary),
                                 "payload_bytes": len(payload), "floppy_sha256": before,
                                 "payload_matches": True, "floppy_unchanged": True})
        report = {"schema_version": 1, "compiler": "pinned Open Watcom 16-bit -2 -ms",
                  "emulator_sha256": h.digest(emulator), "parser_sha256": h.digest(PARSER),
                  "harness_sha256": h.digest(HARNESS), "cases": observations,
                  "limits": "Original parser fixture only; no keyboard I/O, OS boot or physical hardware claim."}
        (out / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(json.dumps(report, indent=2))
        return 0
    except (OSError, ValueError, json.JSONDecodeError, subprocess.TimeoutExpired) as exc:
        print(f"286 key-reply diagnostic failed: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
