#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Build the Macintosh disk-startup developer ROM and native 286 installer."""
import argparse
from importlib import import_module
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
from tempfile import TemporaryDirectory

from m68k_windows_toolchain import inspect, sha

media = import_module("Build-PC286PreviewMedia")
ROOT = media.ROOT
ROM = "disk-startup-mac128k.rom"
INSTALLER = "disk-startup-286-installer.img"
SHARED = ["core/storage/block-read.c", "core/storage/mac-boot-header.c",
          "personality/system1/startup-globals.c", "personality/system1/startup-heap.c"]
MAC = [*SHARED, "core/services/traps.c", "arch/m68k/disk-startup-reset.S", "platform/mac128k/iwm-read.c",
       "platform/mac128k/iwm-capture.S", "platform/mac128k/startup-floppy.c", "platform/mac128k/disk-startup.c"]
PC = [*SHARED, "core/cpu/m68k.c", "core/cpu/m68k-bus.c", "core/memory/guest.c",
      "core/services/traps.c", "platform/pc286/banked-guest.c", "platform/pc286/m68k-banked.c",
      "platform/pc286/mac128k-ram.c", "platform/pc286/mac128k-transfer.c",
      "personality/system1/banked-services.c", "platform/pc286/boothush-floppy-read.c",
      "platform/pc286/disk-startup.c"]
LINKER_SCRIPT = "arch/m68k/mac-rom.ld"
M68K_SHA = {
    "gcc": "35e82461e0cfd0e4d89be95920546155c28aabbd88227dec941c5027d1dfbe30",
    "nm": "114ba98342ca2023cf00edb3c79d5f80ccf99f4978ff2e28086d1d6fb5e0929a",
    "objcopy": "631d6dcdd0a58fde628c56ded48aa8f05df0bbca0ce1ccd74156b75d103706fe",
    "objdump": "cd558fa890b6624a7478abdcbbbc882d6db193a34717123c6883215f0f02d390"}


def source_hashes():
    sources = set(PC + MAC + [LINKER_SCRIPT, "platform/pc286/preview-boot.asm",
        "platform/pc286/preview-install.asm", "scripts/Build-DiskStartup.py",
        "scripts/m68k_windows_toolchain.py", "scripts/Build-PC286PreviewMedia.py",
        "scripts/pc286_loader.py", "scripts/pc286_hard_disk_loader.py", "scripts/pc286-toolchain-lock.json"])
    pending = list(MAC + PC)
    while pending:
        path = ROOT / pending.pop()
        for include in re.findall(r'^\s*#include\s+"([^"]+)"', path.read_text(), re.M):
            rel = (path.parent / include).resolve(strict=True).relative_to(ROOT).as_posix()
            if rel not in sources:
                sources.add(rel)
                pending.append(rel)
    return {path: sha(ROOT/path) for path in sorted(sources)}


