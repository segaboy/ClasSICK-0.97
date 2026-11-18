#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Bounded read-only BootHush observations of the owner-selected System Disk.

The disk is a private black-box runtime input supplied by path. Nothing from it
is written into the repository. The 286 lane copies its 409,600 logical data
bytes into a private disposable PC transfer image beside the report; the Mac
lane mounts the original Disk Copy 4.2 file read-only. The source file hash is
checked before and after every run. Results are observations, not boots.
"""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
PROVIDER = ROOT / "platform/pc286/boothush-floppy-read.c"
PROBE = ROOT / "probes/pc286/selected-disk-survey.c"
CORE = ROOT / "core/storage/block-read.c"
MFS = ROOT / "core/storage/mfs-read.c"
DATA_BYTES, TAG_BYTES, DC42_HEADER = 409600, 9600, 84
FIRST_SECTOR, BLOCKS = 60, 800
HEADER, FILE_BYTES, MAX_FILES = 112, 64, 40
MARK_DONE, MARK_EARLY = 0xBC, 0xBD
MFS_RESULTS = {0: "OK", 1: "ARGUMENT", 2: "SIGNATURE", 3: "GEOMETRY", 4: "ENTRY_BOUNDS",
               5: "INACTIVE", 6: "UNAVAILABLE", 7: "IO", 8: "PROVIDER"}


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def dc42(path):
    raw = Path(path).read_bytes()
    name_length = raw[0]
    data, tags = struct.unpack_from(">II", raw, 0x40)
    encoding, fmt, magic = raw[0x50], raw[0x51], struct.unpack_from(">H", raw, 0x52)[0]
    if (len(raw) != DC42_HEADER + DATA_BYTES + TAG_BYTES or name_length > 63 or
            data != DATA_BYTES or tags != TAG_BYTES or encoding != 0 or magic != 0x0100):
        raise ValueError("not the declared 400 KiB tagged Disk Copy 4.2 container")
    return raw, {"bytes": len(raw), "sha256": hashlib.sha256(raw).hexdigest(),
                 "container": "Disk Copy 4.2", "data_bytes": data, "tag_bytes": tags,
                 "encoding_byte": encoding, "format_byte": fmt, "private_magic": magic,
                 "header_name_length": name_length}


def u16(b, at):
    return b[at] | b[at + 1] << 8


def u32(b, at):
    return u16(b, at) | u16(b, at + 2) << 16


def decode_report(b, private):
    rep = {"stage": b[4], "header_result": MFS_RESULTS.get(b[5], b[5]),
           "master_read_failed": bool(b[6]), "layout_version": b[7]}
    if b[4] < 2:
        return rep
    rep["volume"] = {"created": u32(b, 8), "backup": u32(b, 12), "attributes": u16(b, 16),
                     "file_count": u16(b, 18), "dir_first": u16(b, 20), "dir_blocks": u16(b, 22),
                     "alloc_count": u16(b, 24), "alloc_bytes": u32(b, 26), "clump_bytes": u32(b, 30),
                     "alloc_first": u16(b, 34), "next_file": u32(b, 36), "free_blocks": u16(b, 40),
                     "name_length": b[42]}
    if private:
        rep["volume"]["name_hex"] = bytes(b[43:43 + min(b[42], 27)]).hex()
    if b[4] < 3:
        return rep
    rep["enumeration"] = {"active_entries": u16(b, 70), "entry_bounds_failures": u16(b, 72),
                          "blocks_with_nonzero_tail": u16(b, 74), "directory_read_errors": u16(b, 76)}
    names = ("A_high_nibble_first", "B_low_nibble_first")
    rep["packings"] = {}
    for i, label in enumerate(names):
        base = 78 + i * 16
        rep["packings"][label] = dict(zip(
            ("forks_checked", "forks_consistent", "forks_bad_link", "forks_loop",
             "forks_length_mismatch", "map_used", "map_unused", "map_invalid"),
            (u16(b, base + 2 * k) for k in range(8))))
    if private:
        files = []
        for n in range(b[110]):
            at = HEADER + n * FILE_BYTES
            files.append({"flags": b[at], "name_length": b[at + 1],
                          "name_hex": bytes(b[at + 34:at + 34 + min(b[at + 1], 30)]).hex(),
                          "file_id": u32(b, at + 30),
                          "data": {"start": u16(b, at + 2), "physical": u32(b, at + 4), "logical": u32(b, at + 8),
                                   "chain_A": u16(b, at + 22), "chain_B": u16(b, at + 26)},
                          "resource": {"start": u16(b, at + 12), "physical": u32(b, at + 14),
                                       "logical": u32(b, at + 18), "chain_A": u16(b, at + 24),
                                       "chain_B": u16(b, at + 28)}})
        rep["files"] = files
    return rep


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--disk", type=Path, required=True)
    parser.add_argument("--tool-root", type=Path, required=True)
    parser.add_argument("--emulator", type=Path, required=True)
    parser.add_argument("--rom", type=Path, required=True, help="published ClasSICK Mac ROM for the baseline lane")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--cold-starts", type=int, default=3)
    args = parser.parse_args()
    disk, tool, emulator, rom = (p.resolve(strict=True) for p in (args.disk, args.tool_root, args.emulator, args.rom))
    out = args.output.resolve()
    if out == ROOT or ROOT in out.parents:
        raise ValueError("output must be a new external directory")
    raw, identity = dc42(disk)
    lock_path = ROOT / "scripts/pc286-toolchain-lock.json"
    lock = json.loads(lock_path.read_text(encoding="utf-8"))
    for item in lock["selected_members"]:
        path = tool / item["path"]
        if path.stat().st_size != item["bytes"] or sha(path) != item["sha256"]:
            raise ValueError("compiler identity mismatch: " + item["path"])
    sources = [PROVIDER, PROVIDER.with_suffix(".h"), PROBE, CORE, CORE.with_suffix(".h"), MFS,
               MFS.with_suffix(".h"), ROOT / "arch/i286/c16.h", ROOT / "scripts/pc286_loader.py",
               Path(__file__).resolve()]
    before_sources = {p.relative_to(ROOT).as_posix(): sha(p) for p in sources}
    emulator_sha, rom_sha = sha(emulator), sha(rom)
    spec = importlib.util.spec_from_file_location("floppy_loader", ROOT / "scripts/pc286_loader.py")
    loader = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(loader)
    out.mkdir(parents=True, exist_ok=False)
    env = os.environ.copy()
    for key in ("WCC", "WATCOM", "INCLUDE", "LIB", "WCL", "WLINK"):
        env.pop(key, None)
    commands, checks = [], []

    def run(command, label, timeout=60, check=True):
        command = [str(x) for x in command]
        commands.append(command)
        result = subprocess.run(command, cwd=out, env=env, capture_output=True, timeout=timeout,
                                creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        (out / (label + ".log")).write_bytes(result.stdout + b"\n" + result.stderr)
        if check and result.returncode:
            raise ValueError(label + " failed; see " + str(out / (label + ".log")))
        return result.stdout

    def disk_unchanged(label):
        ok = sha(disk) == identity["sha256"]
        checks.append({"after": label, "selected_disk_unchanged": ok})
        if not ok:
            raise ValueError("selected disk changed after " + label)

    # Mac128K lane: published ROM, original DC42 mounted read-only.
    mac_rows = []
    for n in range(1, args.cold_starts + 1):
        frame = out / ("mac-cold-%d.pbm" % n)
        result = json.loads(run([emulator, "--machine", "mac128k", "--rom", rom, "--floppy", disk,
                                 "--cycles", "100000000", "--timeout-ms", "30000", "--mac-floppy-state", "1",
                                 "--mac-cpu-state", "1", "--frame", frame], "mac-cold-%d" % n))
        disk_unchanged("mac-cold-%d" % n)
        mac_rows.append({"run": n, "result": result, "frame_sha256": sha(frame)})
    first = mac_rows[0]
    mac_repeatable = all(r["result"].get("cycles") == first["result"].get("cycles") and
                         r["result"].get("pc") == first["result"].get("pc") and
                         r["frame_sha256"] == first["frame_sha256"] for r in mac_rows)

    # Native 286 lane: original survey payload, private transfer image.
    wcc, wlink = tool / "binnt64/wcc.exe", tool / "binnt64/wlink.exe"
    flags = ["-zq", "-2", "-ms", "-zl", "-s", "-os", "-zastd=c99", "-we",
             "-dCS_C16_REQUIRE_16BIT", "-i=" + str(tool / "h"), "-i=" + str(PROVIDER.parent)]
    objects = []
    for source, label in ((CORE, "block-read"), (MFS, "mfs-read"), (PROVIDER, "provider"), (PROBE, "survey")):
        obj = out / (label + ".obj")
        run([wcc, *flags, "-fo=" + str(obj), source], label + "-compile")
        metadata = run([tool / "binnt64/dmpobj.exe", obj], label + "-object")
        if b"80286" not in metadata or b"USE16" not in metadata:
            raise ValueError(label + " is not a native 80286 USE16 object")
        objects.append(obj)
    binary, mapping = out / "survey.bin", out / "survey.map"
    run([wlink, "format", "raw", "bin", "option", "start=start_", "option", "nodefaultlibs",
         "option", "map=" + str(mapping), "name", binary, "file", ",".join(str(p) for p in objects)], "survey-link")
    text = mapping.read_text(encoding="utf-8", errors="replace")
    entry = re.search(r"Entry point address:\s*([0-9a-fA-F]+)", text)
    bss = re.search(r"^_BSS\s+BSS\s+DGROUP\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)", text, re.M)
    symbol = re.search(r"(?m)^([0-9a-fA-F]{8})[*+]?\s+_?cs_survey_report_?\s*$", text)
    if not entry or not bss or not symbol or ".lib" in text.lower():
        raise ValueError("missing entry/BSS/report symbol or unexpected runtime library")
    payload = binary.read_bytes()
    bss_start, bss_bytes = (int(v, 16) for v in bss.groups())
    report_at = int(symbol.group(1), 16)
    if not (len(payload) <= 12288 and bss_start >= len(payload) and
            bss_start <= report_at <= bss_start + bss_bytes - (HEADER + MAX_FILES * FILE_BYTES)):
        raise ValueError("payload/report outside inspection bounds")
    base_image = loader.floppy(payload, int(entry.group(1), 16), bss_end=bss_start + bss_bytes,
                               poison_bss=True, verify_payload=True)
    data = raw[DC42_HEADER:DC42_HEADER + DATA_BYTES]
    transfer = bytearray(base_image)
    transfer[FIRST_SECTOR * 512:(FIRST_SECTOR + BLOCKS) * 512] = data
    transfer_path = out / "private-transfer-286.img"
    transfer_path.write_bytes(transfer)
    blank_path = out / "original-no-volume-286.img"
    blank_path.write_bytes(base_image)
    transfer_sha, blank_sha = sha(transfer_path), sha(blank_path)
    inspect = []
    for at in range(0, len(payload), 4096):
        inspect += ["--inspect", "0x%x:%d" % (0x10000 + at, min(4096, len(payload) - at))]
    inspect += ["--inspect", "0x%x:%d" % (0x10000 + report_at, HEADER + MAX_FILES * FILE_BYTES)]
    cases = [("selected-cold-%d" % n, transfer_path, None, MARK_DONE) for n in range(1, args.cold_starts + 1)]
    cases += [("selected-header-fault", transfer_path, "2:0:3", MARK_EARLY),
              ("original-no-volume", blank_path, None, MARK_EARLY)]
    rows = []
    for name, media, fault, marker in cases:
        media_sha = sha(media)
        response = json.loads(run([emulator, "--machine", "pc286", "--floppy", media, "--cycles", "40000000",
                                   "--timeout-ms", "30000", "--until-post", hex(marker), *inspect,
                                   *(["--fail-floppy-read", fault] if fault else [])], name))
        disk_unchanged(name)
        ranges = response.pop("memory_ranges", None) or [response.pop("memory")]
        loaded = b"".join(bytes.fromhex(r["bytes"]) for r in ranges[:-1])
        report_bytes = bytes.fromhex(ranges[-1]["bytes"])
        if (response.get("status") != "condition_met" or response.get("post") != marker or
                loaded != payload or report_bytes[:4] != b"CSSV" or sha(media) != media_sha):
            raise ValueError(name + ": unexpected stop, payload, report or changed media")
        rows.append({"case": name, "media": media.name, "media_sha256": media_sha, "fault": fault,
                     "post": marker, "result": response, "report_sha256": hashlib.sha256(report_bytes).hexdigest(),
                     "private": decode_report(report_bytes, True), "public": decode_report(report_bytes, False)})
    selected = [r for r in rows if r["case"].startswith("selected-cold-")]
    survey_repeatable = len({r["report_sha256"] for r in selected}) == 1 and \
        len({r["result"].get("cycles") for r in selected}) == 1
    fault_row = next(r for r in rows if r["case"] == "selected-header-fault")
    blank_row = next(r for r in rows if r["case"] == "original-no-volume")
    controls = {"header_fault_returns_io": fault_row["public"].get("header_result") == "IO" and
                fault_row["result"].get("floppy_read_failure", {}).get("injected") == 1,
                "no_volume_returns_signature": blank_row["public"].get("header_result") == "SIGNATURE"}
    if before_sources != {p.relative_to(ROOT).as_posix(): sha(p) for p in sources}:
        raise ValueError("input source changed during observation")
    if sha(emulator) != emulator_sha or sha(rom) != rom_sha:
        raise ValueError("emulator or ROM changed")
    disk_unchanged("final")
    common = {"schema_version": 1, "classification": "bounded read-only black-box observation; not a boot",
              "disk": {"path": str(disk), **identity}, "emulator_sha256": emulator_sha, "rom_sha256": rom_sha,
              "source_sha256": before_sources, "pc286_lock_sha256": sha(lock_path),
              "payload_sha256": sha(binary), "payload_bytes": len(payload), "bss": [bss_start, bss_bytes],
              "transfer": {"first_sector": FIRST_SECTOR, "blocks": BLOCKS, "transfer_sha256": transfer_sha,
                           "no_volume_sha256": blank_sha, "tags_carried": False},
              "mac_repeatable": mac_repeatable, "survey_repeatable": survey_repeatable,
              "controls": controls, "disk_checks": checks}
    private = dict(common, mac=mac_rows, native286=rows, commands=commands)
    public = dict(common, mac=[{"run": r["run"], "status": r["result"].get("status"),
                                "cycles": r["result"].get("cycles"), "pc": r["result"].get("pc"),
                                "mac_floppy": r["result"].get("mac_floppy"), "frame_sha256": r["frame_sha256"]}
                               for r in mac_rows],
                  native286=[{k: r[k] for k in ("case", "media_sha256", "fault", "post", "report_sha256", "public")} |
                             {"cycles": r["result"].get("cycles")} for r in rows])
    public["disk"] = {k: v for k, v in public["disk"].items() if k != "path"}
    (out / "private-report.json").write_text(json.dumps(private, indent=2) + "\n", encoding="utf-8")
    (out / "report.json").write_text(json.dumps(public, indent=2) + "\n", encoding="utf-8")
    ok = mac_repeatable and survey_repeatable and all(controls.values())
    print(json.dumps({"status": "pass" if ok else "observed-with-differences", "report": str(out / "report.json"),
                      "mac_repeatable": mac_repeatable, "survey_repeatable": survey_repeatable,
                      "controls": controls}, indent=2))


if __name__ == "__main__":
    main()
