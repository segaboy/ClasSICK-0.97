#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Cold boot original register/CCR fixtures on the normal disk-startup ROM."""
import argparse
from importlib import import_module
import json
from pathlib import Path
import struct
import subprocess

from m68k_windows_toolchain import inspect, sha

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for key in ("product", "m68k-bin", "emulator", "output"):
        parser.add_argument("--" + key, type=Path, required=True)
    args = parser.parse_args()
    out = args.output.resolve()
    if out == ROOT or ROOT in out.parents:
        raise ValueError("use a fresh external output directory")
    out.mkdir(parents=True, exist_ok=False)
    product = args.product.resolve(strict=True)
    manifest = json.loads((product / "manifest.json").read_text())
    rom = product / "disk-startup-mac128k.rom"
    if sha(rom) != manifest["artifacts"][rom.name] or rom.stat().st_size != 65536:
        raise ValueError("product check failed")
    builder = import_module("Build-DiskStartup")
    for path in [*builder.MAC, builder.LINKER_SCRIPT]:
        if sha(ROOT / path) != manifest["sources"][path]:
            raise ValueError("Mac build source changed: " + path)
    tool, _ = inspect(args.m68k_bin.resolve(strict=True))
    fixture = ROOT / "tests/mac-startup-traps.S"
    obj, program = out / "caller.o", out / "caller.bin"
    subprocess.run([tool["gcc"], "-m68000", "-c", fixture, "-o", obj], check=True)
    subprocess.run([tool["objcopy"], "-j", ".text", "-O", "binary", obj, program], check=True)
    body = program.read_bytes()
    if not 0 < len(body) <= 512:
        raise ValueError("fixture crosses startup area")
    before_trap = bytes.fromhex("13fc00550000913d")
    after_trap = bytes.fromhex("13fc00bc0000913c60fe")
    cases = {"registers": body, "unsupported": before_trap + bytes.fromhex("a800") + after_trap,
             "odd-set": before_trap + bytes.fromhex("7017207c00008301a047") + after_trap}
    observations = []
    for name, code in cases.items():
        image = bytearray(409600)
        struct.pack_into(">HHh", image, 0, 0x4c4b, 0x6000, 512-4)
        image[512:512+len(code)] = code
        disk = out / (name + ".img")
        disk.write_bytes(image)
        before = sha(disk)
        machine = [args.emulator, "--machine", "mac128k", "--rom", rom, "--floppy", disk,
                   "--cycles", "16000000", "--timeout-ms", "25000", "--inspect", "0x9100:68"]
        if name == "registers":
            machine += ["--until", "0x913c:1:0xbc"]
        else:
            machine += ["--until-pc", str(manifest["mac"]["startup_stop_pc"])]
        result = subprocess.run(machine, capture_output=True, timeout=30, check=True)
        data = json.loads(result.stdout)
        if data["status"] != "condition_met" or sha(disk) != before:
            raise ValueError(name + ": guest missed required stop")
        seen = bytes.fromhex(data["memory"]["bytes"])
        expected = bytearray(68)
        if name == "registers":
            get = manifest["mac"]["get_trap_address_pc"]
            set_address = manifest["mac"]["set_trap_address_pc"]
            struct.pack_into(">IHIHIHHIHIHIIIIIIIB", expected, 0,
                0x7800,0x2710,get,0x271f,set_address,0x271b,0x2714,0x12345678,
                0x2718,0x12345678,0x2718,0xabcdef,0xffffffff,17,18,19,0x11112222,0x7800,0xbc)
            struct.pack_into(">I", expected, 64, set_address)
        else:
            expected[61] = 0x55
        if seen != expected:
            raise ValueError(name + ": authored register results differ: " + seen.hex())
        observations.append(dict(case=name, cycles=data["cycles"], bytes=seen.hex(), disk_sha256=before))
    report = dict(status="passed", manifest_sha256=sha(product / "manifest.json"), rom_sha256=sha(rom),
                  script_sha256=sha(Path(__file__)), fixture_sha256=sha(fixture), emulator_sha256=sha(args.emulator),
                  program_sha256=sha(program), cases=observations)
    (out / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print("Native Macintosh Get/Set and patched trap checks passed.")


if __name__ == "__main__":
    main()
