#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Build and test original Mac IWM block-provider ROMs; no external media accepted."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
PINNED_EMULATOR = "7adb8a74cef009b7c532df24458c3a5db6b77725356fdb31e5ac4966d7ed971a"
PROVIDER = ROOT / "platform/mac128k/iwm-read.c"


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def original_media():
    data = bytes((lba * 37 + offset * 29 + (offset >> 3)) & 255
                 for lba in range(800) for offset in range(512))
    tags = bytes((lba * 11 + offset * 7 + 3) & 255 for lba in range(800) for offset in range(12))

    def container_sum(payload):
        acc = 0
        for offset in range(0, len(payload), 2):
            acc = (acc + int.from_bytes(payload[offset:offset + 2], "big")) & 0xffffffff
            acc = ((acc & 1) << 31) | (acc >> 1)
        return acc

    header = bytearray(84)
    name = b"ClasSICK original IWM test"
    header[0] = len(name)
    header[1:1 + len(name)] = name
    struct.pack_into(">IIII", header, 64, len(data), len(tags), container_sum(data), container_sum(tags[12:]))
    header[80:84] = bytes([0, 2, 1, 0])
    return data, bytes(header) + data + tags


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--m68k-bin", required=True, type=Path)
    parser.add_argument("--emulator", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    try:
        sources = [PROVIDER, ROOT / "platform/mac128k/iwm-read.h", ROOT / "probes/mac128k/floppy-read.c",
                   ROOT / "core/storage/block-read.c", ROOT / "core/storage/block-read.h",
                   ROOT / "platform/mac128k/iwm-capture.S", ROOT / "arch/m68k/mac-reset.S",
                   ROOT / "arch/m68k/mac-rom.ld", ROOT / "scripts/m68k_windows_toolchain.py",
                   ROOT / "docs/specifications/SPEC-0114-boothush-mac-floppy-provider.md", Path(__file__)]
        source_before = {str(path.relative_to(ROOT)): sha(path) for path in sources}
        loader = importlib.util.spec_from_file_location("mac_iwm_tools", ROOT / "scripts/m68k_windows_toolchain.py")
        native = importlib.util.module_from_spec(loader)
        loader.loader.exec_module(native)
        tools, identity = native.inspect(args.m68k_bin)
        emulator = args.emulator.resolve(strict=True)
        if sha(emulator) != PINNED_EMULATOR:
            raise ValueError("emulator does not match this original fixture admission")
        out = args.output.resolve()
        if out == ROOT or ROOT in out.parents:
            raise ValueError("output must be a fresh directory outside the repository")
        out.mkdir(parents=True, exist_ok=False)
        media = {}
        for name, data in zip(("raw", "dc42"), original_media()):
            path = out / ("original." + name)
            path.write_bytes(data)
            media[name] = path
        env = os.environ.copy()
        env["PATH"] = str(Path(tools["gcc"]).parent) + os.pathsep + env.get("PATH", "")
        commands = []

        def run(command, label):
            command = [str(item) for item in command]
            commands.append(command)
            process = subprocess.run(command, capture_output=True, env=env, timeout=40,
                                     creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            (out / (label + ".stdout")).write_bytes(process.stdout)
            (out / (label + ".stderr")).write_bytes(process.stderr)
            if process.returncode:
                raise ValueError(label + " failed; see retained stdout/stderr")
            return process.stdout

        observations = []
        healthy_command = None
        healthy_inputs = None
        source = PROVIDER.read_text(encoding="utf-8")
        cases = (("raw", 0, "raw", None), ("tags", 0, "dc42", None),
                 ("fault-raw", 1, "raw", None), ("fault-tags", 1, "dc42", None),
                 ("no-disk", 2, None, None), ("mapping-defect", 0, "raw", "mapping"),
                 ("checksum-defect", 1, "raw", "checksum"))
        for name, mode, format_name, mutation in cases:
            c_source = PROVIDER
            if mutation:
                old, replacement = {
                    "mapping": ("uint32_t relative = lba;", "uint32_t relative = lba ^ 1u;"),
                    "checksum": ("if (check[i] != sums[i]) return CS_MAC_IWM_CHECKSUM;",
                                 "if (0 && check[i] != sums[i]) return CS_MAC_IWM_CHECKSUM;")}[mutation]
                if source.count(old) != 1:
                    raise ValueError("mutation anchor differs")
                c_source = out / (name + ".c")
                c_source.write_text(source.replace(old, replacement), encoding="utf-8")
            elf, raw, rom = (out / (name + suffix) for suffix in (".elf", ".bin", ".rom"))
            run([tools["gcc"], "-mcpu=68000", "-std=c99", "-Os", "-Wall", "-Wextra", "-Werror",
                 "-Wconversion", "-Wsign-conversion", "-ffreestanding", "-fno-builtin", "-nostdlib",
                 "-DPROBE_CASE=" + str(mode), "-DEXPECT_TAGS=" + str(int(format_name == "dc42")),
                 "-I", ROOT / "platform/mac128k", "-Wl,--build-id=none",
                 "-Wl,-T," + str(ROOT / "arch/m68k/mac-rom.ld"), ROOT / "arch/m68k/mac-reset.S",
                 ROOT / "core/storage/block-read.c", c_source, ROOT / "platform/mac128k/iwm-capture.S",
                 ROOT / "probes/mac128k/floppy-read.c",
                 "-o", elf], name + "-build")
            if run([tools["nm"], "-u", elf], name + "-imports").strip():
                raise ValueError("unexpected runtime dependency")
            symbols = run([tools["nm"], "-n", elf], name + "-symbols").decode("utf-8")
            match = re.search(r"^([0-9a-fA-F]+) [BbDd] cs_mac_diag\s*$", symbols, re.M)
            if match is None:
                raise ValueError("no native diagnostic symbol")
            address = int(match.group(1), 16)
            run([tools["objcopy"], "-O", "binary", elf, raw], name + "-objcopy")
            payload = raw.read_bytes()
            if not 260 <= len(payload) <= 65536:
                raise ValueError("ROM payload outside admitted size")
            rom.write_bytes(payload + bytes([255]) * (65536 - len(payload)))
            expected = 0xf7 if mutation else 0xb7
            inputs = [rom] + ([media[format_name]] if format_name else [])
            before = {str(path): sha(path) for path in inputs}
            command = [emulator, "--machine", "mac128k", "--rom", rom, "--cycles", "400000000",
                       "--timeout-ms", "30000", "--until", hex(address) + ":4:" + hex(expected),
                       "--inspect", hex(address) + ":28", "--mac-floppy-state", "1", "--mac-cpu-state", "1"]
            if format_name:
                command += ["--floppy", media[format_name]]
            if mode == 1:
                command += ["--mac-floppy-bad-checksum", "0:1"]
            result = json.loads(run(command, name + "-guest"))
            observed = bytes.fromhex(result.get("memory", {}).get("bytes", ""))
            words = list(struct.unpack(">7I", observed)) if len(observed) == 28 else []
            if result.get("status") != "condition_met" or not words or words[0] != expected or words[6] != 0:
                raise ValueError(name + ": wrong guest result")
            if mode == 1:
                fault = result.get("mac_floppy_checksum_fault", {})
                if (fault.get("kind") != "synthetic_gcr_data_checksum" or fault.get("track") != 0 or
                        fault.get("sector") != 1 or fault.get("altered_nibbles", 0) < 1):
                    raise ValueError(name + ": missing observed selected checksum injection")
            if not mutation and mode == 0:
                if words[4] != 16 or result.get("mac_floppy", {}).get("track") != 0:
                    raise ValueError(name + ": missing completed reads or backward seek")
            if name == "raw":
                healthy_command, healthy_inputs = command[:], before.copy()
            if before != {str(path): sha(path) for path in inputs}:
                raise ValueError("an input image changed")
            observations.append({"case": name, "mode": mode, "defect": mutation, "diag": words,
                                 "inputs": before, "payload_bytes": len(payload), "result": result})
        recovered = json.loads(run(healthy_command, "raw-fresh-recovery-guest"))
        if recovered != observations[0]["result"]:
            raise ValueError("fresh healthy run did not reproduce baseline exactly")
        if healthy_inputs != {name: sha(Path(name)) for name in healthy_inputs}:
            raise ValueError("healthy input changed during fault/recovery controls")
        observations.append({"case": "raw-fresh-recovery", "mode": 0, "defect": None,
                             "inputs": healthy_inputs, "result": recovered,
                             "identical_to_raw": True})
        _, after = native.inspect(args.m68k_bin)
        if after != identity or sha(emulator) != PINNED_EMULATOR:
            raise ValueError("tool identities changed")
        source_after = {str(path.relative_to(ROOT)): sha(path) for path in sources}
        if source_after != source_before:
            raise ValueError("source, specification or build helper changed during qualification")
        report = {"status": "pass", "cases": observations, "commands": commands, "tools": identity,
                  "emulator_sha256": PINNED_EMULATOR,
                  "source_sha256": source_before, "source_unchanged": True,
                  "limits": "Original model-only read provider. No historical disk, physical drive, writes, filesystem or boot compatibility claim."}
        (out / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(json.dumps({"status": "pass", "cases": len(observations), "report": str(out / "report.json")}))
        return 0
    except (OSError, ValueError, KeyError, subprocess.TimeoutExpired) as exc:
        print("Mac IWM verification failed: " + str(exc), file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
