#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Exercise original compact-model far-span access on BootHush's 286.

The two fixed far windows are a test fixture, not a production memory provider
or evidence of a bootable OS, physical hardware, or release qualification.
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
PARTS = ("bank-plan", "bank-catalog", "span-bind", "span-access")
HARNESS = ROOT / "probes/pc286/boot-span-access.c"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tool-root", required=True, type=Path)
    parser.add_argument("--emulator", required=True, type=Path)
    parser.add_argument("--multi-inspect", action="store_true",
                        help="inspect the entire payload and both banks in a single stop")
    parser.add_argument("--output", required=True, type=Path,
                        help="new scratch directory outside the repository")
    args = parser.parse_args()
    try:
        spec = importlib.util.spec_from_file_location(
            "pc286_boot_helpers", ROOT / "scripts/Test-BootHushPC286BankPlan.py")
        helpers = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(helpers)
        root = args.tool_root.resolve(strict=True)
        emulator = args.emulator.resolve(strict=True)
        out = args.output.resolve()
        if out == ROOT or ROOT in out.parents:
            raise ValueError("output must be outside the repository")
        lock = json.loads((ROOT / "scripts/pc286-toolchain-lock.json").read_text(encoding="utf-8"))
        for member in lock["selected_members"]:
            path = root / member["path"]
            if path.stat().st_size != member["bytes"] or helpers.digest(path) != member["sha256"]:
                raise ValueError("pinned compiler identity mismatch: " + member["path"])
        out.mkdir(parents=True, exist_ok=False)
        env = os.environ.copy()
        for key in ("WCC", "WATCOM", "INCLUDE", "LIB", "WCL", "WLINK"):
            env.pop(key, None)
        wcc, wlink = root / "binnt64/wcc.exe", root / "binnt64/wlink.exe"
        flags = ["-zq", "-2", "-mc", "-zl", "-s", "-os", "-zastd=c99",
                 "-dCS_C16_REQUIRE_16BIT", "-i=" + str(root / "h")]
        objects = []
        for part in PARTS:
            obj = out / (part + ".obj")
            helpers.run([wcc, *flags, "-fo=" + str(obj), ROOT / ("probes/pc286/" + part + ".c")],
                        out, env, out / (part + "-compile.log"))
            objects.append(obj)
        observations = []
        cases = (("baseline", None, 0xa6), ("bad-span", "CS_P286_MUTATE_SPAN", 0xe6),
                 ("bad-epoch", "CS_P286_MUTATE_EPOCH", 0xe8),
                 ("bad-binding", "CS_P286_MUTATE_BINDING", 0xec))
        for name, mutant, expected in cases:
            obj, binary, mapfile = (out / (name + suffix) for suffix in (".obj", ".bin", ".map"))
            helpers.run([wcc, *flags, *(["-d" + mutant] if mutant else []),
                         "-fo=" + str(obj), HARNESS], out, env, out / (name + "-compile.log"))
            joined = ",".join(str(path) for path in (*objects, obj))
            helpers.run([wlink, "format", "raw", "bin", "option", "start=start_", "option", "nodefaultlibs",
                         "option", "map=" + str(mapfile), "name", binary, "file", joined],
                        out, env, out / (name + "-link.log"))
            match = re.search(r"Entry point address:\s*([0-9a-fA-F]+)",
                              mapfile.read_text(encoding="utf-8", errors="replace"))
            if not match:
                raise ValueError("missing linked entry address")
            entry, payload = int(match.group(1), 16), binary.read_bytes()
            image = out / (name + ".img")
            with image.open("xb") as stream:
                stream.write(helpers.floppy(payload, entry))
            before, runs = helpers.digest(image), []
            spans = [("code", 0x10000, payload[:4096])]
            if args.multi_inspect and len(payload) > 4096:
                spans.append(("code-rest", 0x11000, payload[4096:]))
            spans.extend((("bank-a", 0x20000, bytes.fromhex("00000044aabb")),
                          ("bank-b", 0x30000, bytes.fromhex("ccdd88000000"))))
            if any(len(data) > 4096 for _, _, data in spans):
                raise ValueError("diagnostic payload exceeds supported inspection spans")
            groups = [spans] if args.multi_inspect else [[span] for span in spans]
            for group in groups:
                label = "combined" if args.multi_inspect else group[0][0]
                inspect = []
                for _, address, expected_bytes in group:
                    inspect.extend(("--inspect", f"{hex(address)}:{len(expected_bytes)}"))
                stdout = helpers.run([emulator, "--machine", "pc286", "--floppy", image,
                                      "--cycles", "8000000", "--timeout-ms", "20000",
                                      "--until-post", hex(expected), *inspect],
                                     out, env, out / (name + "-" + label + ".log"))
                result = json.loads(stdout)
                if (result.get("status") != "condition_met" or result.get("post") != expected or
                        result.get("cs") != 0x1000):
                    raise ValueError(f"{name}/{label}: unexpected guest result or inspected bytes")
                memories = result.pop("memory_ranges", []) if args.multi_inspect else [result.pop("memory", {})]
                if len(memories) != len(group):
                    raise ValueError(f"{name}/{label}: missing inspection ranges")
                for memory, (_, address, expected_bytes) in zip(memories, group):
                    if (memory.get("address") != address or
                            bytes.fromhex(memory.get("bytes", "")) != expected_bytes):
                        raise ValueError(f"{name}/{label}: unexpected inspected bytes")
                runs.append({"observation": label, "result": result, "bytes_match": True})
            if (len({r["result"]["cycles"] for r in runs}) != 1 or
                    len({r["result"]["pc"] for r in runs}) != 1 or helpers.digest(image) != before):
                raise ValueError(f"{name}: repeated runs diverged or input image changed")
            observations.append({"case": name, "expected_post": expected, "entry": entry,
                                 "payload_bytes": len(payload), "payload_sha256": helpers.digest(binary),
                                 "inspected_payload_bytes": len(payload) if args.multi_inspect else min(len(payload), 4096),
                                 "floppy_sha256": before, "floppy_unchanged": True, "runs": runs})
        sources = {part: helpers.digest(ROOT / ("probes/pc286/" + part + ".c")) for part in PARTS}
        report = {"schema_version": 1, "compiler": "pinned Open Watcom 16-bit -2 -mc",
                  "emulator_sha256": helpers.digest(emulator), "source_sha256": sources,
                  "harness_sha256": helpers.digest(HARNESS), "cases": observations,
                  "limits": ("Fixed test windows; " +
                             ("simultaneous inspection; " if args.multi_inspect else "separate cold-boot observations; ") +
                             "no production provider, OS or hardware claim.")}
        (out / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(json.dumps(report, indent=2))
        return 0
    except (OSError, ValueError, json.JSONDecodeError, subprocess.TimeoutExpired) as exc:
        print(f"286 span diagnostic failed: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
