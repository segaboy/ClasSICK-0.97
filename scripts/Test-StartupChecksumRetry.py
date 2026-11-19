#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Check native startup checksum recovery on original generated media only."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
import sys

from startup_files_fixture import original_media


ROOT = Path(__file__).resolve().parents[1]
EMULATOR = "5fa45b7542409300b7604606a78c81d4ee95e888a57c592a83d0cec43ce1056e"
OLD_ROM = "94e95ba320b51d14ffd6fec646403b2135310fc94d9dfbe6946928169dea1fb1"
DISK = "79aa17dd27c2ab48d384f9bf5ee542a2ab3e9e477232ac6eefe204a1cd9099d8"


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def verify(exe, disk, rom, diag, report, counter, mode, oracle):
    spans = [(diag, 28), (report, 64)]
    if counter is not None:
        spans.append((counter, 4))
    cmd = [str(exe), "--machine", "mac128k", "--rom", str(rom), "--floppy", str(disk),
           "--cycles", "200000000", "--timeout-ms", "30000", "--until", f"{diag}:4:184"]
    for address, length in spans:
        cmd.extend(["--inspect", f"{address}:{length}"])
    if mode != "healthy":
        option = "--mac-floppy-bad-checksum" if mode == "persistent" else "--mac-floppy-bad-checksum-once"
        cmd.extend([option, ":".join(str(n) for n in oracle["mac_fault"])])
    result = json.loads(subprocess.run(cmd, capture_output=True, check=True, timeout=60,
                                      creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0)).stdout)
    memories = result.pop("memory_ranges", [])
    if result["status"] != "condition_met" or len(memories) != len(spans):
        raise ValueError(f"{mode} did not complete the expected guest probe")
    observed = []
    for memory, (address, length) in zip(memories, spans):
        data = bytes.fromhex(memory["bytes"])
        if memory["address"] != address or len(data) != length:
            raise ValueError("unexpected guest RAM extent")
        observed.append(list(struct.unpack(f">{length//4}I", data)))
    retries = int(counter is not None and mode != "healthy")
    refused = mode == "persistent" or (mode == "once" and counter is None)
    if observed[:2] != [[184] + [0]*6, oracle["fault_expected" if refused else "expected"]]:
        raise ValueError(f"{mode} diagnostic or startup result differs: {observed}")
    if counter is not None and observed[2] != [retries]:
        raise ValueError("guest did not perform exactly the expected retries")
    injection = result.get("mac_floppy_checksum_fault")
    expected_fault = None if mode == "healthy" else {
        "kind": "synthetic_gcr_data_checksum", "mode": mode, "track": oracle["mac_fault"][0],
        "sector": oracle["mac_fault"][1], "altered_nibbles": 1+int(mode == "persistent" and retries == 1)}
    if injection != expected_fault:
        raise ValueError(f"unexpected fault delivery: {injection}")
    return {"case": mode, "command": cmd, "result": result, "startup_report": observed[1],
            "guest_checksum_retries": observed[2][0] if counter is not None else None}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("emulator", "single-attempt-rom", "candidate-dir", "output"):
        parser.add_argument("--"+name, required=True, type=Path)
    args = parser.parse_args()
    receipt = args.candidate_dir/"report.json"
    exe, control = args.emulator.resolve(strict=True), args.single_attempt_rom.resolve(strict=True)
    rom = (args.candidate_dir/"mac.rom").resolve(strict=True)
    symbols = (args.candidate_dir/"mac-symbols.stdout").read_text(encoding="utf-8")
    build_report = json.loads(receipt.read_text(encoding="utf-8"))
    build = build_report["builds"]["mac128k"]
    match = re.search(r"^([a-fA-F0-9]+) B cs_startup_checksum_retries$", symbols, re.M)
    if (build_report["status"] != "pass" or not match or digest(exe) != EMULATOR or
            digest(control) != OLD_ROM or digest(rom) != build["rom_sha256"] or
            not build["bss_start"] <= int(match[1], 16) <= build["bss_end"]-4):
        raise ValueError("emulator, control, candidate or diagnostic identity differs")
    logical, oracle = original_media()
    if hashlib.sha256(logical).hexdigest() != DISK:
        raise ValueError("original generated fixture identity differs")
    out = args.output.resolve()
    if out == ROOT or ROOT in out.parents:
        raise ValueError("use a new output directory outside the repository")
    out.mkdir(parents=True, exist_ok=False)
    disk = out/"generated-startup.raw"
    disk.write_bytes(logical)
    inputs = [control, rom, exe, disk, receipt, args.candidate_dir/"mac-symbols.stdout",
              Path(__file__).resolve(), Path(sys.executable).resolve(),
              ROOT/"scripts/startup_files_fixture.py", ROOT/"scripts/resource_media_fixture.py"]
    before = {str(path): digest(path) for path in inputs}
    results = []
    for label, source, diag, report, counter in [
            ("single-attempt", control, 12420, 2816, None),
            ("startup-retry", rom, build["diag_address"], build["report_address"], int(match[1], 16))]:
        for mode in ("healthy", "once", "persistent"):
            observation = verify(exe, disk, source, diag, report, counter, mode, oracle)
            results.append(dict(rom=label, **observation))
    if before != {str(path): digest(path) for path in inputs}:
        raise ValueError("input changed")
    (out/"report.json").write_text(json.dumps({"status":"pass", "inputs":before, "runs":results},
                                               indent=2)+"\n", encoding="utf-8")
    print(f"Passed all {len(results)} startup runs. Report at {out/'report.json'}")


if __name__ == "__main__":
    main()