def build(m68k_bin, tool, assembler, output):
    output = output.resolve()
    if output == ROOT or ROOT in output.parents or output.exists():
        raise ValueError("choose a new output directory outside the repository")
    output.parent.mkdir(parents=True, exist_ok=True)
    inputs = source_hashes()
    native, identity = inspect(m68k_bin)
    if {k: v["sha256"] for k, v in identity["tools"].items()} != M68K_SHA:
        raise ValueError("m68k tool identity differs")
    lock = json.loads((ROOT/"scripts/pc286-toolchain-lock.json").read_text())
    for member in lock["selected_members"]:
        if sha(tool/member["path"]) != member["sha256"]:
            raise ValueError("286 tool or header identity differs: " + member["path"])
    env = os.environ.copy()
    for name in ("WCC", "WCL", "WLINK", "WATCOM", "INCLUDE", "LIB", "GCC_EXEC_PREFIX",
                 "COMPILER_PATH", "LIBRARY_PATH", "CPATH", "C_INCLUDE_PATH"):
        env.pop(name, None)
    env["PATH"] = str(native["gcc"].parent) + os.pathsep + env["PATH"]
    with TemporaryDirectory(dir=output.parent) as tmp:
        work = Path(tmp)
        result = work/"package"
        result.mkdir()
        def run(*args):
            return subprocess.check_output(list(map(str, args)), cwd=work, env=env, timeout=60,
                text=True, creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))

        elf, raw = work/"mac.elf", work/"mac.bin"
        run(native["gcc"], "-mcpu=68000", "-std=c99", "-Os", "-Wall", "-Wextra", "-Werror",
            "-ffreestanding", "-fno-builtin", "-fstack-usage", "-nostdlib", "-Wl,--build-id=none",
            "-Wl,-T,"+str(ROOT/LINKER_SCRIPT), *(ROOT/s for s in MAC), "-o", elf)
        if run(native["nm"], "-u", elf).strip():
            raise ValueError("unresolved Mac import")
        symbols = {s: int(v, 16) for v, s in re.findall(r"^([0-9a-fA-F]+)\s+\w\s+(\S+)$",
                                                          run(native["nm"], "-n", elf), re.M)}
        run(native["objcopy"], "-O", "binary", elf, raw)
        frames = {}
        for path in work.glob("*.su"):
            for line in path.read_text().splitlines():
                label, count, kind = line.split("\t")
                if kind not in ("static", "dynamic,bounded"):
                    raise ValueError("unbounded Mac C stack frame: " + label)
                frames[label.rsplit(":", 1)[-1]] = int(count)
        if not frames or sum(frames.values()) >= 0x4000-0x2000:
            raise ValueError("missing stack report or C frames exceed startup stack reservation")
        image = raw.read_bytes()
        vectors = struct.unpack_from(">64I", image)
        service_pcs = {field: symbols[label] for field, label in (
            ("get_trap_address_pc", "cs_mac_get_trap_address"),
            ("set_trap_address_pc", "cs_mac_set_trap_address"),
            ("new_ptr_pc", "cs_mac_new_ptr"),
            ("new_ptr_dispatch_pc", "cs_mac_new_ptr_dispatch"),
            ("new_ptr_success_pc", "cs_mac_new_ptr_success"),
            ("new_ptr_failure_pc", "cs_mac_new_ptr_failure"),
            ("service_return_pc", "cs_mac_startup_service_return"),
            ("line_a_pc", "cs_mac_startup_line_a"),
            ("startup_stop_pc", "cs_mac_startup_stop"))}
        if (len(image) != symbols["__rom_end"]-0x400000 or len(image) > 65536 or
            vectors[:2] != (0x4000, symbols["cs_mac_reset"]) or
            any(addr != symbols["cs_mac_startup_line_a" if i == 10 else "cs_mac_startup_stop"]
                for i, addr in enumerate(vectors[2:], 2)) or
            len(set(service_pcs.values())) != len(service_pcs) or
            any(addr & 1 or not 0x400104 <= addr < symbols["__rom_code_end"]
                for addr in (*vectors[1:], *service_pcs.values())) or
            not 0xb00 <= symbols["__data_start"] <= symbols["__bss_end"] <= 0x2000 <= vectors[0] <= 0x7800):
            raise ValueError("Mac ROM vectors or linked layout differ")
        (result/ROM).write_bytes(image.ljust(65536, b"\xff"))
        objects = []
        for index, src in enumerate(PC):
            obj = work/f"pc{index}.obj"
            run(tool/"binnt64/wcc.exe", "-zq", "-2", "-ms", "-zl", "-s", "-os", "-zastd=c99",
                "-we", "-dCS_C16_REQUIRE_16BIT", "-i="+str(tool/"h"), "-fo="+str(obj), ROOT/src)
            objects.append(str(obj))
        linker, binary, mapping = tool/"binnt64/wlink.exe", work/"pc.bin", work/"pc.map"
        run(linker, "format", "raw", "bin", "option", "start=start_", "option", "nodefaultlibs",
            "option", "map="+str(mapping), "name", binary, "file", ",".join(objects))
        text = mapping.read_text()
        entry = re.search(r"Entry point address:\s*([0-9a-fA-F]+)", text)
        native_pcs = {}
        for field, name in (("startup_step_pc", "cs_s1_banked_startup_step_"),
                            ("new_ptr_pc", "cs_pc286_startup_new_ptr_")):
            match = re.search(r"^\s*([0-9a-fA-F]+)[+*]?\s+"+name+r"\s*$", text, re.M)
            if not match or int(match[1], 16) >= binary.stat().st_size:
                raise ValueError("missing native PC inside payload: " + name)
            native_pcs[field] = 0x10000 + int(match[1], 16)
        bss = re.search(r"^_BSS\s+BSS\s+DGROUP\s+([\da-fA-F]+)\s+([\da-fA-F]+)", text, re.M)
        if not entry or not bss or ".lib" in text.lower():
            raise ValueError("unexpected PC link map")
        base, size = [int(v, 16) for v in bss.groups()]
        if base < binary.stat().st_size:
            raise ValueError("PC BSS overlaps payload")
        package = media.build_install_media(binary.read_bytes(), int(entry[1], 16), base + size,
                                            assembler, linker, work/"installer")
        shutil.copyfile(work/"installer/install-floppy.img", result/INSTALLER)
        if source_hashes() != inputs:
            raise ValueError("source inputs changed during build")
        manifest = dict(profile="disk-startup-developer", specification="SPEC-0141",
            artifacts={n: sha(result/n) for n in (ROM, INSTALLER)}, sources=inputs,
            mac_sources=MAC, pc_sources=PC, m68k_tools=M68K_SHA,
            pc_assembler=package["assembler_sha256"], pc_linker=package["linker_sha256"],
            mac=dict(rom_used=len(image), reset_sp=vectors[0], reset_pc=vectors[1],
                     exception_vectors=list(vectors[2:]), ram_data_start=symbols["__data_start"],
                     ram_bss_end=symbols["__bss_end"], entry_stack_top=0x7800,
                     **service_pcs, c_stack_frames=dict(sorted(frames.items())),
                     iwm_wire_bytes=706, iwm_decoded_bytes=524),
            pc=dict(payload_sha256=sha(binary), installed_disk_sha256=package["images"]["hard-disk.img"],
                    entry=int(entry[1], 16), start_pc=0x10000+int(entry[1], 16),
                    **native_pcs, payload_bytes=binary.stat().st_size, bss_start=base, bss_bytes=size,
                    boot_stack_top=0xfffe, guest_stack_top=0x7800),
            limits="Experimental startup only. Complete ROM services, application workload and physical hardware remain open.")
        (result/"manifest.json").write_text(json.dumps(manifest, indent=2)+"\n", encoding="utf-8")
        result.rename(output)
        return manifest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("m68k-bin", "tool-root", "assembler", "output"):
        parser.add_argument("--"+name, required=True, type=Path)
    args = parser.parse_args()
    result = build(args.m68k_bin.resolve(), args.tool_root.resolve(), args.assembler.resolve(), args.output)
    print(json.dumps(result["artifacts"]))


if __name__ == "__main__":
    main()
