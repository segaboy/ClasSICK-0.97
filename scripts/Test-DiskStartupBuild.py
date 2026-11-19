#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Twin build, install and cold-boot the disk-startup profile using generated sectors."""
import argparse
from importlib import import_module
import json
from pathlib import Path
import struct
import subprocess

from m68k_windows_toolchain import sha
from pc286_hard_disk_loader import DISK_BYTES

builder = import_module("Build-DiskStartup")
EMULATOR = "a00fb8b507d79b0f0b88026c2e9cd613a6a7bd78c3aae10858bb5735e4f6db98"


def fixture(value, trap=False):
    image = bytearray(409600)
    struct.pack_into(">HHh", image, 0, 0x4c4b, 0x6000, 510-4)
    # Run over 256 steps and copy a small marker-writing routine beyond the
    # loaded blocks. Only disk code can set the marker on either target.
    program = bytearray.fromhex("303c012c 51c8fffe")
    if trap:
        program += bytes.fromhex("7046 a146 23c800009104")  # Save GetTrapAddress(0x46).
    program += struct.pack(">HII", 0x23fc, 0x13fc0000 | value, 0x8800)
    program += struct.pack(">HII", 0x23fc, 0x00009100, 0x8804)
    program += bytes.fromhex("33fc60fe00008808 4ef900008800")
    image[510:510+len(program)] = program
    return image


def patched_fixture():
    image = fixture(0)
    # Install an original handler at $8300, read its address back, invoke it,
    # save its result and write the completion marker only after its return.
    main = bytes.fromhex("7017 207c00008300 a047 207c00000000 a146 23c800009104 "
                         "a017 23c000009100 13fc00ac00009108 60fe")
    image[510:510+len(main)] = main
    image[0x300:0x308] = bytes.fromhex("203c43535031 4e75")
    return image


