#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Observe the qualified developer ROM and installed 286 using one identified System Disk."""
import argparse
from importlib import import_module
import json
from pathlib import Path
import struct
from tempfile import TemporaryDirectory

from m68k_windows_toolchain import sha
from pc286_hard_disk_loader import DISK_BYTES

builder = import_module("Build-DiskStartup")
observer = import_module("Test-BootHandoff")
dc42_data = import_module("Test-MfsVolumeMedia").dc42_data


def checked_inputs(qualified, report_hash, emulator, selected):
    report = qualified / "report.json"
    if sha(report) != report_hash:
        raise ValueError("generated qualification receipt identity differs")
    qualification = json.loads(report.read_text())
    package = qualified / "a"
    manifest = json.loads((package / "manifest.json").read_text())
    artifacts = manifest["artifacts"]
    if (qualification["status"] != "generated-controls-pass" or
            qualification["manifest_sha256"] != sha(package / "manifest.json") or
            qualification["emulator_sha256"] != observer.PIN or sha(emulator) != observer.PIN or
            manifest["profile"] != "disk-startup-developer" or manifest["specification"] != "SPEC-0141" or
            manifest["sources"] != builder.source_hashes() or
            set(artifacts) != {builder.ROM, builder.INSTALLER} or
            qualification["installed_sha256"] != manifest["pc"]["installed_disk_sha256"] or
            sha(qualified / "installed.img") != qualification["installed_sha256"]):
        raise ValueError("generated qualification, developer source or installed disk differs")
    for name, expected in {**artifacts, "manifest.json": qualification["manifest_sha256"]}.items():
        if sha(package / name) != expected or sha(qualified / "b" / name) != expected:
            raise ValueError("developer artifact twins differ")
    if builder.ROOT in selected.parents or sha(selected) != observer.SELECTED:
        raise ValueError("selected external DC42 identity differs")
    return manifest


def install(exe, installer, expected, scratch):
    blank, fixed = scratch / "blank.img", scratch / "installed.img"
    blank.write_bytes(bytes(DISK_BYTES))
    initial = sha(blank)
    keys = []
    for i, key in enumerate((0x43, 0x31, 0x1b, 0x2c, 0x1c, 0x4b, 0x4b, 0x5a)):
        for delta, code in ((0, key), (10000, 0xf0), (20000, key)):
            keys.extend(["--scancode", f"{10000000+i*100000+delta}:{code}"])
    row = observer.guest(exe, "pc286", ["--floppy", installer, "--hard-disk", blank, "--disk-out", fixed,
                                        "--until-post", "0xac", *keys], 600000000)
    if row.get("status") != "condition_met" or row.get("post") != 0xac or sha(blank) != initial or sha(fixed) != expected:
        raise ValueError("fresh installation differs")
    return fixed


def pc_stop(exe, fixed, carrier, predicate):
    args = ["--boot-drive", "hard-disk", "--hard-disk", fixed, "--floppy", carrier, *predicate]
    row = observer.guest(exe, "pc286", args, 250000000)
    stop = row.get("status")
    if stop not in ("condition_met", "halted", "machine_fault", "cycle_limit", "timeout"):
        raise ValueError("unrecognized native 286 stop")
    pc, cycles = row.get("pc"), row.get("cycles")
    if type(pc) != int or not 0 <= pc <= 0xffffff or type(cycles) != int or not 0 <= cycles <= 250000016:
        raise ValueError("invalid native PC or cycle count")
    result = dict(stop=stop, pc=pc, cycles=cycles)
    # Only the three failure codes emitted by our native startup program.
    if row.get("post") in (0xd0, 0xd1, 0xd2):
        result["startup_post"] = row["post"]
    return result


