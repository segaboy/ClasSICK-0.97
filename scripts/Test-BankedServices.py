#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Qualify opt-in bank-backed GetTrapAddress with atomic six-byte completion."""
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
SERVICE = ROOT / "personality/system1/banked-services.c"
CORE = [SERVICE, ROOT / "core/cpu/m68k-bus.c", ROOT / "core/cpu/m68k.c",
        ROOT / "core/memory/guest.c", ROOT / "core/services/traps.c"]
NATIVE = [*CORE, ROOT / "platform/pc286/banked-guest.c", ROOT / "platform/pc286/m68k-banked.c",
          ROOT / "platform/pc286/m68k-transfer.c", ROOT / "probes/pc286/boot-banked-services.c"]
EMULATOR_SHA = "7adb8a74cef009b7c532df24458c3a5db6b77725356fdb31e5ac4966d7ed971a"


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("cc", "tool-root", "emulator", "output"):
        parser.add_argument("--" + name, required=True, type=Path)
    args = parser.parse_args()
    try:
        sources = [*NATIVE, *[ROOT / name for name in (
            "personality/system1/banked-services.h", "core/cpu/m68k-bus.h", "core/cpu/m68k-guard.h",
            "core/cpu/m68k-backend.h", "core/cpu/m68k-transfer.h", "core/cpu/m68k.h",
            "core/memory/guest.h", "core/services/traps.h", "platform/pc286/banked-guest.h",
            "platform/pc286/m68k-banked.h", "platform/pc286/m68k-transfer.h", "arch/i286/c16.h", "scripts/pc286_loader.py",
            "scripts/pc286-toolchain-lock.json", "tests/banked-services.c",
            "docs/specifications/SPEC-0020-shared-trap-address-services.md",
            "docs/specifications/SPEC-0025-guest-memory-traps.md",
            "docs/specifications/SPEC-0117-pc286-banked-guest-memory.md",
            "docs/specifications/SPEC-0120-banked-m68k-execution.md",
            "docs/specifications/SPEC-0123-m68k-loop-register-transfer.md",
            "docs/specifications/SPEC-0124-banked-gettrap-service.md",
            "provenance/records/SRC-0618-movem-instruction.md")], Path(__file__)]
        before = {path.relative_to(ROOT).as_posix(): sha(path) for path in sources}
        cc, tool, emulator = args.cc.resolve(strict=True), args.tool_root.resolve(strict=True), args.emulator.resolve(strict=True)
        if sha(emulator) != EMULATOR_SHA:
            raise ValueError("emulator does not match admitted original-fixture profile")
        lock = json.loads((ROOT / "scripts/pc286-toolchain-lock.json").read_text(encoding="utf-8"))
        tool_paths = {"host_cc": cc, "emulator": emulator, "python": Path(sys.executable).resolve(strict=True)}
        for item in lock["selected_members"]:
            path = tool / item["path"]
            if path.stat().st_size != item["bytes"] or sha(path) != item["sha256"]:
                raise ValueError("pinned compiler/header mismatch: " + item["path"])
            tool_paths[item["path"]] = path
        tool_hashes = {name: sha(path) for name, path in tool_paths.items()}
        module_spec = importlib.util.spec_from_file_location("banked_service_loader", ROOT / "scripts/pc286_loader.py")
        loader = importlib.util.module_from_spec(module_spec)
        module_spec.loader.exec_module(loader)
        out = args.output.resolve()
        if out == ROOT or ROOT in out.parents:
            raise ValueError("output must be outside repository")
        out.mkdir(parents=True, exist_ok=False)
        env = os.environ.copy()
        env["PATH"] = str(cc.parent) + os.pathsep + env.get("PATH", "")
        for key in ("WCC", "WATCOM", "INCLUDE", "LIB", "WCL", "WLINK"):
            env.pop(key, None)
        commands = []

        def run(command, label):
            command = [str(item) for item in command]
            commands.append(command)
            result = subprocess.run(command, cwd=out, env=env, capture_output=True, timeout=40,
                                    creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            (out / (label + ".stdout")).write_bytes(result.stdout)
            (out / (label + ".stderr")).write_bytes(result.stderr)
            if result.returncode:
                raise ValueError(label + " failed; see retained output")
            return result.stdout

        warnings = ["-Wall", "-Wextra", "-Werror", "-Wconversion", "-Wsign-conversion"]
        hosted = []
        for profile, language, flags in (("release", "c11", ["-O3", "-DNDEBUG"]),
                ("sanitized", "c11", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all", "-fno-omit-frame-pointer"]),
                ("c99", "c99", ["-O2"])):
            binary = out / (profile + ".exe")
            run([cc, "-std=" + language, *warnings, *flags, *CORE, ROOT / "tests/banked-services.c", "-o", binary], profile + "-build")
            answer = run([binary], profile + "-test")
            hosted.append({"profile": profile, "result": answer.decode().strip(), "sha256": sha(binary)})
        text = SERVICE.read_text(encoding="utf-8")
        replacements = {
            "early-frame": ("    report->memory=backend->write_bytes(b->context,scratch,6,bytes);",
                            "    copy_frame(frame,&proposed);\n    report->memory=backend->write_bytes(b->context,scratch,6,bytes);"),
            "split-store": ("    report->memory=backend->write_bytes(b->context,scratch,6,bytes);",
                            "    report->memory=b->write(b->context,scratch,2,proposed.sr);\n"
                            "    if(report->memory==CS_GUEST_OK)report->memory=b->write(b->context,scratch+2,4,proposed.pc);"),
            "signature": ("    if(target!=gateway->config.get_token){", "    if(0){")}
        cases = (("optimized", "-os", None, 0xb8), ("unoptimized-service", "-od", None, 0xb8),
                 ("bad-early-frame", "-os", "early-frame", 0xe1),
                 ("bad-split-store", "-os", "split-store", 0xe2),
                 ("bad-signature", "-os", "signature", 0xe3))
        observations = []
        for name, optimization, defect, marker in cases:
            service = SERVICE
            if defect:
                old, new = replacements[defect]
                if text.count(old) != 1:
                    raise ValueError("mutation anchor changed: " + defect)
                service = out / (name + ".c")
                service.write_text(text.replace(old, new), encoding="utf-8")
            objects, object_rows = [], []
            for index, source in enumerate([service, *NATIVE[1:]]):
                obj = out / (name + "-" + str(index) + ".obj")
                objects.append(obj)
                flags = ["-zq", "-2", "-ms", "-zl", "-s", optimization if index == 0 else "-os",
                         "-zastd=c99", "-we", "-dCS_C16_REQUIRE_16BIT", "-i=" + str(tool / "h"), "-i=" + str(SERVICE.parent)]
                run([tool / "binnt64/wcc.exe", *flags, "-fo=" + str(obj), source], name + "-" + str(index) + "-compile")
                metadata = run([tool / "binnt64/dmpobj.exe", obj], name + "-" + str(index) + "-metadata")
                if b"80286" not in metadata or b"USE16" not in metadata:
                    raise ValueError("not an 80286 USE16 object")
                object_rows.append({"source": str(source), "sha256": sha(obj)})
            binary, mapfile = out / (name + ".bin"), out / (name + ".map")
            run([tool / "binnt64/wlink.exe", "format", "raw", "bin", "option", "start=start_", "option", "nodefaultlibs",
                 "option", "map=" + str(mapfile), "name", binary, "file", ",".join(str(item) for item in objects)], name + "-link")
            linked = mapfile.read_text(encoding="utf-8", errors="replace")
            entry = re.search(r"Entry point address:\s*([0-9a-fA-F]+)", linked)
            if entry is None or ".lib" in linked.lower():
                raise ValueError("unexpected runtime library or missing entry")
            payload = binary.read_bytes()
            image = out / (name + ".img")
            image.write_bytes(loader.floppy(payload, int(entry.group(1), 16), verify_payload=True))
            original_hash = sha(image)
            ranges = [(0x10000 + offset, min(4096, len(payload) - offset)) for offset in range(0, len(payload), 4096)]
            expected_ram = {0x38ffc: "271000", 0x41000: "00", 0x42000: "9002"}
            if defect is None:
                ranges += [(address, len(bytes.fromhex(value))) for address, value in expected_ram.items()]
            # CLI inspection has a 16KiB total cap. A larger payload uses fresh
            # deterministic replays; every replay verifies the complete payload
            # checksum in the loader before executing its protected code bytes.
            groups, current, total = [], [], 0
            for request in ranges:
                if current and (len(current) == 16 or total + request[1] > 16384):
                    groups.append(current)
                    current, total = [], 0
                current.append(request)
                total += request[1]
            groups.append(current)
            memory, replays = [], []
            for replay, group in enumerate(groups):
                command = [emulator, "--machine", "pc286", "--floppy", image, "--cycles", "60000000",
                           "--timeout-ms", "25000", "--until-post", hex(marker)]
                for address, size in group:
                    command += ["--inspect", hex(address) + ":" + str(size)]
                result = json.loads(run(command, name + "-guest-" + str(replay)))
                memory.extend(result.pop("memory_ranges", None) or [result.pop("memory", {})])
                if result.get("status") != "condition_met" or result.get("post") != marker or result.get("cs") != 0x1000:
                    raise ValueError(name + ": unexpected guest stop")
                if replays and result != replays[0]:
                    raise ValueError(name + ": replay stopped in a different reported machine state")
                replays.append(result)
            if len(memory) != len(ranges) or any(row.get("address") != request[0] or
                    len(bytes.fromhex(row.get("bytes", ""))) != request[1] for row, request in zip(memory, ranges)):
                raise ValueError("invalid inspection result")
            loaded = b"".join(bytes.fromhex(row["bytes"]) for row in memory if 0x10000 <= row["address"] < 0x20000)
            if loaded != payload or sha(image) != original_hash:
                raise ValueError("loaded payload mismatch or source media changed")
            actual = {row["address"]: row["bytes"] for row in memory if row["address"] in expected_ram}
            if defect is None and actual != expected_ram:
                raise ValueError("physical data/stack bytes differ")
            observations.append({"case": name, "defect": defect, "expected_post": marker, "objects": object_rows,
                                 "payload_bytes": len(payload), "payload_sha256": sha(binary), "replays": replays,
                                 "floppy_sha256": original_hash, "loaded_payload_matches": True,
                                 "physical_ram": actual, "source_media_unchanged": True})
        if before != {path.relative_to(ROOT).as_posix(): sha(path) for path in sources}:
            raise ValueError("source/spec/helper changed during qualification")
        if tool_hashes != {name: sha(path) for name, path in tool_paths.items()}:
            raise ValueError("compiler/header/emulator changed during qualification")
        report = {"status": "pass", "source_sha256": before, "inputs_unchanged": True,
                  "tool_sha256": tool_hashes, "toolchain": lock, "commands": commands, "hosted": hosted,
                  "pc286": observations,
                  "limits": "Original native286 opt-in GetTrapAddress only. No Set, memory manager, protected program, physical target or System startup claim. Unoptimized case changes service-wrapper optimization only."}
        (out / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(json.dumps({"status": "pass", "hosted": len(hosted), "pc286": len(observations), "report": str(out / "report.json")}))
        return 0
    except (OSError, ValueError, KeyError, subprocess.TimeoutExpired) as exc:
        print("Banked service verification failed: " + str(exc), file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
