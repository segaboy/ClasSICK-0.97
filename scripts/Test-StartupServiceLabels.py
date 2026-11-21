#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Qualify the Mac service-label diagnostic or observe one admitted disk read-only."""
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

product = import_module("Build-DiskStartup")
ROOT = product.ROOT
ASSEMBLY = "probes/mac128k/startup-service-label.S"
CATALOG = "probes/mac128k/service-labels.def"
ROM = "startup-service-labels.rom"
EMU_SHA = "a00fb8b507d79b0f0b88026c2e9cd613a6a7bd78c3aae10858bb5735e4f6db98"
DISK_SHA = "1e03b75a05bbf441ae4e1808f70e38ffe0a07e3cc3298de151c0d2775552c647"


def inputs():
    pending = list(product.MAC) + [ASSEMBLY]
    files = {product.LINKER_SCRIPT, "scripts/Build-DiskStartup.py", "scripts/Build-PC286PreviewMedia.py",
             "scripts/m68k_windows_toolchain.py", "scripts/pc286_loader.py", "scripts/pc286_hard_disk_loader.py",
             Path(__file__).relative_to(ROOT).as_posix()}
    while pending:
        name = pending.pop()
        if name in files:
            continue
        files.add(name)
        path = ROOT / name
        for child in re.findall(r'^\s*#include\s+"([^"]+)"', path.read_text(), re.M):
            pending.append((path.parent/child).resolve(strict=True).relative_to(ROOT).as_posix())
    return {p: sha(ROOT/p) for p in sorted(files)}


def catalog():
    entries = re.findall(r"^CS_SERVICE\((0x[0-9a-f]+), ([A-Za-z][A-Za-z0-9]+)\)$",
                         (ROOT/CATALOG).read_text(), re.M)
    keys, names = [int(key, 16) for key, _ in entries], [name for _, name in entries]
    if not keys or keys != sorted(set(keys)) or len(names) != len(set(names)) or any(
            not (0 <= key <= 0xff or 0x800 <= key <= 0x9ff) for key in keys):
        raise ValueError("invalid diagnostic service catalog")
    return dict(zip(keys, names))


