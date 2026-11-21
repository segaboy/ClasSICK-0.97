#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Observe our linked ROM service entries and native dispatcher on the admitted System Disk."""
import argparse
from importlib import import_module
import json
from pathlib import Path
from tempfile import TemporaryDirectory

from m68k_windows_toolchain import sha

prior = import_module("Test-InstalledDiskStartup")
builder, observer = prior.builder, prior.observer
EMULATOR = "543767ae09f39d330803c05bd34311b3b3f485e53f00ec814c54c82d9c8a329f"
MAC_ENTRIES = ("line_a_pc", "get_trap_address_pc", "set_trap_address_pc", "service_return_pc", "startup_stop_pc")


def mac_entries(manifest):
    if "new_ptr_pc" in manifest["mac"]:
        return MAC_ENTRIES + ("new_ptr_dispatch_pc", "new_ptr_success_pc", "new_ptr_failure_pc")
    return MAC_ENTRIES


def check_inputs(exe, qualified, expected_report, source):
    receipt = qualified / "report.json"
    if sha(receipt) != expected_report:
        raise ValueError("generated qualification receipt differs")
    public = json.loads(receipt.read_text())
    bundle = qualified / "a"
    manifest = json.loads((bundle / "manifest.json").read_text())
    names = {builder.ROM, builder.INSTALLER}
    expected = dict(manifest["artifacts"])
    if (public["status"] != "generated-controls-pass" or sha(exe) != EMULATOR or public["emulator_sha256"] != EMULATOR or
            manifest["profile"] != "disk-startup-developer" or manifest["specification"] != "SPEC-0141" or
            set(expected) != names or manifest["sources"] != builder.source_hashes() or
            sha(qualified / "installed.img") != public["installed_sha256"] or
            public["installed_sha256"] != manifest["pc"]["installed_disk_sha256"]):
        raise ValueError("qualified runtime inputs differ")
    expected["manifest.json"] = public["manifest_sha256"]
    for name, digest in expected.items():
        if any(sha(qualified / twin / name) != digest for twin in ("a", "b")):
            raise ValueError("build twin or manifest identity differs")
    pcs = [manifest["mac"][name] for name in mac_entries(manifest)]
    if (len(set(pcs)) != len(pcs) or any(type(n) != int or n & 1 or not 0x400000 <= n < 0x410000 for n in pcs)
            or manifest["mac"]["line_a_pc"] != manifest["mac"]["exception_vectors"][8]):
        raise ValueError("expected distinct, linked Mac service boundaries")
    if builder.ROOT in source.parents or sha(source) != observer.SELECTED:
        raise ValueError("selected external media identity differs")
    return manifest


def mac_boundary(exe, manifest, rom, floppy):
    def run(*args):
        return observer.mac_summary(exe, rom, floppy, args)

    steps = 4096 if "new_ptr_pc" in manifest["mac"] else 128
    linked = {name: run("--until-pc", str(manifest["mac"][name])) for name in mac_entries(manifest)}
    result = dict(entry=run("--until-pc", "0x8002"), first=run("--until-mac-ram-steps", "0x8002:2:1"),
                  loaded_steps=run("--until-mac-ram-steps", f"0x8000:1024:{steps}"), linked_entries=linked)
    success = linked.get("new_ptr_success_pc")
    if success and success["condition_met"]:
        # The fixture first calls GetTrapAddress. Its RTE cannot count as
        # evidence that the later allocation returned to the guest.
        def after(name, event):
            return run("--until-pc", str(manifest["mac"][name]), "--until-pc-after", str(event["cycles"] + 1))
        returned = after("service_return_pc", success)
        result["return_after_allocation"] = returned
        if returned["condition_met"]:
            result["next_line_a"] = after("line_a_pc", returned)
    return result


def native_boundary(exe, manifest, installed, carrier, target):
    pc = manifest["pc"]
    args = ["--boot-drive", "hard-disk", "--hard-disk", installed, "--floppy", carrier,
            "--until-pc", str(pc["startup_step_pc"]), "--until-pc-visits", str(target)]
    row = observer.guest(exe, "pc286", args, 250000000)
    stop = row.get("status")
    if stop not in ("condition_met", "halted", "cycle_limit", "machine_fault"):
        raise ValueError("unexpected native dispatcher stop")
    visits = row["pc_visits"]
    count = visits["count"]
    first, last = visits["first_cycle"], visits["last_cycle"]
    if (visits["address"] != pc["startup_step_pc"] or visits["target"] != target or type(count) != int or
            not 0 <= count <= target or (stop == "condition_met") != (count == target) or
            any(type(n) != int for n in (first, last)) or not 0 <= first <= last <= row["cycles"] or
            (count == 0 and (first or last))):
        raise ValueError("invalid dispatcher count")
    location, cycles = row["pc"], row["cycles"]
    if type(location) != int or not 0 <= location <= 0xffffff or type(cycles) != int or not 0 <= cycles <= 250000016:
        raise ValueError("invalid native observation")
    dispatch = dict(stop=stop, visits=count, pc=location, cycles=cycles, first_cycle=first, last_cycle=last)
    result = dict(entry=prior.pc_stop(exe, installed, carrier, ["--until-pc", str(pc["start_pc"])]),
                  dispatcher=dispatch, failure=prior.pc_stop(exe, installed, carrier, ["--until-post", "0xd2"]))
    if "new_ptr_pc" in pc:
        result["new_ptr"] = prior.pc_stop(exe, installed, carrier, ["--until-pc", str(pc["new_ptr_pc"])])
    return result


