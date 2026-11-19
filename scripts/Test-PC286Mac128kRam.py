#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Qualify Macintosh low RAM aliases on the native 286 using disposable test media."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess

from pc286_loader import floppy

ROOT = Path(__file__).resolve().parents[1]
UNITS = ["platform/pc286/banked-guest.c", "platform/pc286/m68k-banked.c",
         "platform/pc286/mac128k-ram.c", "probes/pc286/boot-mac128k-ram.c"]
PIN = "3d48d7515d3492ac796d3899ed2c47603d3330e3f0f64352fd29ed6a78284ff4"


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for key in ("tool-root", "emulator", "output"):
        parser.add_argument("--" + key, type=Path, required=True)
    parser.add_argument("--transfer", action="store_true", help="also verify the installed runtime's block writes")
    args = parser.parse_args()
    tool = args.tool_root.resolve(strict=True)
    exe = args.emulator.resolve(strict=True)
    out = args.output.resolve()
    if digest(exe) != PIN or out == ROOT or ROOT in out.parents:
        raise ValueError("use the qualified BootHush binary and a fresh external output")
    out.mkdir(parents=True, exist_ok=False)
    lock = ROOT / "scripts/pc286-toolchain-lock.json"
    for member in json.loads(lock.read_text())["selected_members"]:
        if digest(tool / member["path"]) != member["sha256"]:
            raise ValueError("C16 toolchain identity changed")
    units = [*UNITS, "platform/pc286/mac128k-transfer.c"] if args.transfer else UNITS
    sources = set(units)
    queue = list(units)
    while queue:
        path = ROOT / queue.pop()
        for name in re.findall(r'^\s*#include\s+"([^"]+)"', path.read_text(), re.M):
            relative = (path.parent / name).resolve().relative_to(ROOT).as_posix()
            if relative not in sources:
                sources.add(relative)
                queue.append(relative)
    sources.update(["scripts/pc286_loader.py", "scripts/pc286-toolchain-lock.json",
                    "scripts/Test-PC286Mac128kRam.py", "docs/specifications/SPEC-0139-mac128k-ram-aperture.md"])
    before = {key: digest(ROOT / key) for key in sorted(sources)}
    env = os.environ.copy()
    for key in ("WCC", "WCL", "WLINK", "WATCOM", "LIB", "INCLUDE"):
        env.pop(key, None)
    commands = []

    def run(command):
        commands.append([str(arg) for arg in command])
        return subprocess.run(command, cwd=out, env=env, capture_output=True, check=True,
                              timeout=40, creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0)).stdout

    objects = []
    flags = ["-dCS_QUALIFY_RAM_BLOCKS"] if args.transfer else []
    for unit in units:
        obj = out / (Path(unit).stem + ".obj")
        run([tool / "binnt64/wcc.exe", "-zq", "-2", "-ms", "-zl", "-s", "-os", "-we", "-zastd=c99",
             "-dCS_C16_REQUIRE_16BIT", *flags, "-i=" + str(tool / "h"), "-fo=" + str(obj), ROOT / unit])
        metadata = run([tool / "binnt64/dmpobj.exe", obj])
        if b"80286" not in metadata or b"USE16" not in metadata:
            raise ValueError("unexpected native object")
        objects.append(obj)
    binary = out / "ram.bin"
    linkmap = out / "ram.map"
    run([tool / "binnt64/wlink.exe", "format", "raw", "bin", "option", "start=start_", "option", "nodefaultlibs",
         "option", "map=" + str(linkmap), "name", binary, "file", ",".join(map(str, objects))])
    linked = linkmap.read_text()
    if ".lib" in linked.lower():
        raise ValueError("unexpected runtime import")
    entry = int(re.search(r"Entry point address:\s*([\da-fA-F]+)", linked).group(1), 16)
    image = out / "ram.img"
    payload = binary.read_bytes()
    image.write_bytes(floppy(payload, entry, verify_payload=True))
    media_hash = digest(image)
    checks = {0x10000 + start: payload[start:start + 4096] for start in range(0, len(payload), 4096)}
    checks.update({0x30000: bytes.fromhex("f123"), 0x37ffe: bytes.fromhex("1234"),
                   0x3a000: bytes.fromhex("001f"), 0x55ff0: bytes.fromhex("decafbad"), 0x55ffe: bytes.fromhex("abcd")})
    cmd = [exe, "--machine", "pc286", "--floppy", image, "--cycles", "30000000", "--timeout-ms", "25000",
           "--until-post", "0xb8"]
    for addr, data in checks.items():
        cmd += ["--inspect", f"{addr:#x}:{len(data)}"]
    guest = json.loads(run(cmd))
    rows = guest.pop("memory_ranges")
    observed = {row["address"]: bytes.fromhex(row["bytes"]) for row in rows}
    if guest["status"] != "condition_met" or guest["post"] != 0xb8 or guest["cs"] != 0x1000 or observed != checks:
        raise ValueError("guest stop or inspected bytes differed: " + str((guest["status"], guest["post"])))
    if digest(image) != media_hash or before != {key: digest(ROOT / key) for key in before}:
        raise ValueError("inputs changed")
    report = {"status": "pass", "guest": guest, "source_sha256": before, "payload_sha256": digest(binary),
              "payload_bytes": len(payload), "emulator_sha256": PIN, "media_sha256": media_hash,
              "checks": {hex(addr): data.hex() for addr, data in observed.items() if addr >= 0x30000},
              "loaded_payload_matches": True, "transfer_profile": args.transfer, "commands": commands}
    (out / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print("128K RAM mirrors passed:", out / "report.json")


if __name__ == "__main__":
    main()
