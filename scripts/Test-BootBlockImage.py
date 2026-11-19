#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Run the independently authored two-block program on Mac128K and native 286."""
import argparse
from importlib import import_module
import json
import os
from pathlib import Path
import re
import struct
import subprocess

from m68k_windows_toolchain import inspect, sha
from pc286_hard_disk_loader import DISK_BYTES

ROOT = Path(__file__).resolve().parents[1]
BOOT = ["core/storage/" + name + ".c" for name in ("block-read", "mac-boot-header", "mac-boot-image")]
MAC = [*BOOT, "arch/m68k/mac-reset.S", "platform/mac128k/iwm-read.c",
       "platform/mac128k/iwm-capture.S", "probes/mac128k/boot-image.c"]
PC = [*BOOT, "core/cpu/m68k-bus.c", "core/cpu/m68k.c", "core/memory/guest.c",
      "core/services/traps.c", "platform/pc286/banked-guest.c", "platform/pc286/m68k-banked.c",
      "platform/pc286/boothush-floppy-read.c", "probes/pc286/boot-image.c"]
PIN = "5fa45b7542409300b7604606a78c81d4ee95e888a57c592a83d0cec43ce1056e"


def fixture(marker):
    data = bytearray(409600)
    data[:6] = struct.pack(">HHh", 0x4c4b, 0x6000, 510 - 4)
    # MOVE.L #marker,D0 and RTS. The operand begins in the second sector.
    struct.pack_into(">HIH", data, 510, 0x203c, marker, 0x4e75)
    data[1023] = 0xb9
    return data


