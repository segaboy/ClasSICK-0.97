#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Build the paired experimental handoff, qualify synthetic controls, or run a bounded private trial."""
import argparse
from importlib import import_module
import json
import os
from pathlib import Path
import re
import struct
import subprocess
from tempfile import TemporaryDirectory

from m68k_windows_toolchain import inspect, sha
from pc286_hard_disk_loader import DISK_BYTES

original = import_module("Test-BootBlockImage")
ROOT = original.ROOT
PC = original.PC[:-1] + ["personality/system1/startup-globals.c", "probes/pc286/boot-handoff.c"]
MAC = original.MAC[:-1] + ["personality/system1/startup-globals.c", "probes/mac128k/boot-handoff.c",
                           "probes/mac128k/boot-handoff-entry.S"]
PIN = "c709c7f10b5f8e2a908cc2f06a74192269073dda52e8aa4f7d23a80dac89ac28"
SELECTED = "1e03b75a05bbf441ae4e1808f70e38ffe0a07e3cc3298de151c0d2775552c647"


def dependencies():
    files = set(PC + MAC + ["arch/m68k/mac-rom.ld", "platform/pc286/preview-boot.asm",
        "platform/pc286/preview-install.asm", "scripts/Test-BootBlockImage.py",
        "scripts/Build-PC286PreviewMedia.py", "scripts/pc286_hard_disk_loader.py", "scripts/pc286_loader.py",
        "scripts/pc286-toolchain-lock.json", "scripts/m68k_windows_toolchain.py", "scripts/Test-MfsVolumeMedia.py",
        "scripts/Test-BootHandoff.py", "docs/specifications/SPEC-0133-boot-handoff.md"])
    queue = list(PC + MAC)
    while queue:
        file = ROOT / queue.pop()
        for local in re.findall(r'^\s*#include\s+"([^"]+)"', file.read_text(), re.M):
            path = (file.parent / local).resolve().relative_to(ROOT).as_posix()
            if path not in files:
                files.add(path)
                queue.append(path)
    return {path: sha(ROOT / path) for path in sorted(files)}


def fresh(path):
    if ROOT == path or ROOT in path.parents:
        raise ValueError("use a new external output directory")
    path.mkdir(parents=True, exist_ok=False)