def observe(exe, manifest, rom, fixed, mac_disk, carrier):
    mac = manifest["mac"]
    aline = mac["exception_vectors"][10-2]
    if aline == mac["startup_stop_pc"]:
        raise ValueError("expected a dedicated Macintosh Line-A vector")
    entry = observer.mac_summary(exe, rom, mac_disk, ["--until-pc", "0x8002"])
    first = observer.mac_summary(exe, rom, mac_disk, ["--until-mac-ram-steps", "0x8002:2:1"])
    bounded = observer.mac_summary(exe, rom, mac_disk, ["--until-mac-ram-steps", "0x8000:1024:128"])
    vector = observer.mac_summary(exe, rom, mac_disk, ["--until-pc", str(aline)])
    stop = observer.mac_summary(exe, rom, mac_disk, ["--until-pc", str(mac["startup_stop_pc"])])
    pc = manifest["pc"]
    native = pc_stop(exe, fixed, carrier, ["--until-pc", str(pc["start_pc"])])
    entries = []
    after = 0
    # Every replay begins with the same cold hard disk. A hit increments the
    # cycle threshold so a later hit must be a distinct visit to this PC.
    for _ in range(8):
        row = pc_stop(exe, fixed, carrier, ["--until-pc", str(pc["startup_step_pc"]), "--until-pc-after", str(after)])
        entries.append(row)
        if row["stop"] != "condition_met":
            break
        if row["pc"] != pc["startup_step_pc"] or row["cycles"] < after:
            raise ValueError("native dispatch stop did not match its requested boundary")
        after = row["cycles"] + 1
    terminal = pc_stop(exe, fixed, carrier, ["--until-post", str(0xd2)])
    return dict(mac_entry=entry, mac_first=first, mac_bounded=bounded, mac_line_a=vector, mac_common_stop=stop,
                pc_native_entry=native, pc_dispatch_visits=entries, pc_runtime_failure=terminal)


def run(args):
    exe = args.emulator.resolve(strict=True)
    qualified = args.qualification.resolve(strict=True)
    source = args.disk.resolve(strict=True)
    out = args.output.resolve()
    manifest = checked_inputs(qualified, args.qualification_sha256, exe, source)
    observer.fresh(out)
    package = qualified / "a"
    rom = package / builder.ROM
    with TemporaryDirectory(dir=out) as temporary:
        scratch = Path(temporary)
        expected = manifest["pc"]["installed_disk_sha256"]
        fixed = install(exe, package / builder.INSTALLER, expected, scratch)
        # Generated cross-sector branch, MOVEQ, GetTrapAddress, then ILLEGAL.
        # The Macintosh should enter its dedicated Line-A vector. The 286
        # implements GetTrapAddress and visits its native dispatcher four times.
        control = observer.original.fixture(0)
        struct.pack_into(">HHH", control, 510, 0x7046, 0xa146, 0x4afc)
        raw, carrier = scratch / "control.raw", scratch / "transport.img"
        raw.write_bytes(control)
        carrier.write_bytes(control.ljust(1228800, b"\0"))
        control_ids = [sha(p) for p in (raw, carrier)]
        known = observe(exe, manifest, rom, fixed, raw, carrier)
        visits = known["pc_dispatch_visits"]
        if (not known["mac_entry"]["condition_met"] or not known["mac_first"]["condition_met"] or
                known["mac_first"]["pc"] != 0x81fe or not known["mac_line_a"]["condition_met"] or
                not known["mac_common_stop"]["condition_met"] or known["mac_bounded"]["returned"] != 3 or
                known["pc_native_entry"]["stop"] != "condition_met" or len(visits) != 5 or
                any(row["stop"] != "condition_met" for row in visits[:4]) or
                visits[-1].get("startup_post") != 0xd2 or known["pc_runtime_failure"]["stop"] != "condition_met" or
                [sha(p) for p in (raw, carrier)] != control_ids or sha(fixed) != expected):
            raise ValueError("generated dispatch, Mac vector or runtime failure control differs")
        logical = dc42_data(source)
        carrier.write_bytes(logical.ljust(1228800, b"\0"))
        carrier_id = sha(carrier)
        trials = [observe(exe, manifest, rom, fixed, source, carrier) for _ in range(3)]
        if not all(row == trials[0] for row in trials) or sha(fixed) != expected or sha(carrier) != carrier_id:
            raise ValueError("selected cold repetitions or media identities differ")
    checked_inputs(qualified, args.qualification_sha256, exe, source)
    report = dict(status="bounded-observation", qualification_sha256=args.qualification_sha256,
                  manifest_sha256=sha(package / "manifest.json"), emulator_sha256=observer.PIN,
                  artifacts=manifest["artifacts"], installed_sha256=expected, selected_sha256=observer.SELECTED,
                  runner_sha256=sha(Path(__file__)), numeric_observer_sha256=sha(Path(observer.__file__)),
                  generated_control_sha256=control_ids[0], generated_control=known, selected_repetitions=trials)
    (out / "report.json").write_text(json.dumps(report, indent=2)+"\n", encoding="utf-8")
    print(report["status"])


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("emulator", "qualification", "disk", "output"):
        parser.add_argument("--"+name, type=Path, required=True)
    parser.add_argument("--qualification-sha256", required=True)
    run(parser.parse_args())