def symbol(pattern, text):
    match = re.search(pattern, text, re.M)
    if not match:
        raise ValueError("missing symbol: " + pattern)
    return int(match.group(1), 16)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for key in ("emulator", "m68k-bin", "tool-root", "assembler", "cc", "output"):
        parser.add_argument("--" + key, type=Path, required=True)
    args = parser.parse_args()
    out = args.output.resolve()
    if out == ROOT or ROOT in out.parents or sha(args.emulator) != PIN:
        raise ValueError("need external fresh output and the admitted BootHush build")
    out.mkdir(parents=True, exist_ok=False)
    m68k, identity = inspect(args.m68k_bin)
    tool = args.tool_root.resolve(strict=True)
    for row in json.loads((ROOT / "scripts/pc286-toolchain-lock.json").read_text())["selected_members"]:
        if sha(tool / row["path"]) != row["sha256"]:
            raise ValueError("C16 tool mismatch: " + row["path"])
    env = os.environ.copy()
    cc = args.cc.resolve(strict=True)
    env["PATH"] = os.pathsep.join((str(m68k["gcc"].parent), str(cc.parent), env["PATH"]))
    for var in ("WCC", "WATCOM", "INCLUDE", "LIB", "WCL", "WLINK"):
        env.pop(var, None)
    sources = set(MAC + PC)
    # Include all directly or transitively included local headers.
    pending = list(sources)
    while pending:
        p = ROOT / pending.pop()
        for include in re.findall(r'^\s*#include\s+"([^"]+)"', p.read_text(), re.M):
            path = (p.parent / include).resolve().relative_to(ROOT).as_posix()
            if path not in sources:
                sources.add(path)
                pending.append(path)
    sources.update(["arch/m68k/mac-rom.ld", "scripts/m68k_windows_toolchain.py",
                    "scripts/pc286_loader.py", "scripts/pc286-toolchain-lock.json",
                    "scripts/Build-PC286PreviewMedia.py", "scripts/pc286_hard_disk_loader.py",
                    "platform/pc286/preview-install.asm", "platform/pc286/preview-boot.asm",
                    "tests/mac-boot-image.c",
                    "scripts/Test-BootBlockImage.py", "docs/specifications/SPEC-0130-boot-block-image.md"])
    def source_hashes():
        return {path: sha(ROOT / path) for path in sorted(sources)}
    before = source_hashes()
    commands = []
    def run(cmd):
        commands.append([str(x) for x in cmd])
        result = subprocess.run(commands[-1], cwd=out, capture_output=True, env=env, timeout=45,
                                creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        if result.returncode:
            raise RuntimeError(result.stderr.decode(errors="replace") + result.stdout.decode(errors="replace"))
        return result.stdout.decode()

    hosted = []
    for profile, flags in (("release", ["-O3"]), ("sanitized", ["-O1", "-fsanitize=address,undefined",
                                                                 "-fno-sanitize-recover=all"])):
        binary = out / (profile + ".exe")
        run([cc, "-std=c11", *flags, "-Werror", "-Wall", "-Wextra", "-Wconversion", "-Wsign-conversion",
             *(ROOT / p for p in BOOT), ROOT / "tests/mac-boot-image.c", "-o", binary])
        hosted.append({"profile": profile, "result": run([binary]).strip(), "sha256": sha(binary)})
    elf, macbin, rom = [out / ("mac." + ext) for ext in ("elf", "bin", "rom")]
    run([m68k["gcc"], "-mcpu=68000", "-std=c99", "-Os", "-Wall", "-Wextra", "-Werror",
         "-ffreestanding", "-fno-builtin", "-nostdlib", "-Wl,--build-id=none",
         "-Wl,-T," + str(ROOT / "arch/m68k/mac-rom.ld"), *(ROOT / p for p in MAC), "-o", elf])
    if run([m68k["nm"], "-u", elf]).strip():
        raise ValueError("unresolved Mac symbols")
    macmap = run([m68k["nm"], "-n", elf]).replace("\r\n", "\n")
    macdiag = symbol(r"^([0-9a-fA-F]+) [BD] cs_mac_diag$", macmap)
    macimage = symbol(r"^([0-9a-fA-F]+) [BD] cs_boot_image$", macmap)
    run([m68k["objcopy"], "-O", "binary", elf, macbin])
    contents = macbin.read_bytes()
    if len(contents) > 65536:
        raise ValueError("ROM overflow")
    rom.write_bytes(contents.ljust(65536, b"\xff"))
    objects = []
    for i, path in enumerate(PC):
        obj = out / ("p" + str(i) + ".obj")
        run([tool / "binnt64/wcc.exe", "-zq", "-2", "-ms", "-zl", "-s", "-os", "-zastd=c99",
             "-we", "-dCS_C16_REQUIRE_16BIT", "-i=" + str(tool / "h"), "-fo=" + str(obj), ROOT / path])
        objects.append(obj)
    pcbin, pcmap = out / "pc.bin", out / "pc.map"
    run([tool / "binnt64/wlink.exe", "format", "raw", "bin", "option", "start=start_", "option",
         "nodefaultlibs", "option", "map=" + str(pcmap), "name", pcbin, "file",
         ",".join(str(p) for p in objects)])
    mapping = pcmap.read_text()
    bss = re.search(r"^_BSS\s+BSS\s+DGROUP\s+([\da-f]+)\s+([\da-f]+)", mapping, re.I | re.M)
    if not bss or ".lib" in mapping.lower():
        raise ValueError("invalid PC link")
    entry = symbol(r"Entry point address:\s*([\da-fA-F]+)", mapping)
    def pcaddr(name):
        return 0x10000 + symbol(r"^\s*([\da-fA-F]+)[+*]?\s+_" + name + r"\s*$", mapping)
    pcdiag, pcimage = pcaddr("cs_boot_report"), pcaddr("cs_boot_image")
    payload = pcbin.read_bytes()
    installer = import_module("Build-PC286PreviewMedia").build_install_media(
        payload, entry, sum(int(x, 16) for x in bss.groups()), args.assembler,
        tool / "binnt64/wlink.exe", out / "installer")
    install_media = out / "installer/install-floppy.img"
    blank, installed = out / "blank.img", out / "installed.img"
    blank.write_bytes(bytes(DISK_BYTES))
    blank_sha = sha(blank)
    keys = []
    for index, key in enumerate((0x43, 0x31, 0x1b, 0x2c, 0x1c, 0x4b, 0x4b, 0x5a)):
        tick = 10000000 + index * 100000
        for delta, value in ((0, key), (10000, 0xf0), (20000, key)):
            keys += ["--scancode", f"{tick+delta}:{value}"]
    copying = json.loads(run([args.emulator, "--machine", "pc286", "--floppy", install_media,
              "--hard-disk", blank, "--disk-out", installed, "--cycles", "600000000",
              "--timeout-ms", "30000", "--until-post", "0xac", *keys]))
    installed_sha = sha(installed)
    if (copying["status"] != "condition_met" or copying["post"] != 0xac or
            installed_sha != installer["images"]["hard-disk.img"] or sha(blank) != blank_sha or
            sha(install_media) != installer["images"]["install-floppy.img"]):
        raise ValueError("install confirmation, original inputs or whole output disk differs")

    observations = []
    for label, result, marker in [("healthy", 0, 0x1256abcd), ("alternate", 0, 0xaab16798),
            ("bad-signature", 3, 0), ("bad-name", 4, 0), ("second-sector-fault", 6, 0),
            ("cold-recovery", 0, 0x1256abcd)]:
        logical = fixture(marker or 0x1256abcd)
        if label == "bad-signature":
            logical[1] ^= 1
        if label == "bad-name":
            logical[10] = 16
        for lane in ("mac128k", "pc286"):
            mac = lane == "mac128k"
            media = out / (lane + "-" + label + ".img")
            image = logical if mac else logical + bytes(1600*512)
            media.write_bytes(image)
            hashes = {p: sha(p) for p in ([media, rom] if mac else [media, installed])}
            cmd = [args.emulator, "--machine", lane, "--floppy", media,
                   "--cycles", "400000000", "--timeout-ms", "30000"]
            if mac:
                cmd += ["--rom", rom, "--until", hex(macdiag) + ":4:0xb1"]
            else:
                cmd += ["--hard-disk", installed, "--boot-drive", "hard-disk", "--until-post", "0xb1"]
            diag_address, image_address = (macdiag, macimage) if mac else (pcdiag, pcimage)
            cmd += ["--inspect", hex(diag_address) + (":28" if mac else ":16"),
                    "--inspect", hex(image_address) + ":1024"]
            if label == "second-sector-fault":
                cmd += ["--mac-floppy-bad-checksum", "0:1"] if mac else ["--fail-floppy-read", "0:0:2"]
            guest = json.loads(run(cmd))
            if guest.get("status") != "condition_met" or (not mac and
                    (guest.get("cs") != 0x1000 or guest.get("post") != 0xb1)):
                raise ValueError(f"{lane} {label} did not reach test termination: {guest}")
            diag, loaded = guest["memory_ranges"]
            words = struct.unpack(">7I" if mac else "<4I", bytes.fromhex(diag["bytes"]))
            expected = (0xb1, result, marker, int(result == 0))
            if (words[:4] != expected or (mac and words[6] != 0) or diag["address"] != diag_address
                    or loaded["address"] != image_address or bytes.fromhex(loaded["bytes"]) !=
                    (logical[:1024] if not result else bytes(1024))):
                raise ValueError(f"{lane} {label}: report {words}, expected {expected}, image or status mismatch")
            if label == "second-sector-fault":
                if mac:
                    fault = guest["mac_floppy_checksum_fault"]
                    if fault["track"] != 0 or fault["sector"] != 1 or not fault["altered_nibbles"]:
                        raise ValueError("missing Mac second-sector fault")
                else:
                    fault = guest["floppy_read_failure"]
                    if [fault[k] for k in ("cylinder", "head", "sector", "injected")] != [0, 0, 2, 1]:
                        raise ValueError("missing PC second-sector fault")
            if not mac and label == "healthy":
                # A second cold run inspects the installed runtime itself without
                # exceeding the observation window used for the 1024-byte image.
                verification = cmd[:cmd.index("--inspect")]
                chunks = [payload[i:i+4096] for i in range(0, len(payload), 4096)]
                for i, chunk in enumerate(chunks):
                    verification += ["--inspect", f"{0x10000 + i*4096}:{len(chunk)}"]
                loaded_code = json.loads(run(verification))
                ranges = loaded_code["memory_ranges"]
                if (loaded_code["status"] != "condition_met" or loaded_code["cycles"] != guest["cycles"]
                        or len(chunks) != len(ranges) or any(r["address"] != 0x10000 + i*4096 or
                        bytes.fromhex(r["bytes"]) != chunks[i] for i, r in enumerate(ranges))):
                    raise ValueError("loaded installed native runtime differs")
            if any(sha(p) != h for p, h in hashes.items()):
                raise ValueError("input changed")
            observations.append(dict(lane=lane, case=label, report=words, source_media_sha256=hashes[media],
                                     cycles=guest["cycles"]))
    for lane in ("mac128k", "pc286"):
        healthy, recovered = [row for row in observations if row["lane"] == lane and
                              row["case"] in ("healthy", "cold-recovery")]
        if any(healthy[key] != recovered[key] for key in ("report", "source_media_sha256", "cycles")):
            raise ValueError("fresh recovery differs on " + lane)
    if before != source_hashes() or sha(args.emulator) != PIN:
        raise ValueError("verification inputs changed")
    receipt = dict(status="pass", source_sha256=before, emulator_sha256=PIN, m68k_toolchain=identity,
                   installed_sha256=installed_sha, install_media=installer, install_cycles=copying["cycles"],
                   host_cc_sha256=sha(cc), host_tests=hosted,
                   mac_rom_sha256=sha(rom), pc_payload_sha256=sha(pcbin),
                   mac_payload_bytes=len(contents), pc_payload_bytes=pcbin.stat().st_size,
                   cases=observations, commands=commands)
    (out / "report.json").write_text(json.dumps(receipt, indent=2) + "\n")
    print(f"PASS: {len(observations)} native startup-image trials")


if __name__ == "__main__":
    main()