def guest(exe, lane, options, cycles=16000000):
    # The raw CLI response may contain an unsupported opcode on failure. Never
    # persist or interpolate stdout/stderr, including in exception messages.
    run = subprocess.run([exe, "--machine", lane, *options, "--cycles", str(cycles), "--timeout-ms", "30000"],
                         capture_output=True, timeout=36, creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
    try:
        row = json.loads(run.stdout)
    except (ValueError, UnicodeError):
        raise ValueError("emulator returned no usable status") from None
    if not isinstance(row, dict):
        raise ValueError("malformed status")
    return row


def build(args, out):
    before = dependencies()
    tool = args.tool_root.resolve(strict=True)
    for member in json.loads((ROOT / "scripts/pc286-toolchain-lock.json").read_text())["selected_members"]:
        if sha(tool / member["path"]) != member["sha256"]:
            raise ValueError("C16 tools differ from lock")
    m68k, identity = inspect(args.m68k_bin)
    env = os.environ.copy()
    for name in ("WCC", "WCL", "WLINK", "WATCOM", "INCLUDE", "LIB"):
        env.pop(name, None)
    env["PATH"] = str(m68k["gcc"].parent) + os.pathsep + env["PATH"]
    commands = []
    def compile_tool(args):
        commands.append(list(map(str, args)))
        return subprocess.check_output(commands[-1], env=env, cwd=out, timeout=45).decode().splitlines()

    elf, macbin, rom = [out / ("mac." + suffix) for suffix in ("elf", "bin", "rom")]
    compile_tool([m68k["gcc"], "-mcpu=68000", "-std=c99", "-Os", "-Wall", "-Wextra", "-Werror",
        "-ffreestanding", "-fno-builtin", "-nostdlib", "-Wl,--build-id=none", "-Wl,-T," + str(ROOT/"arch/m68k/mac-rom.ld"),
        *(ROOT / file for file in MAC), "-o", elf])
    if compile_tool([m68k["nm"], "-u", elf]):
        raise ValueError("Mac imports unresolved")
    macmap = "\n".join(compile_tool([m68k["nm"], "-n", elf]))
    diag = original.symbol(r"^([\da-fA-F]+) [BD] cs_mac_diag$", macmap)
    compile_tool([m68k["objcopy"], "-O", "binary", elf, macbin])
    native = macbin.read_bytes()
    if len(native) > 65536:
        raise ValueError("ROM too large")
    rom.write_bytes(native.ljust(65536, b"\xff"))
    objects = []
    for i, src in enumerate(PC):
        obj = out / (str(i) + ".obj")
        compile_tool([tool / "binnt64/wcc.exe", "-zq", "-2", "-ms", "-zl", "-s", "-os",
                      "-zastd=c99", "-we", "-dCS_C16_REQUIRE_16BIT", "-i=" + str(tool/"h"),
                      "-fo=" + str(obj), ROOT / src])
        objects.append(str(obj))
    pcbin, mapping = out / "pc.bin", out / "pc.map"
    compile_tool([tool/"binnt64/wlink.exe", "format", "raw", "bin", "option", "start=start_",
                  "option", "nodefaultlibs", "option", "map=" + str(mapping), "name", pcbin, "file", ",".join(objects)])
    text = mapping.read_text()
    bss = re.search(r"^_BSS\s+BSS\s+DGROUP\s+([\da-f]+)\s+([\da-f]+)", text, re.I | re.M)
    entry = original.symbol(r"Entry point address:\s*([\da-fA-F]+)", text)
    report = original.symbol(r"^\s*([\da-fA-F]+)[+*]?\s+_cs_handoff_summary\s*$", text)
    if not bss or ".lib" in text.lower():
        raise ValueError("unexpected native link")
    bss_base, bss_length = (int(v, 16) for v in bss.groups())
    if not len(pcbin.read_bytes()) <= bss_base <= report <= bss_base + bss_length - 20:
        raise ValueError("native report outside own linked BSS")
    installer = import_module("Build-PC286PreviewMedia").build_install_media(pcbin.read_bytes(), entry,
        bss_base+bss_length, args.assembler, tool/"binnt64/wlink.exe", out/"installer")
    blank, installed = out/"blank.img", out/"installed.img"
    blank.write_bytes(bytes(DISK_BYTES))
    blank_hash = sha(blank)
    keys = []
    for i, key in enumerate((0x43, 0x31, 0x1b, 0x2c, 0x1c, 0x4b, 0x4b, 0x5a)):
        t = 10000000 + i*100000
        for delta, v in ((0, key), (10000, 0xf0), (20000, key)):
            keys.extend(["--scancode", f"{t+delta}:{v}"])
    copy = guest(args.emulator, "pc286", ["--floppy", out/"installer/install-floppy.img",
        "--hard-disk", blank, "--disk-out", installed, "--until-post", "0xac", *keys], 600000000)
    if (copy.get("status") != "condition_met" or copy.get("post") != 0xac or
        sha(blank) != blank_hash or sha(installed) != installer["images"]["hard-disk.img"] or before != dependencies()):
        raise ValueError("install or build inputs changed")
    return dict(source_sha256=before, emulator_sha256=PIN, m68k=identity, build_commands=commands,
                pc_payload_sha256=sha(pcbin), installed_sha256=sha(installed), rom_sha256=sha(rom),
                installer=installer, mac_diag=diag, pc_summary=0x10000+report)


def pc_summary(exe, installed, carrier, address, fault=False, generated=False):
    args = ["--boot-drive", "hard-disk", "--hard-disk", installed, "--floppy", carrier,
            "--until-post", "0xbd", "--inspect", f"{address}:20"]
    if fault:
        args.extend(["--fail-floppy-read", "0:0:2"])
    if generated:
        # Guest address $8400 starts at 286 physical $38400 in this private bank map.
        args.extend(["--inspect", "0x38400:16"])
    row = guest(exe, "pc286", args, 250000000)
    if row.get("status") != "condition_met" or row.get("post") != 0xbd or row.get("cs") != 0x1000:
        raise ValueError("installed 286 did not reach numeric diagnostic stop")
    if fault:
        fault_data = row.get("floppy_read_failure", {})
        if [fault_data.get(k) for k in ("cylinder", "head", "sector", "injected")] != [0, 0, 2, 1]:
            raise ValueError("missing PC second-sector injection")
    if generated:
        obs, adjacent = row["memory_ranges"]
        if adjacent["address"] != 0x38400 or bytes.fromhex(adjacent["bytes"]) != bytes(16):
            raise ValueError("guest bytes after startup blocks changed")
    else:
        obs = row.get("memory", {})
    if obs.get("address") != address or len(obs.get("bytes", "")) != 40:
        raise ValueError("wrong native diagnostic range")
    state, steps, pc, first, failure = struct.unpack("<5I", bytes.fromhex(obs["bytes"]))
    if state not in range(1, 8) or steps > 256 or max(pc, first) > 0xffffff or failure > 6:
        raise ValueError("invalid numeric diagnostic")
    return dict(stop=state, completed_steps=steps, pc=pc, first_successor=first, cpu_status=failure)


def mac_summary(exe, rom, floppy, condition):
    row = guest(exe, "mac128k", ["--rom", rom, "--floppy", floppy, *condition])
    # Avoid returning arbitrary stop/error strings, which can include guest data.
    known_stops = ("condition_met", "machine_fault", "cycle_limit", "timeout", "halted")
    out = dict(condition_met=(row.get("status") == "condition_met"), pc=row.get("pc"), cycles=row.get("cycles"),
               stop=next((kind for kind in known_stops if row.get("status") == kind), "other"))
    if not isinstance(out["pc"], int) or not 0 <= out["pc"] <= 0xffffff:
        raise ValueError("missing native PC")
    if out["stop"] == "machine_fault":
        fault = row.get("fault", {})
        allowed = ("read8", "read16", "read32", "write8", "write16", "write32", "halt")
        address = fault.get("address")
        kind = next((op for op in allowed if op == fault.get("operation")), None)
        if kind is not None and type(address) is int and 0 <= address <= 0xffffff:
            out["fault"] = dict(operation=kind, address=address)
    if "--until-mac-ram-steps" in condition:
        counts = row["mac_ram_steps"]
        for key in ("started", "returned", "first_pc", "last_pc"):
            n = counts[key]
            if not isinstance(n, int) or not 0 <= n <= 0xffffff:
                raise ValueError("invalid RAM step counts")
            out[key] = n
    if "--mac-floppy-bad-checksum" in condition:
        fault = row.get("mac_floppy_checksum_fault", {})
        if (fault.get("kind") != "synthetic_gcr_data_checksum" or fault.get("mode") != "persistent" or
                fault.get("track") != 0 or fault.get("sector") != 1 or not fault.get("altered_nibbles", 0)):
            raise ValueError("missing Mac second-sector injection")
    return out


def trial(exe, built, manifest, scratch, logical, name, count, mac_floppy=None, fault=False):
    carrier = scratch / (name + "-transport.img")
    carrier.write_bytes(logical + bytes(1600*512))
    if mac_floppy is None:
        mac_floppy = scratch / (name + ".raw")
        mac_floppy.write_bytes(logical)
    media = [carrier, mac_floppy, built/"mac.rom", built/"installed.img"]
    before = [sha(p) for p in media]
    if before[2:] != [manifest["rom_sha256"], manifest["installed_sha256"]]:
        raise ValueError("prepared ROM or installed disk changed")
    pc = pc_summary(exe, built/"installed.img", carrier, manifest["pc_summary"], fault, generated=name!="selected")
    if name in ("bad-header", "failed-read"):
        mac = mac_summary(exe, built/"mac.rom", mac_floppy, ["--until", f"{manifest['mac_diag']}:4:0xd0"]
                          + (["--mac-floppy-bad-checksum", "0:1"] if fault else []))
        if not mac["condition_met"] or pc != dict(stop=1, completed_steps=0, pc=0, first_successor=0, cpu_status=0):
            raise ValueError("read or header refusal missed")
        result = dict(name=name, mac_refusal=mac, pc=pc)
    else:
        entry = mac_summary(exe, built/"mac.rom", mac_floppy, ["--until-pc", "0x8002"])
        first = mac_summary(exe, built/"mac.rom", mac_floppy, ["--until-mac-ram-steps", "0x8002:2:1"])
        later = mac_summary(exe, built/"mac.rom", mac_floppy, ["--until-mac-ram-steps", f"0x8000:1024:{count}"])
        result = dict(name=name, mac_entry=entry, mac_first=first, mac_bounded=later, pc=pc)
        if name == "unmapped-write":
            if (later.get("fault") != dict(operation="write8", address=0x800000)
                    or later["started"] != 2 or later["returned"] != 1
                    or pc != dict(stop=3, completed_steps=1, pc=0x81fe, first_successor=0x81fe, cpu_status=2)):
                raise ValueError("authored unmapped write failed to report expected fault")
        if name in ("fixture", "framebuffer", "framebuffer-short", "direct-boundary"):
            expect = 0x7ffe if name in ("fixture", "direct-boundary") else 0x1a700
            terminal = 5 if name == "fixture" else (7 if name == "direct-boundary" else 4)
            if not (entry["condition_met"] and entry["pc"] == 0x8002 and
                first["condition_met"] and first["returned"] == 1 and first["pc"] == 0x81fe and
                later["condition_met"] and later["returned"] == count and later["pc"] == expect and
                pc == dict(stop=terminal, completed_steps=count,
                           pc=expect, first_successor=0x81fe, cpu_status=0)):
                raise ValueError("authored control transfer differed: " + name)
            generated_state = guest(exe, "mac128k", ["--rom", built/"mac.rom", "--floppy", mac_floppy,
                "--until-pc", "0x8002", "--mac-cpu-state", "1", "--inspect", "0x77fc:4",
                "--inspect", "0x8400:16"])
            cpu = generated_state["cpu"]
            slot, adjacent = generated_state["memory_ranges"]
            if (generated_state["status"] != "condition_met" or cpu["sr"] != 0x2700 or cpu["d"] != [0]*8
                    or cpu["a"] != [0x8000]+[0]*6+[0x77fc] or slot["address"] != 0x77fc
                    or bytes.fromhex(slot["bytes"]) != struct.pack(">I", 0x7ffe)
                    or adjacent["address"] != 0x8400 or bytes.fromhex(adjacent["bytes"]) != bytes(16)):
                raise ValueError("generated pre-entry register, stack or neighboring RAM differs")
    if before != [sha(p) for p in media]:
        raise ValueError("read-only input changed")
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("emulator", "output"):
        parser.add_argument("--"+name, type=Path, required=True)
    for name in ("tool-root", "assembler", "m68k-bin", "build", "disk"):
        parser.add_argument("--"+name, type=Path)
    args = parser.parse_args()
    out = args.output.resolve()
    if sha(args.emulator) != PIN:
        raise ValueError("unexpected BootHush build")
    fresh(out)
    if args.disk or args.build:
        if not args.disk or not args.build:
            parser.error("private trial requires both --build and --disk")
        built, disk = args.build.resolve(strict=True), args.disk.resolve(strict=True)
        if ROOT in disk.parents or sha(disk) != SELECTED:
            raise ValueError("private trial requires the identified external System Disk")
        record = json.loads((built/"report.json").read_text())
        if (record.get("status") != "generated-controls-pass" or record["source_sha256"] != dependencies() or
                record["emulator_sha256"] != PIN):
            raise ValueError("prior generated qualification differs")
        logical = import_module("Test-MfsVolumeMedia").dc42_data(disk)
        with TemporaryDirectory(dir=out) as temp:
            rows = [trial(args.emulator, built, record, Path(temp), logical, "selected", 32, disk)
                    for _ in range(2)]
        if rows[0] != rows[1] or sha(disk) != SELECTED:
            raise ValueError("selected input or repeated numeric result changed")
        report = dict(status="bounded-observation", selected_sha256=SELECTED,
                      build_report_sha256=sha(built/"report.json"), trials=rows)
    else:
        if not all([args.tool_root, args.assembler, args.m68k_bin]):
            parser.error("building requires --m68k-bin, --assembler and --tool-root")
        report = build(args, out)
        authored = original.fixture(0x1256abcd)
        screen = bytearray(authored)
        # MOVEA.L ($824).L,A0, JMP (A0). Reads the real seeded screen pointer.
        struct.pack_into(">HIH", screen, 510, 0x2079, 0x824, 0x4ed0)
        short_screen = bytearray(authored)
        struct.pack_into(">HHH", short_screen, 510, 0x2078, 0x824, 0x4ed0)
        direct = bytearray(authored)
        struct.pack_into(">HI", direct, 510, 0x4ef9, 0x7ffe)
        unmapped = bytearray(authored)
        struct.pack_into(">HHI", unmapped, 510, 0x13fc, 0x11, 0x800000)
        bad = bytearray(authored)
        bad[0] = 0
        report["trials"] = [trial(args.emulator, out, report, out, image, name,
                                  2 if name == "direct-boundary" else 3, fault=fault)
                            for name, image, fault in (("fixture", authored, False), ("framebuffer", screen, False),
                                                       ("framebuffer-short", short_screen, False),
                                                       ("direct-boundary", direct, False),
                                                       ("unmapped-write", unmapped, False),
                                                       ("bad-header", bad, False), ("failed-read", authored, True))]
        report["status"] = "generated-controls-pass"
    if sha(args.emulator) != PIN:
        raise ValueError("emulator changed during run")
    (out/"report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(report["status"])


if __name__ == "__main__":
    main()
