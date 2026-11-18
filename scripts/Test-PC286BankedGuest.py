#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Qualify an actual native 286 bank-backed scalar provider with original media."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
PROVIDER = ROOT / "platform/pc286/banked-guest.c"
PROBE = ROOT / "probes/pc286/boot-banked-guest.c"
SPEC = ROOT / "docs/specifications/SPEC-0117-pc286-banked-guest-memory.md"
EMULATOR_SHA = "7adb8a74cef009b7c532df24458c3a5db6b77725356fdb31e5ac4966d7ed971a"


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tool-root", required=True, type=Path)
    parser.add_argument("--emulator", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    try:
        sources = [PROVIDER, PROVIDER.with_suffix(".h"), PROBE, SPEC, ROOT / "arch/i286/c16.h",
                   ROOT / "scripts/pc286_loader.py", ROOT / "scripts/pc286-toolchain-lock.json", Path(__file__)]
        before = {path.relative_to(ROOT).as_posix(): sha(path) for path in sources}
        tool, emulator = args.tool_root.resolve(strict=True), args.emulator.resolve(strict=True)
        if sha(emulator) != EMULATOR_SHA:
            raise ValueError("emulator does not match admitted original fixture profile")
        lock = json.loads((ROOT / "scripts/pc286-toolchain-lock.json").read_text(encoding="utf-8"))

        def tools_unchanged():
            return all((tool / item["path"]).stat().st_size == item["bytes"] and
                       sha(tool / item["path"]) == item["sha256"] for item in lock["selected_members"])

        if not tools_unchanged():
            raise ValueError("pinned compiler tool mismatch")
        module_spec = importlib.util.spec_from_file_location("bank_guest_loader", ROOT / "scripts/pc286_loader.py")
        loader = importlib.util.module_from_spec(module_spec)
        module_spec.loader.exec_module(loader)
        out = args.output.resolve()
        if out == ROOT or ROOT in out.parents:
            raise ValueError("output must be outside the repository")
        out.mkdir(parents=True, exist_ok=False)
        env = os.environ.copy()
        for key in ("WCC", "WATCOM", "INCLUDE", "LIB", "WCL", "WLINK"):
            env.pop(key, None)
        commands = []

        def run(command, label):
            command = [str(item) for item in command]
            commands.append(command)
            result = subprocess.run(command, cwd=out, env=env, capture_output=True, timeout=35,
                                    creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            (out / (label + ".stdout")).write_bytes(result.stdout)
            (out / (label + ".stderr")).write_bytes(result.stderr)
            if result.returncode:
                raise ValueError(label + " failed; see retained output")
            return result.stdout

        text = PROVIDER.read_text(encoding="utf-8")
        replacements = {
            "byte-order": ("staged[0] = (uint8_t)(value >> 24);", "staged[0] = (uint8_t)value;"),
            "generation": ("generation != s->generation", "(generation != s->generation && 0)"),
            "early-write": ("    if (result != CS_PG_OK) return result;\n    staged[0]",
                            "    if (result != CS_PG_OK) {\n"
                            "        if (result == CS_PG_BACKING && width == 4u)\n"
                            "            *(volatile uint8_t __far *)(((uint32_t)s->banks[0].segment << 16) | s->banks[0].offset) = 0xee;\n"
                            "        return result;\n    }\n    staged[0]")}
        cases = (("optimized", "-os", None, 0xb8), ("unoptimized", "-od", None, 0xb8),
                 ("bad-byte-order", "-os", "byte-order", 0xe1),
                 ("bad-generation", "-os", "generation", 0xe2),
                 ("bad-early-write", "-os", "early-write", 0xe3))
        observations = []
        for name, optimization, defect, marker in cases:
            source = PROVIDER
            if defect:
                old, new = replacements[defect]
                if text.count(old) != 1:
                    raise ValueError("mutation anchor changed: " + defect)
                source = out / (name + ".c")
                source.write_text(text.replace(old, new), encoding="utf-8")
            flags = ["-zq", "-2", "-ms", "-zl", "-s", optimization, "-zastd=c99", "-we",
                     "-dCS_C16_REQUIRE_16BIT", "-i=" + str(tool / "h"), "-i=" + str(PROVIDER.parent)]
            provider_object, probe_object = out / (name + "-provider.obj"), out / (name + "-probe.obj")
            run([tool / "binnt64/wcc.exe", *flags, "-fo=" + str(provider_object), source], name + "-provider")
            run([tool / "binnt64/wcc.exe", *flags, "-fo=" + str(probe_object), PROBE], name + "-probe")
            metadata = run([tool / "binnt64/dmpobj.exe", provider_object], name + "-metadata")
            if b"80286" not in metadata or b"USE16" not in metadata:
                raise ValueError("provider is not an 80286 USE16 object")
            binary, mapfile = out / (name + ".bin"), out / (name + ".map")
            run([tool / "binnt64/wlink.exe", "format", "raw", "bin", "option", "start=start_",
                 "option", "nodefaultlibs", "option", "map=" + str(mapfile), "name", binary,
                 "file", str(provider_object) + "," + str(probe_object)], name + "-link")
            linked = mapfile.read_text(encoding="utf-8", errors="replace")
            entry = re.search(r"Entry point address:\s*([0-9a-fA-F]+)", linked)
            if entry is None or ".lib" in linked.lower():
                raise ValueError("unexpected runtime library or missing entry")
            payload = binary.read_bytes()
            if len(payload) > 12288:
                raise ValueError("payload exceeds complete inspection budget")
            image = out / (name + ".img")
            image.write_bytes(loader.floppy(payload, int(entry.group(1), 16), verify_payload=True))
            original_hash = sha(image)
            ranges = [(0x10000 + offset, min(4096, len(payload) - offset))
                      for offset in range(0, len(payload), 4096)]
            expected_ram = {0x2ffff: "11", 0x32001: "22", 0x4200f: "33", 0x53234: "44"}
            if defect is None:
                ranges += [(address, 1) for address in expected_ram]
            command = [emulator, "--machine", "pc286", "--floppy", image,
                       "--cycles", "40000000", "--timeout-ms", "20000", "--until-post", hex(marker)]
            for address, size in ranges:
                command += ["--inspect", hex(address) + ":" + str(size)]
            result = json.loads(run(command, name + "-guest"))
            memory = result.get("memory_ranges", [result.get("memory", {})])
            if result.get("status") != "condition_met" or result.get("post") != marker or result.get("cs") != 0x1000:
                raise ValueError(name + ": unexpected guest stop")
            if len(memory) != len(ranges) or any(row.get("address") != requested[0]
                    or len(bytes.fromhex(row.get("bytes", ""))) != requested[1]
                    for row, requested in zip(memory, ranges)):
                raise ValueError("invalid inspection result")
            loaded = b"".join(bytes.fromhex(row["bytes"]) for row in memory if 0x10000 <= row["address"] < 0x13000)
            if loaded != payload or sha(image) != original_hash:
                raise ValueError("loaded payload mismatch or source media changed")
            if defect is None:
                actual = {row["address"]: row["bytes"] for row in memory if row["address"] in expected_ram}
                if actual != expected_ram:
                    raise ValueError("far pointer physical targets differ")
            observations.append({"case": name, "defect": defect, "expected_post": marker,
                                 "payload_bytes": len(payload), "payload_sha256": sha(binary),
                                 "floppy_sha256": original_hash, "loaded_payload_matches": True,
                                 "source_media_unchanged": True, "result": result})
        if before != {path.relative_to(ROOT).as_posix(): sha(path) for path in sources}:
            raise ValueError("source/spec/helper changed during qualification")
        if not tools_unchanged() or sha(emulator) != EMULATOR_SHA:
            raise ValueError("tool/emulator changed during qualification")
        report = {"status": "pass", "source_sha256": before, "source_unchanged": True,
                  "toolchain": lock, "emulator_sha256": EMULATOR_SHA, "commands": commands,
                  "cases": observations,
                  "limits": "Original native286 scalar bank primitive only; trusted arena ownership, no interpreter integration, full OS or physical validation."}
        (out / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(json.dumps({"status": "pass", "cases": len(observations), "report": str(out / "report.json")}))
        return 0
    except (OSError, ValueError, KeyError, subprocess.TimeoutExpired) as exc:
        print("Native bank verification failed: " + str(exc), file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