def verify(args):
    out = args.output.resolve()
    if out == builder.ROOT or builder.ROOT in out.parents or sha(args.emulator) != EMULATOR:
        raise ValueError("need external output and qualified emulator")
    out.mkdir(parents=True, exist_ok=False)
    builds = [builder.build(args.m68k_bin, args.tool_root, args.assembler, out/name) for name in ("a", "b")]
    record = builds[0]
    bundle = out/"a"
    if (builds[0] != builds[1] or {f.name for f in bundle.iterdir()} !=
            {builder.ROM, builder.INSTALLER, "manifest.json"}):
        raise ValueError("developer packages differ")
    for name in record["artifacts"]:
        if sha(bundle/name) != sha(out/"b"/name) or sha(bundle/name) != record["artifacts"][name]:
            raise ValueError("artifact identity mismatch")
    rom, install = bundle/builder.ROM, bundle/builder.INSTALLER
    blank, installed = out/"blank.img", out/"installed.img"
    blank.write_bytes(bytes(DISK_BYTES))
    blank_sha = sha(blank)
    def guest(lane, options, *, cycles):
        cmd = [str(args.emulator), "--machine", lane, "--timeout-ms", "40000", "--cycles", str(cycles), *map(str, options)]
        proc = subprocess.run(cmd, text=True, capture_output=True, timeout=48,
                              creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        row = json.loads(proc.stdout)
        if proc.returncode or row.get("status") != "condition_met":
            raise ValueError(f"{lane} fixture missed stop ({proc.returncode}) {row}")
        return row
    keys = []
    for i, key in enumerate((0x43, 0x31, 0x1b, 0x2c, 0x1c, 0x4b, 0x4b, 0x5a)):
        for delta, value in ((0, key), (10000, 0xf0), (20000, key)):
            keys.extend(["--scancode", f"{10000000+i*100000+delta}:{value}"])
    guest("pc286", ["--floppy", install, "--hard-disk", blank, "--disk-out", installed,
                    "--until-post", "0xac", *keys], cycles=600000000)
    if sha(blank) != blank_sha or sha(installed) != record["pc"]["installed_disk_sha256"]:
        raise ValueError("installer output differs from expected entire disk")
    observations = []
    for name, value in (("first", 0x37), ("alternate", 0x82), ("retry", 0x37),
                        ("bad-header", 0x37), ("read-error", 0x37), ("get-trap", 0x54),
                        ("patched-trap", 0xac)):
        logical = patched_fixture() if name == "patched-trap" else fixture(value, name == "get-trap")
        if name == "bad-header":
            logical[0] ^= 1
        for lane in ("mac128k", "pc286"):
            pc = lane == "pc286"
            if name in ("get-trap", "patched-trap") and not pc:
                continue
            source = out/f"{name}-{lane}.img"
            source.write_bytes(logical.ljust(1228800, b"\0") if pc else logical)
            before = sha(source)
            opts = ["--boot-drive", "hard-disk", "--hard-disk", installed] if pc else ["--rom", rom]
            opts.extend(["--floppy", source])
            marker = 0x39100 if pc else 0x9100
            negative = name in ("bad-header", "read-error")
            if name == "retry":
                opts += (["--fail-hard-disk-read-once", "0:0:2"] if pc else ["--mac-floppy-bad-checksum-once", "0:0"])
            if name == "read-error":
                opts += (["--fail-floppy-read", "0:0:2"] if pc else ["--mac-floppy-bad-checksum", "0:0"])
            if negative:
                opts += (["--until-post", "0xd0"] if pc else ["--until-pc", record["mac"]["startup_stop_pc"]])
            else:
                stop = marker + 8 if name == "patched-trap" else marker
                opts += ["--until", f"{stop}:1:{value}"]
            opts += ["--inspect", f"{marker}:8"]
            row = guest(lane, opts, cycles=250000000 if pc else 16000000)
            view = row["memory"]
            expected = bytes([0 if negative else value]) + bytes(3) + (bytes.fromhex("00400000") if name == "get-trap" else bytes(4))
            if name == "patched-trap":
                expected = bytes.fromhex("4353503100008300")
            if view["address"] != marker or bytes.fromhex(view["bytes"]) != expected or sha(source) != before:
                raise ValueError(f"{lane} {name} guest result or source differs: {view}")
            if name in ("retry", "read-error"):
                if pc:
                    fault = row["hard_disk_read_failure" if name == "retry" else "floppy_read_failure"]
                    if fault["injected"] != 1:
                        raise ValueError("expected one PC firmware injection")
                elif row["mac_floppy_checksum_fault"]["altered_nibbles"] != (1 if name == "retry" else 2):
                    raise ValueError("Mac did not observe expected checksum fault count")
                if name == "retry" and row["cycles"] <= next(o["cycles"] for o in observations
                                                             if o["case"] == "first" and o["lane"] == lane):
                    raise ValueError("injection did not increase the cold-start cycle count")
            observations.append(dict(case=name, lane=lane, cycles=row["cycles"], marker=view["bytes"],
                                     source_sha256=before))
    if (sha(installed) != record["pc"]["installed_disk_sha256"] or sha(args.emulator) != EMULATOR or
            any(sha(bundle/n) != h for n, h in record["artifacts"].items())):
        raise ValueError("tested runtime or emulator changed")
    report = dict(status="generated-controls-pass", manifest_sha256=sha(bundle/"manifest.json"),
                  installed_sha256=sha(installed), emulator_sha256=EMULATOR, observations=observations)
    (out/"report.json").write_text(json.dumps(report, indent=2)+"\n", encoding="utf-8")
    print(report["status"])


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("m68k-bin", "tool-root", "assembler", "emulator", "output"):
        parser.add_argument("--"+name, type=Path, required=True)
    opts = parser.parse_args()
    for name in ("m68k_bin", "tool_root", "assembler", "emulator"):
        setattr(opts, name, getattr(opts, name).resolve(strict=True))
    verify(opts)
