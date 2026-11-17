#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Qualify original native 286 block reads through modeled INT13 floppy media."""
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
PROVIDER = ROOT / "platform/pc286/boothush-floppy-read.c"
PROBE = ROOT / "probes/pc286/boot-floppy-read.c"
CORE = ROOT / "core/storage/block-read.c"
SPEC = ROOT / "docs/specifications/SPEC-0113-boothush-pc286-floppy-provider.md"


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tool-root", type=Path, required=True)
    parser.add_argument("--emulator", type=Path, required=True)
    parser.add_argument("--model-contract", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--mfs-integration", action="store_true",
                        help="also read original MFS metadata through the real floppy provider")
    args = parser.parse_args()
    tool, emulator, contract = (p.resolve(strict=True) for p in
                                (args.tool_root, args.emulator, args.model_contract))
    out = args.output.resolve()
    if out == ROOT or ROOT in out.parents:
        raise ValueError("output must be a new external directory")
    lock_path = ROOT / "scripts/pc286-toolchain-lock.json"
    lock = json.loads(lock_path.read_text(encoding="utf-8"))
    for item in lock["selected_members"]:
        path = tool / item["path"]
        if path.stat().st_size != item["bytes"] or sha(path) != item["sha256"]:
            raise ValueError("compiler identity mismatch: " + item["path"])
    sources = [PROVIDER, PROVIDER.with_suffix(".h"), PROBE, CORE, CORE.with_suffix(".h"),
               ROOT / "arch/i286/c16.h", ROOT / "scripts/pc286_loader.py", SPEC, Path(__file__).resolve()]
    mfs_source, mfs_probe = ROOT / "core/storage/mfs-read.c", ROOT / "probes/pc286/boot-floppy-mfs-read.c"
    if args.mfs_integration:
        sources += [mfs_source, mfs_source.with_suffix(".h"), mfs_probe,
                    ROOT / "docs/specifications/SPEC-0112-mfs-metadata-reader.md"]
    before_sources = {p.relative_to(ROOT).as_posix(): sha(p) for p in sources}
    emulator_sha, contract_sha = sha(emulator), sha(contract)
    loader_spec = importlib.util.spec_from_file_location("floppy_loader", ROOT / "scripts/pc286_loader.py")
    loader = importlib.util.module_from_spec(loader_spec)
    loader_spec.loader.exec_module(loader)
    out.mkdir(parents=True, exist_ok=False)
    env = os.environ.copy()
    for key in ("WCC", "WATCOM", "INCLUDE", "LIB", "WCL", "WLINK"):
        env.pop(key, None)
    commands = []

    def run(command, label):
        command = [str(x) for x in command]
        commands.append(command)
        result = subprocess.run(command, cwd=out, env=env, capture_output=True, timeout=35,
                                creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        (out / (label + ".log")).write_bytes(result.stdout + b"\n" + result.stderr)
        if result.returncode:
            raise ValueError(label + " failed; see " + str(out / (label + ".log")))
        return result.stdout

    wcc, wlink = tool / "binnt64/wcc.exe", tool / "binnt64/wlink.exe"
    flags = ["-zq", "-2", "-ms", "-zl", "-s", "-os", "-zastd=c99", "-we",
             "-dCS_C16_REQUIRE_16BIT", "-i=" + str(tool / "h"), "-i=" + str(PROVIDER.parent)]
    core_obj = out / "block-read.obj"
    run([wcc, *flags, "-fo=" + str(core_obj), CORE], "core-compile")
    mfs_obj = out / "mfs-read.obj"
    if args.mfs_integration:
        run([wcc, *flags, "-fo=" + str(mfs_obj), mfs_source], "mfs-compile")
    provider_text = PROVIDER.read_text(encoding="utf-8")
    bad_head = "head = (uint16_t)((physical_sector / 15u) % 2u);"
    fail_return = "                    (uint16_t)(head << 8), provider->bounce) != 0)\n        return CS_BLOCK_IO_ERROR;"
    if provider_text.count(bad_head) != 1 or provider_text.count(fail_return) != 1:
        raise ValueError("mutation anchor changed")
    cases = [("baseline", None, False, 0xb8), ("late-fault", None, True, 0xb9),
             ("wrong-head", provider_text.replace(bad_head,
               "head = (uint16_t)(((physical_sector / 15u) % 2u) ^ 1u);"), False, 0xe2),
             ("ignored-carry", provider_text.replace(fail_return,
               "                    (uint16_t)(head << 8), provider->bounce) != 0)\n        { /* test-only ignored failure */ }"), True, 0xe4)]
    if args.mfs_integration:
        cases += [("mfs-header-entry", None, False, 0xba), ("mfs-header-fault", None, True, 0xbb)]
    observed = []
    for name, mutated, inject, marker in cases:
        is_mfs = name.startswith("mfs-")
        source = PROVIDER
        if mutated is not None:
            source = out / (name + ".c")
            source.write_text(mutated, encoding="utf-8")
        provider_obj, probe_obj = out / (name + "-provider.obj"), out / (name + "-probe.obj")
        run([wcc, *flags, "-fo=" + str(provider_obj), source], name + "-provider-compile")
        metadata = run([tool / "binnt64/dmpobj.exe", provider_obj], name + "-provider-object")
        if b"80286" not in metadata or b"USE16" not in metadata:
            raise ValueError("provider is not a native 80286 USE16 object")
        references = set(re.findall(rb"(?m)^\s*[0-9]+ - '([^']+)' Type:", metadata))
        definitions = set(re.findall(rb"(?m)^\s*[0-9a-fA-F]+h - '([^']+)' Type:", metadata))
        if references - definitions or re.search(rb"\b(?:COMDEF|LCOMDEF)\(", metadata):
            raise ValueError("provider acquired an external runtime dependency")
        fault_define = "-dCS_FLOPPY_MFS_EXPECT_FAULT" if is_mfs else "-dCS_FLOPPY_EXPECT_FAULT"
        run([wcc, *flags, *([fault_define] if inject else []),
             "-fo=" + str(probe_obj), mfs_probe if is_mfs else PROBE], name + "-probe-compile")
        binary, mapping = out / (name + ".bin"), out / (name + ".map")
        run([wlink, "format", "raw", "bin", "option", "start=start_", "option", "nodefaultlibs",
             "option", "map=" + str(mapping), "name", binary, "file",
             ",".join(str(p) for p in (core_obj, provider_obj, probe_obj, *((mfs_obj,) if is_mfs else ())))], name + "-link")
        text = mapping.read_text(encoding="utf-8", errors="replace")
        entry = re.search(r"Entry point address:\s*([0-9a-fA-F]+)", text)
        bss = re.search(r"^_BSS\s+BSS\s+DGROUP\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)", text, re.M)
        if not entry or not bss or ".lib" in text.lower():
            raise ValueError("missing entry/BSS or unexpected runtime library")
        payload = binary.read_bytes()
        bss_start, bss_bytes = (int(value, 16) for value in bss.groups())
        if not len(payload) <= 4096 or bss_start < len(payload):
            raise ValueError("payload outside full-inspection bounds")
        image = bytearray(loader.floppy(payload, int(entry.group(1), 16),
                          bss_end=bss_start + bss_bytes, poison_bss=True, verify_payload=True))
        for sector in range(60, 2400):
            image[sector * 512:(sector + 1) * 512] = bytes(
                (sector ^ (offset * 13) ^ (offset >> 1) ^ 0x5a) & 255 for offset in range(512))
        fixture_hashes = {}
        if is_mfs:
            # Original format values authored from SPEC-0112; no media imported.
            header = bytearray(512)
            struct.pack_into(">HIIHHHHHIIHIH", header, 0, 0xd2d7, 0x01234567, 0x89abcdef,
                             0x8081, 3, 4, 2, 31, 1024, 1536, 6, 0x10203040, 7)
            header[36:42] = b"\x05Lab\x00\x80"
            directory = bytearray(512)
            entry = bytearray(60)
            entry[0:2] = bytes((0xd1, 0x37))
            entry[2:18] = bytes(0xf0 ^ (i * 7) for i in range(16))
            struct.pack_into(">IHIIHIIII", entry, 18, 0x89abcdef, 0, 0, 0, 0, 0, 0,
                             0xdeadbeef, 0x76543210)
            entry[50:59] = b"\x08Native\x00\xff"
            directory[20:80] = entry
            for relative, data in ((2, header), (4, directory)):
                image[(60 + relative) * 512:(61 + relative) * 512] = data
                fixture_hashes[str(relative)] = hashlib.sha256(data).hexdigest()
        media = out / (name + ".img")
        with media.open("xb") as stream:
            stream.write(image)
        media_sha = sha(media)
        fault_chs = "2:0:3" if is_mfs else "2:1:1"
        response = json.loads(run([emulator, "--machine", "pc286", "--floppy", media,
                                   "--cycles", "8000000", "--timeout-ms", "20000", "--until-post", hex(marker),
                                   "--inspect", "0x10000:" + str(len(payload)),
                                   *(["--fail-floppy-read", fault_chs] if inject else [])], name + "-boot"))
        memory = response.pop("memory", {})
        if (response.get("status") != "condition_met" or response.get("post") != marker or
            response.get("cs") != 0x1000 or memory.get("address") != 0x10000 or
            bytes.fromhex(memory.get("bytes", "")) != payload or sha(media) != media_sha):
            raise ValueError(name + ": incorrect native result/payload or changed original media")
        fault = response.get("floppy_read_failure")
        if inject and (not fault or fault.get("injected") != 1 or
                       [fault.get(k) for k in ("cylinder", "head", "sector")] != [int(x) for x in fault_chs.split(":")]):
            raise ValueError("late fault was not the single requested firmware failure")
        observed.append({"case": name, "post": marker, "result": response,
                         "payload_bytes": len(payload), "payload_sha256": sha(binary),
                         "floppy_sha256": media_sha, "floppy_unchanged": True,
                         "full_loaded_payload_matches": True, "runtime_modules": [],
                         "bss_start": bss_start, "bss_bytes": bss_bytes,
                         "mfs_fixture_blocks_sha256": fixture_hashes})
    if before_sources != {p.relative_to(ROOT).as_posix(): sha(p) for p in sources}:
        raise ValueError("input source changed during verification")
    if sha(emulator) != emulator_sha or sha(contract) != contract_sha:
        raise ValueError("emulator or model-contract input changed")
    report = {"status": "pass", "schema_version": 1, "source_sha256": before_sources,
              "emulator_sha256": emulator_sha, "model_contract_sha256": contract_sha,
              "pc286_lock_sha256": sha(lock_path), "observed": observed, "commands": commands,
              "limits": "Original PC floppy diagnostic through BootHush built-in firmware; no physical IBM BIOS, "
                        "Mac GCR, protected media, mount/enumeration/allocation-map/fork validation, System boot or release claim."}
    (out / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"status": "pass", "cases": len(observed), "report": str(out / "report.json")}, indent=2))


if __name__ == "__main__":
    main()
