#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Qualify the original bounded block reader; no external disk/media inputs."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "core/storage/block-read.c"
HEADER = ROOT / "core/storage/block-read.h"
TEST = ROOT / "tests/block-read.c"
PROBE = ROOT / "probes/pc286/boot-block-read.c"
SHARED_PROBE = ROOT / "probes/storage/block-read.c"


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cc", required=True, type=Path)
    parser.add_argument("--m68k-bin", required=True, type=Path)
    parser.add_argument("--tool-root", required=True, type=Path)
    parser.add_argument("--emulator", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    try:
        h = module("block_helpers", ROOT / "scripts/Test-BootHushPC286BankPlan.py")
        native = module("block_m68k", ROOT / "scripts/m68k_windows_toolchain.py")
        cc = args.cc.resolve(strict=True)
        tool = args.tool_root.resolve(strict=True)
        emulator = args.emulator.resolve(strict=True)
        out = args.output.resolve()
        if out == ROOT or ROOT in out.parents:
            raise ValueError("output must be a new directory outside the repository")
        lock_path = ROOT / "scripts/pc286-toolchain-lock.json"
        lock = json.loads(lock_path.read_text(encoding="utf-8"))
        for member in lock["selected_members"]:
            path = tool / member["path"]
            if path.stat().st_size != member["bytes"] or h.digest(path) != member["sha256"]:
                raise ValueError("pinned compiler identity mismatch: " + member["path"])
        m68k, identity = native.inspect(args.m68k_bin)
        out.mkdir(parents=True, exist_ok=False)
        env = os.environ.copy()
        env["PATH"] = str(cc.parent) + os.pathsep + env.get("PATH", "")
        for key in ("WCC", "WATCOM", "INCLUDE", "LIB", "WCL", "WLINK"):
            env.pop(key, None)
        commands = []

        def run(command, name, timeout=30):
            commands.append([str(x) for x in command])
            return h.run(command, out, env, out / (name + ".log"), timeout)

        warning = ["-Wall", "-Wextra", "-Werror", "-Wconversion", "-Wsign-conversion"]
        hosted = []
        for name, flags in (("release", ["-O3", "-DNDEBUG"]),
                            ("sanitized", ["-O1", "-fsanitize=address,undefined",
                                           "-fno-sanitize-recover=all", "-fno-omit-frame-pointer"])):
            binary = out / (name + ".exe")
            run([cc, "-std=c11", *warning, *flags, SOURCE, TEST, "-o", binary], name + "-build")
            output = run([binary], name + "-test").decode("utf-8").strip()
            if not re.fullmatch(r"block-read: [0-9]+ checks passed", output):
                raise ValueError("missing host test result")
            hosted.append({"profile": name, "result": output, "sha256": h.digest(binary)})

        m68k_objects = []
        for optimize in ("-O0", "-Os"):
            obj = out / ("m68k-" + optimize[1:] + ".o")
            run([m68k["gcc"], "-mcpu=68000", "-std=c99", *warning,
                 "-ffreestanding", "-fno-builtin", "-nostdlib", "-fstack-usage", optimize,
                 "-c", SOURCE, "-o", obj], "m68k-" + optimize[1:] + "-compile")
            undefined = run([m68k["nm"], "-u", obj], "m68k-" + optimize[1:] + "-undefined")
            if undefined.strip():
                raise ValueError("unexpected 68000 runtime import")
            symbols = run([m68k["nm"], "-S", "--defined-only", obj],
                          "m68k-" + optimize[1:] + "-symbols").decode("utf-8")
            text_size = re.search(r"^[0-9a-fA-F]+\s+([0-9a-fA-F]+)\s+[Tt]\s+cs_block_read\s*$",
                                  symbols, re.M)
            if text_size is None:
                raise ValueError("missing 68000 function size")
            frame = obj.with_suffix(".su").read_text(encoding="utf-8").strip()
            m68k_objects.append({"optimization": optimize, "sha256": h.digest(obj), "undefined": [],
                                 "text_bytes": int(text_size.group(1), 16), "compiler_frame": frame})

        wcc, wlink = tool / "binnt64/wcc.exe", tool / "binnt64/wlink.exe"
        c16_flags = ["-zq", "-2", "-zl", "-s", "-os", "-zastd=c99", "-we",
                     "-dCS_C16_REQUIRE_16BIT", "-i=" + str(tool / "h"),
                     "-i=" + str(HEADER.parent)]
        c16 = []
        for model in ("ms", "mc"):
            obj = out / (model + ".obj")
            run([wcc, *c16_flags, "-" + model, "-fo=" + str(obj), SOURCE], model + "-compile")
            metadata = run([tool / "binnt64/dmpobj.exe", obj], model + "-object")
            if b"80286" not in metadata or b"USE16" not in metadata:
                raise ValueError("missing 80286/USE16 object metadata")
            if re.search(rb"\b(?:EXTDEF|LEXTDEF|COMDEF|LCOMDEF)\(", metadata):
                raise ValueError("unexpected C16 external import or common symbol")
            lengths = [int(x, 16) for x in re.findall(rb"USE16 Length ([0-9a-fA-F]+)h", metadata)]
            if not lengths or any(lengths[1:]):
                raise ValueError("unexpected C16 segment layout")
            c16.append({"model": model, "sha256": h.digest(obj), "metadata_80286_use16": True,
                        "text_bytes": lengths[0], "static_data_bytes": 0, "undefined": []})

        original = SOURCE.read_text(encoding="utf-8")
        mutations = {
            "early-copy": ("        if (io != CS_BLOCK_IO_OK) {",
                           "        output[0] = staging[0];\n        if (io != CS_BLOCK_IO_OK) {", 0xe4),
            "skip-range": ("    if (lba >= device->blocks || count > device->blocks - lba)",
                           "    if (0)", 0xe5),
        }
        probe = out / "probe.obj"
        shared = out / "shared-probe.obj"
        run([wcc, *c16_flags, "-ms", "-fo=" + str(shared), SHARED_PROBE], "shared-probe-compile")
        run([wcc, *c16_flags, "-ms", "-fo=" + str(probe), PROBE], "probe-compile")
        observed = []
        for name, marker in (("baseline", 0xb4), ("early-copy", 0xe4), ("skip-range", 0xe5)):
            obj = out / "ms.obj"
            if name != "baseline":
                old, replacement, expected = mutations[name]
                if expected != marker or original.count(old) != 1:
                    raise ValueError("mutation anchor changed")
                mutant = out / (name + ".c")
                mutant.write_text(original.replace(old, replacement), encoding="utf-8")
                obj = out / (name + ".obj")
                run([wcc, *c16_flags, "-ms", "-fo=" + str(obj), mutant], name + "-compile")
            binary, mapping = out / (name + ".bin"), out / (name + ".map")
            run([wlink, "format", "raw", "bin", "option", "start=start_", "option", "nodefaultlibs",
                 "option", "map=" + str(mapping), "name", binary,
                 "file", str(obj) + "," + str(shared) + "," + str(probe)], name + "-link")
            map_text = mapping.read_text(encoding="utf-8", errors="replace")
            entry = re.search(r"Entry point address:\s*([0-9a-fA-F]+)", map_text)
            if entry is None or ".lib" in map_text.lower():
                raise ValueError("missing entry or unexpected runtime library")
            payload = binary.read_bytes()
            if len(payload) > 4096:
                raise ValueError("diagnostic exceeds full-payload inspection limit")
            image = out / (name + ".img")
            with image.open("xb") as stream:
                stream.write(h.floppy(payload, int(entry.group(1), 16)))
            before = h.digest(image)
            response = json.loads(run([emulator, "--machine", "pc286", "--floppy", image,
                                      "--cycles", "8000000", "--timeout-ms", "20000",
                                      "--until-post", hex(marker), "--inspect", "0x10000:" + str(len(payload))],
                                     name + "-boot"))
            memory = response.pop("memory", {})
            if (response.get("status") != "condition_met" or response.get("post") != marker or
                    response.get("cs") != 0x1000 or h.digest(image) != before or
                    memory.get("address") != 0x10000 or bytes.fromhex(memory.get("bytes", "")) != payload):
                raise ValueError(name + ": wrong target result, loaded payload or changed input image")
            observed.append({"case": name, "expected_post": marker, "result": response,
                             "payload_bytes": len(payload), "payload_sha256": h.digest(binary), "image_sha256": before,
                             "loaded_payload_matches": True, "image_unchanged": True})
        _, after = native.inspect(args.m68k_bin)
        if after != identity:
            raise ValueError("68000 frontend identity changed")
        report = {"status": "pass", "hosted": hosted, "m68k_identity": identity,
                  "m68k_compile_only": m68k_objects, "c16_compile": c16, "pc286_observed": observed,
                  "cc_sha256": h.digest(cc), "emulator_sha256": h.digest(emulator),
                  "pc286_lock_sha256": h.digest(lock_path), "commands": commands,
                  "source_sha256": {str(p.relative_to(ROOT)).replace("\\", "/"): h.digest(p)
                                    for p in (SOURCE, HEADER, TEST, PROBE, SHARED_PROBE,
                                              ROOT / "probes/storage/block-read.h", Path(__file__),
                                              ROOT / "scripts/Test-BootHushPC286BankPlan.py",
                                              ROOT / "scripts/m68k_windows_toolchain.py")},
                  "limits": "RAM providers only; 68000 compile-only; original 286 emulator diagnostic. "
                            "No general segmented backend, disk driver, MFS, System boot or physical proof."}
        (out / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(json.dumps({"status": "pass", "hosted": hosted, "pc286_cases": len(observed),
                          "report": str(out / "report.json")}, indent=2))
        return 0
    except (OSError, ValueError, KeyError, subprocess.TimeoutExpired) as exc:
        print("block-read verification failed: " + str(exc), file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