def observe(exe, manifest, rom, installed, mac_disk, carrier, target):
    return dict(mac=mac_boundary(exe, manifest, rom, mac_disk),
                pc=native_boundary(exe, manifest, installed, carrier, target))


def run(args):
    exe, qualified, source = [p.resolve(strict=True) for p in (args.emulator, args.qualification, args.disk)]
    manifest = check_inputs(exe, qualified, args.qualification_sha256, source)
    out = args.output.resolve()
    observer.fresh(out)
    bundle = qualified / "a"
    rom = bundle / builder.ROM
    expected = manifest["pc"]["installed_disk_sha256"]
    with TemporaryDirectory(dir=out) as temporary:
        work = Path(temporary)
        installed = prior.install(exe, bundle / builder.INSTALLER, expected, work)
        # The common fixture exercises GetTrapAddress before a deliberate
        # illegal instruction. On allocation-capable builds it also requires
        # noErr, a nonnull pointer, and a successful write and readback.
        fixture = observer.original.fixture(0)
        expect_visits = 4
        control = bytes.fromhex("7046 a146")
        allocator = "new_ptr_pc" in manifest["mac"]
        if allocator != ("new_ptr_pc" in manifest["pc"]):
            raise ValueError("expected paired allocation entries")
        if allocator:
            control += bytes.fromhex("7010 a11e 6702 4afc 2208 6602 4afc "
                                     "20bc 4a613731 0c90 4a613731 6702 4afc")
            expect_visits = 12
        control += bytes.fromhex("4afc")
        fixture[510:510+len(control)] = control
        raw, carrier = work / "fixture.raw", work / "transport.img"
        raw.write_bytes(fixture)
        carrier.write_bytes(fixture.ljust(1228800, b"\0"))
        fixture_id, transport_id = sha(raw), sha(carrier)
        positive = observe(exe, manifest, rom, installed, raw, carrier, 128)
        linked = positive["mac"]["linked_entries"]
        native = positive["pc"]
        if (not positive["mac"]["entry"]["condition_met"] or positive["mac"]["first"]["pc"] != 0x81fe or
                any(not linked[key]["condition_met"] for key in ("line_a_pc", "get_trap_address_pc", "service_return_pc")) or
                linked["set_trap_address_pc"]["condition_met"] or
                native["entry"]["stop"] != "condition_met" or native["dispatcher"]["visits"] != expect_visits or
                native["dispatcher"]["stop"] != "halted" or native["failure"].get("startup_post") != 0xd2 or
                native["failure"]["stop"] != "condition_met" or sha(raw) != fixture_id or sha(carrier) != transport_id):
            raise ValueError("independent generated service or dispatcher control differs")
        if allocator:
            end_pc = 0x8000 + 510 + len(control) - 2
            if (not linked["new_ptr_dispatch_pc"]["condition_met"] or
                    not positive["mac"].get("return_after_allocation", {}).get("condition_met") or
                    linked["new_ptr_failure_pc"]["condition_met"] or
                    positive["mac"]["loaded_steps"]["last_pc"] != end_pc or
                    native["new_ptr"]["stop"] != "condition_met"):
                raise ValueError("allocation fixture failed to reach its checked continuation")
        carrier.write_bytes(prior.dc42_data(source).ljust(1228800, b"\0"))
        transport_id = sha(carrier)
        trials = [observe(exe, manifest, rom, installed, source, carrier, 128) for _ in range(3)]
        if any(trial != trials[0] for trial in trials[1:]) or sha(installed) != expected or sha(carrier) != transport_id:
            raise ValueError("selected repetitions or disk identities differ")
    check_inputs(exe, qualified, args.qualification_sha256, source)
    report = dict(status="bounded-observation", qualification_sha256=args.qualification_sha256,
                  selected_sha256=observer.SELECTED, emulator_sha256=sha(exe), installed_sha256=expected,
                  artifacts=manifest["artifacts"], manifest_sha256=sha(bundle / "manifest.json"),
                  observation_scripts={Path(mod.__file__).name:sha(Path(mod.__file__)) for mod in (prior, observer)},
                  runner_sha256=sha(Path(__file__)), control_sha256=fixture_id, generated=positive, selected=trials)
    (out / "report.json").write_text(json.dumps(report, indent=2)+"\n", encoding="utf-8")
    print(report["status"])


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("emulator", "qualification", "disk", "output"):
        parser.add_argument("--"+name, type=Path, required=True)
    parser.add_argument("--qualification-sha256", required=True)
    run(parser.parse_args())
