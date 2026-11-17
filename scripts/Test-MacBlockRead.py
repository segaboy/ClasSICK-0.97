#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Execute the original block-reader diagnostic natively on BootHush Mac128K."""
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
SOURCE = ROOT / "core/storage/block-read.c"


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--m68k-bin", required=True, type=Path)
    p.add_argument("--emulator", required=True, type=Path)
    p.add_argument("--output", required=True, type=Path)
    args = p.parse_args()
    try:
        spec = importlib.util.spec_from_file_location("block_m68k_tools", ROOT / "scripts/m68k_windows_toolchain.py")
        native = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(native)
        tools, identity = native.inspect(args.m68k_bin)
        exe = args.emulator.resolve(strict=True)
        out = args.output.resolve()
        if out == ROOT or ROOT in out.parents:
            raise ValueError("output must be a new directory outside the repository")
        out.mkdir(parents=True, exist_ok=False)
        commands = []
        env = os.environ.copy()
        env["PATH"] = str(Path(tools["gcc"]).parent) + os.pathsep + env.get("PATH", "")

        def run(command, label):
            command = [str(x) for x in command]
            commands.append(command)
            result = subprocess.run(command, capture_output=True, timeout=30, env=env,
                                    creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            (out / (label + ".stdout")).write_bytes(result.stdout)
            (out / (label + ".stderr")).write_bytes(result.stderr)
            if result.returncode:
                raise ValueError(label + " failed: see retained output")
            return result.stdout

        source = SOURCE.read_text(encoding="utf-8")
        mutations = {
            "early-copy": ("        if (io != CS_BLOCK_IO_OK) {",
                           "        output[0] = staging[0];\n        if (io != CS_BLOCK_IO_OK) {"),
            "skip-range": ("    if (lba >= device->blocks || count > device->blocks - lba)",
                           "    if (0)")}
        observed = []
        for name, optimize, marker in (("O0", "-O0", 0xb4), ("Os", "-Os", 0xb4),
                                       ("early-copy", "-Os", 0xe4), ("skip-range", "-Os", 0xe5)):
            core = SOURCE
            if name in mutations:
                old, replacement = mutations[name]
                if source.count(old) != 1:
                    raise ValueError("mutation anchor changed")
                core = out / (name + ".c")
                core.write_text(source.replace(old, replacement), encoding="utf-8")
            elf = out / (name + ".elf")
            raw = out / (name + ".raw")
            rom = out / (name + ".rom")
            run([tools["gcc"], "-mcpu=68000", "-std=c99", optimize, "-Wall", "-Wextra", "-Werror",
                 "-Wconversion", "-Wsign-conversion", "-ffreestanding", "-fno-builtin", "-nostdlib",
                 "-I", ROOT / "core/storage", "-Wl,--build-id=none", "-Wl,-T," + str(ROOT / "arch/m68k/mac-rom.ld"),
                 ROOT / "arch/m68k/mac-reset.S", ROOT / "probes/mac128k/block-read.c",
                 ROOT / "probes/storage/block-read.c", core, "-o", elf], name + "-link")
            undefined = run([tools["nm"], "-u", elf], name + "-imports")
            if undefined.strip():
                raise ValueError("unexpected native runtime imports")
            symbols = run([tools["nm"], "-n", elf], name + "-symbols").decode("utf-8").replace("\r\n", "\n")
            found = re.search(r"^([0-9a-fA-F]+) [BbDd] cs_mac_diag$", symbols, re.M)
            if not found:
                raise ValueError("missing diagnostic symbol")
            address = int(found.group(1), 16)
            if not 0xb00 <= address <= 0x2000 - 28:
                raise ValueError("diagnostic outside original reset data region")
            run([tools["objcopy"], "-O", "binary", elf, raw], name + "-objcopy")
            data = raw.read_bytes()
            if not 260 <= len(data) <= 65536:
                raise ValueError("unexpected original ROM payload size")
            rom.write_bytes(data + bytes([255]) * (65536 - len(data)))
            before = sha(rom)
            result = json.loads(run([exe, "--machine", "mac128k", "--rom", rom,
                                    "--cycles", "10000000", "--timeout-ms", "20000",
                                    "--until", hex(address) + ":4:" + hex(marker),
                                    "--inspect", hex(address) + ":28"], name + "-boot"))
            memory = result.pop("memory", {})
            actual = bytes.fromhex(memory.get("bytes", ""))
            if (result.get("status") != "condition_met" or memory.get("address") != address or
                    len(actual) != 28 or int.from_bytes(actual[:4], "big") != marker or
                    int.from_bytes(actual[24:28], "big") != 0 or sha(rom) != before):
                raise ValueError(name + ": wrong result, unexpected fault or altered input ROM")
            observed.append({"case": name, "expected_marker": marker, "result": result,
                             "diag_address": address, "diag_hex": actual.hex(),
                             "rom_sha256": before, "payload_bytes": len(data), "undefined": []})
        _, after = native.inspect(args.m68k_bin)
        if after != identity:
            raise ValueError("toolchain changed during qualification")
        inputs = [SOURCE, ROOT / "core/storage/block-read.h", ROOT / "probes/storage/block-read.c",
                  ROOT / "probes/storage/block-read.h", ROOT / "probes/mac128k/block-read.c",
                  ROOT / "arch/m68k/mac-reset.S", ROOT / "arch/m68k/mac-rom.ld",
                  ROOT / "scripts/m68k_windows_toolchain.py", Path(__file__)]
        report = {"status": "pass", "mac128k_observed": observed, "m68k_identity": identity,
                  "emulator_sha256": sha(exe), "commands": commands,
                  "source_sha256": {str(x.relative_to(ROOT)).replace("\\", "/"): sha(x) for x in inputs},
                  "limits": "Original RAM-provider diagnostics only. No floppy driver, MFS, System Disk boot or physical validation."}
        (out / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(json.dumps({"status": "pass", "cases": len(observed), "report": str(out / "report.json")}, indent=2))
        return 0
    except (OSError, ValueError, KeyError, subprocess.TimeoutExpired) as exc:
        print("Mac block-read verification failed: " + str(exc), file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