def build(bin_dir, out, unhandled_only=False):
    native, identity = inspect(bin_dir)
    if {k: v["sha256"] for k, v in identity["tools"].items()} != product.M68K_SHA:
        raise ValueError("compiler identity differs")
    environment = os.environ.copy()
    for name in ("GCC_EXEC_PREFIX", "COMPILER_PATH", "LIBRARY_PATH", "CPATH", "C_INCLUDE_PATH"):
        environment.pop(name, None)
    environment["PATH"] = str(bin_dir) + os.pathsep + environment["PATH"]
    out.mkdir()
    with TemporaryDirectory(dir=out) as temp:
        work = Path(temp)
        def run(*args):
            return subprocess.check_output(list(map(str, args)), cwd=work, env=environment, timeout=60,
                text=True, creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        elf, raw = work/"labels.elf", work/"labels.bin"
        wrap = (["-DCS_DIAG_UNHANDLED", "-Wl,--wrap=cs_mac_trap_target"] if unhandled_only else
                ["-Wl,--wrap=cs_mac_load_startup_disk"])
        run(native["gcc"], "-mcpu=68000", "-std=c99", "-Os", "-Wall", "-Wextra", "-Werror",
            "-ffreestanding", "-fno-builtin", "-nostdlib", "-Wl,--build-id=none",
            *wrap, "-Wl,-T,"+str(ROOT/product.LINKER_SCRIPT),
            *(ROOT/s for s in product.MAC+[ASSEMBLY]), "-o", elf)
        if run(native["nm"], "-u", elf).strip():
            raise ValueError("unresolved diagnostic imports")
        symbols = {s: int(v, 16) for v, s in re.findall(r"^([0-9a-f]+)\s+\w\s+(\S+)$",
                                                          run(native["nm"], "-n", elf), re.M)}
        run(native["objcopy"], "-O", "binary", elf, raw)
        image = raw.read_bytes()
        names = [*catalog().values(), "unmatched"]
        terminals = {name: symbols["cs_diag_"+name] for name in names}
        gateway = symbols["cs_diag_gateway"]
        if (len(set([*terminals.values(), gateway])) != len(names)+1 or
                len(image) != symbols["__rom_end"]-0x400000 or len(image) > 65536 or
                any(pc & 1 or not 0x400104 <= pc < symbols["__rom_code_end"]
                    for pc in [gateway, *terminals.values()]) or
                any(image[pc-0x400000:pc-0x400000+2] != b"\x60\xfe" for pc in terminals.values()) or
                struct.unpack_from(">I", image, 40)[0] != symbols["cs_mac_startup_line_a"]):
            raise ValueError("diagnostic layout or terminal identities differ")
        (out/ROM).write_bytes(image.ljust(65536, b"\xff"))
        layout = {"gateway": gateway, "terminals": terminals, "rom_sha256": sha(out/ROM), "used_bytes": len(image)}
        if unhandled_only:
            layout["wrapped_lookup"] = symbols["__wrap_cs_mac_trap_target"]
        return layout


def bounded(exe, rom, disk, condition=(), allow_guest_stop=False):
    run = subprocess.run([exe, "--machine", "mac128k", "--rom", rom, "--floppy", disk,
                          "--cycles", "12000000", "--timeout-ms", "15000", *condition],
                         capture_output=True, timeout=22, creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
    try:
        row = json.loads(run.stdout)
    except (ValueError, UnicodeError):
        raise ValueError("unrecognized emulator response") from None
    accepted = {("cycle_limit", 0), ("cycle_limit", 1), ("condition_met", 0)}
    if allow_guest_stop:
        accepted.update({("machine_fault", 1), ("halted", 0), ("halted", 1)})
    if not isinstance(row, dict) or type(row.get("pc")) != int or (row.get("status"), run.returncode) not in accepted:
        raise ValueError("diagnostic failed before qualification")
    return row["status"], row["pc"]


def classify(exe, rom, disk, layout, unhandled_only=False):
    gateway = layout["gateway"]
    status, pc = bounded(exe, rom, disk, ["--until-pc", str(gateway)], unhandled_only)
    if status != "condition_met" or pc != gateway:
        return "no-unhandled-service-observed" if unhandled_only else "gateway-not-entered"
    status, pc = bounded(exe, rom, disk)
    if status != "cycle_limit":
        raise ValueError("unexpected terminal run status")
    for name, address in layout["terminals"].items():
        if pc == address:
            return name
    raise ValueError("no owned service terminal after gateway entry")


def fixture(code):
    raw = bytearray(409600)
    struct.pack_into(">HHH", raw, 0, 0x4c4b, 0x6000, 506)  # Cross-sector entry.
    raw[510:510+len(code)] = code
    return raw


def controls(layout):
    # Classify every name through the actual vector. Independent literal
    # spot checks also exercise both flag formats and conflicting adjacent data.
    for key, name in catalog().items():
        yield name, struct.pack(">HH", 0xa000 | key, 0x60fe), name
    yield "flagged-os", bytes.fromhex("a746 a9f0"), "GetTrapAddress"
    yield "autopop-toolbox", bytes.fromhex("adf0 a146"), "LoadSeg"
    yield "unlisted-os", bytes.fromhex("a0fc a146"), "unmatched"
    yield "unlisted-toolbox", bytes.fromhex("a9fc a9f0"), "unmatched"
    yield "reserved-toolbox", bytes.fromhex("abf0 60fe"), "unmatched"
    # Seed D0 and the words adjacent to the actual exception with conflicting
    # recognizable values. The only evidence comes from the stacked call PC.
    yield "stale", bytes.fromhex("303c a995 a002 a146"), "PBRead"
    yield "no-trap", bytes.fromhex("303c a995 60fe"), "gateway-not-entered"
    yield "direct-label", struct.pack(">HI", 0x4ef9, layout["terminals"]["InitUtil"]), "gateway-not-entered"


def unhandled_controls(layout):
    no_call = "no-unhandled-service-observed"
    # D0=8 and A0=0. Require a returned nonnull pointer, then
    # write/read it before reaching PBRead. Failure loops short of the call.
    newptr = bytes.fromhex("7008 207c00000000 a11e 2208 670c 10bc005a 0c10005a 6602")
    yield "newptr-then-read", newptr + bytes.fromhex("a002 60fe"), "PBRead"
    yield "newptr-then-unlisted", newptr + bytes.fromhex("a0fc 60fe"), "unmatched"
    yield "newptr-alone", newptr + bytes.fromhex("60fe 60fe"), no_call
    # Patch the PBRead slot to a guest RTS handler at $8300. Reach PBWrite
    # only after Set, Get and that handler return with its original value.
    patched = bytearray.fromhex("7002 207c00008300 a047 a146 a002 0c401234 6602 a003 60fe")
    patched.extend(bytes(0x300 - 510 - len(patched)))
    patched.extend(bytes.fromhex("303c1234 4e75"))
    yield "patch-then-write", patched, "PBWrite"
    yield "unknown-os", bytes.fromhex("a0fc 60fe"), "unmatched"
    yield "unknown-toolbox", bytes.fromhex("a9fc 60fe"), "unmatched"
    yield "toolbox", bytes.fromhex("a995 60fe"), "InitResources"
    yield "stale", bytes.fromhex("303ca995 a002 a146"), "PBRead"
    yield "no-trap", bytes.fromhex("303ca995 60fe"), no_call
    yield "fault-before-lookup", bytes.fromhex("13fc001100800000 60fe"), no_call
    # A successful Get followed by a direct jump to a label never enters
    # the refusal gateway. Nor does a guest subroutine call to the resolver.
    direct_label = struct.pack(">HI", 0x4ef9, layout["terminals"]["InitUtil"])
    yield "direct-label", bytes.fromhex("7046 a146") + direct_label, no_call
    direct_lookup = struct.pack(">HIHH", 0x4eb9, layout["wrapped_lookup"], 0x588f, 0x60fe)
    yield "direct-lookup", bytes.fromhex("2f3c0000a002") + direct_lookup, no_call


def write_json(path, value):
    path.write_bytes((json.dumps(value, indent=2, sort_keys=True)+"\n").encode())


def run(args):
    exe = args.emulator.resolve(strict=True)
    out = args.output.resolve()
    if sha(exe) != EMU_SHA or out == ROOT or ROOT in out.parents or out.exists():
        raise ValueError("expected the qualified emulator and a new external output directory")
    before = inputs()
    unhandled = args.unhandled_only
    mode = "unhandled-only" if unhandled else "first-service"
    fixture_cases = unhandled_controls if unhandled else controls
    if args.mode == "qualify":
        out.mkdir(parents=True)
        layout = build(args.m68k_bin.resolve(strict=True), out/"a", unhandled)
        if build(args.m68k_bin.resolve(strict=True), out/"b", unhandled) != layout:
            raise ValueError("diagnostic twins differ")
        checks = {}
        with TemporaryDirectory(dir=out) as tmp:
            disk = Path(tmp)/"authored.raw"
            for case, code, expected in fixture_cases(layout):
                disk.write_bytes(fixture(code))
                observed = classify(exe, out/"a"/ROM, disk, layout, unhandled)
                if observed != expected:
                    raise ValueError(f"authored control {case} observed {observed}, expected {expected}")
                checks[case] = {"handler": observed, "fixture_sha256": sha(disk)}
        report = {"status":"generated-controls-pass", "observation_mode":mode, "inputs":before, "layout":layout, "checks":checks,
                  "emulator_sha256":EMU_SHA, "m68k_tool_sha256":product.M68K_SHA}
    else:
        package = args.qualification.resolve(strict=True)
        receipt = package/"report.json"
        if sha(receipt) != args.qualification_sha256:
            raise ValueError("generated qualification identity differs")
        generated = json.loads(receipt.read_bytes())
        if generated.get("observation_mode") != mode:
            raise ValueError("generated qualification uses a different observation mode")
        layout = generated["layout"]
        rom = package/"a"/ROM
        disk = args.disk.resolve(strict=True)
        if (generated["status"] != "generated-controls-pass" or generated["inputs"] != before or
                generated["emulator_sha256"] != EMU_SHA or generated["m68k_tool_sha256"] != product.M68K_SHA or
                set(layout["terminals"]) != {*catalog().values(), "unmatched"} or
                set(generated["checks"]) != {case for case, _, _ in fixture_cases(layout)} or
                any(generated["checks"][case]["handler"] != expected for case, _, expected in fixture_cases(layout)) or
                not sha(rom) == sha(package/"b"/ROM) == layout["rom_sha256"] or
                ROOT in disk.parents or sha(disk) != DISK_SHA):
            raise ValueError("qualified sources, fixtures, ROM or selected media differ")
        out.mkdir(parents=True)
        labels = [classify(exe, rom, disk, layout, unhandled) for _ in range(3)]
        if len(set(labels)) != 1 or sha(disk) != DISK_SHA:
            raise ValueError("cold observations or selected disk identity differ")
        report = {"status":"bounded-observation", "observation_mode":mode, "selected_sha256":DISK_SHA, "rom_sha256":layout["rom_sha256"],
                  "qualification_sha256":args.qualification_sha256, "emulator_sha256":EMU_SHA, "handlers":labels}
    if inputs() != before or sha(exe) != EMU_SHA:
        raise ValueError("sources or emulator changed during the trial")
    write_json(out/"report.json", report)
    print(report["status"], report.get("handlers", []))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    actions = parser.add_subparsers(dest="mode", required=True)
    q = actions.add_parser("qualify")
    q.add_argument("--m68k-bin", required=True, type=Path)
    t = actions.add_parser("selected")
    for arg in ("disk", "qualification"):
        t.add_argument("--"+arg, required=True, type=Path)
    t.add_argument("--qualification-sha256", required=True)
    for sub in (q, t):
        sub.add_argument("--unhandled-only", action="store_true",
                         help="observe only service lookups declined by the native Line-A dispatcher")
        sub.add_argument("--emulator", required=True, type=Path)
        sub.add_argument("--output", required=True, type=Path)
    run(parser.parse_args())
