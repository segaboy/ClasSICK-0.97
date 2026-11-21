#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Twin build, install and cold-boot the disk-startup profile using generated sectors."""
import argparse
from dataclasses import dataclass
from importlib import import_module
import json
from pathlib import Path
import struct
import subprocess

from m68k_windows_toolchain import sha
from pc286_hard_disk_loader import DISK_BYTES

builder = import_module("Build-DiskStartup")
EMULATOR = "543767ae09f39d330803c05bd34311b3b3f485e53f00ec814c54c82d9c8a329f"


def fixture(value, trap=False):
    image = bytearray(409600)
    struct.pack_into(">HHh", image, 0, 0x4c4b, 0x6000, 510-4)
    # Run over 256 steps and copy a small marker-writing routine beyond the
    # loaded blocks. Only disk code can set the marker on either target.
    program = bytearray.fromhex("303c012c 51c8fffe")
    if trap:
        program += bytes.fromhex("7046 a146 23c800009104")  # Save GetTrapAddress(0x46).
    program += struct.pack(">HII", 0x23fc, 0x13fc0000 | value, 0x8800)
    program += struct.pack(">HII", 0x23fc, 0x00009100, 0x8804)
    program += bytes.fromhex("33fc60fe00008808 4ef900008800")
    image[510:510+len(program)] = program
    return image


def patched_fixture():
    image = fixture(0)
    # Install an original handler at $8300, read its address back, invoke it,
    # save its result and write the completion marker only after its return.
    main = bytes.fromhex("7017 207c00008300 a047 207c00000000 a146 23c800009104 "
                         "a017 23c000009100 13fc00ac00009108 60fe")
    image[510:510+len(main)] = main
    image[0x300:0x308] = bytes.fromhex("203c43535031 4e75")
    return image


def program_disk(program):
    if not 0 < len(program) <= 1024-510:
        raise ValueError("generated code must fit the two startup sectors")
    image = bytearray(409600)
    struct.pack_into(">HHh", image, 0, 0x4c4b, 0x6000, 510-4)
    image[510:510+len(program)] = program
    return image


def new_ptr_disks(m68k_bin, out):
    tools, _ = builder.inspect(m68k_bin)
    source = builder.ROOT/"tests/startup-new-ptr.S"
    obj = out/"new-ptr-fixtures.o"
    subprocess.run([tools["gcc"], "-m68000", "-c", source, "-o", obj], check=True, timeout=60)
    disks = {}
    for section in ("allocations", "limits", "patching"):
        raw = out/f"new-ptr-{section}.bin"
        subprocess.run([tools["objcopy"], "-j", "."+section, "-O", "binary", obj, raw], check=True, timeout=60)
        disks[section] = program_disk(raw.read_bytes())
    return disks


def immediate_disk():
    # A conditional branch over ILLEGAL makes a wrong condition observable.
    # First exercise signed overflow, carry and equality on byte data in D0.
    program = bytes.fromhex("707f 0600 0001 6b02 4afc 6902 4afc 6402 4afc "
                            "0c00 0080 6702 4afc 0a00 0055 0000 0002 0200 000f 0400 0001")
    # Set all five CCR bits, retain only carry, then invert carry to zero.
    program += bytes.fromhex("003c 001f 6502 4afc 023c 0001 6a02 4afc 0a3c 0001 6402 4afc "
                             "23c0 00009100")
    # SUBI.W, ORI.L, EORI.W (displacement), ADDI.L, then CMPI.L (postincrement).
    # The following marker uses the advanced A0, independently checking writeback.
    program += bytes.fromhex("207c 00009104 20bc 00010000 0450 0001 0090 10203040 "
                             "0a68 3045 0002 0690 01000000 0c98 11200005 "
                             "6702 4afc 10bc 00ac 60fe")
    return program_disk(program)


def quick_disk():
    # ADDQ.B $ff+1 sets C/Z. ADDQ.W on An preserves both. The subsequent
    # SUBQ.W, TST.W and CLR.B must preserve D0's unselected high bits.
    program = bytes.fromhex("70ff 5200 6502 4afc 6702 4afc 5248 6502 4afc 6702 4afc "
                            "5140 4a40 6b02 4afc 4200 6702 4afc 23c0 00009100")
    # Exercise quick long, displaced byte clear and two consecutive word
    # postincrements. The final SUBQ borrows. A0 must reach the marker byte.
    program += bytes.fromhex("207c 00009104 20bc 80000001 5090 4228 0003 4a58 "
                             "6b02 4afc 5158 6502 4afc 10bc 00ac 60fe")
    return program_disk(program)


def direct_trap_disk():
    # Save the actual ROM Get/Set addresses in A2/A3, call both with JSR,
    # then query the installed $8300 address using the original Get entry.
    return program_disk(bytes.fromhex("7046 a146 2448 7047 4e92 23c8 00009100 "
                                      "2648 7017 207c 00008300 4e93 207c 00000000 "
                                      "4e92 23c8 00009104 13fc 00ac 00009108 60fe"))


def refusal_disk(instruction):
    # The first store proves guest entry. A fallthrough would overwrite it.
    return program_disk(bytes.fromhex("13fc 005a 00009107 " + instruction +
                                      " 13fc 00ee 00009107 60fe"))


@dataclass
class ColdCase:
    name: str
    disk: bytearray
    expected: bytes
    marker: int = 0xAC
    marker_offset: int = 8
    failure_post: int = 0


def cold_cases(pc, mac, allocation_disks):
    for name, value in (("first", 0x37), ("alternate", 0x82), ("retry", 0x37)):
        yield ColdCase(name, fixture(value), bytes([value]) + bytes(7), value, 0)
    bad = fixture(0x37)
    bad[0] ^= 1
    yield ColdCase("bad-header", bad, bytes(8), failure_post=0xd0)
    yield ColdCase("read-error", fixture(0x37), bytes(8), failure_post=0xd0)
    get = 0x400000 if pc else mac["get_trap_address_pc"]
    yield ColdCase("get-trap", fixture(0x54, True), struct.pack(">II", 0x54000000, get), 0x54, 0)
    yield ColdCase("patched-trap", patched_fixture(), bytes.fromhex("4353503100008300"))
    yield ColdCase("immediate", immediate_disk(), bytes.fromhex("0000000611200005"))
    yield ColdCase("quick-unary", quick_disk(), bytes.fromhex("fffffe008000fff8"))
    # Compare the caller's actual pointers, guest writes and preserved register
    # results. The completion marker follows the last result in each disk.
    new_ptr = 0x400004 if pc else mac["new_ptr_pc"]
    allocations = struct.pack(">III", new_ptr, 0x10080, 0x10086) + bytes.fromhex(
        "00000055 11223344 0000a080 0000a088 00000000 00c70000 00000000 00007800")
    limits = bytes.fromhex("ffffff94 00000000 ff94 1234ff94 00000000 ff94 "
                           "00010080 00000000 0000 00010082 ba0000ce ee00 0000ff94 00000000 ff94 "
                           "0000a080 0000 0000a082")
    patching = bytes.fromhex("00009400 00003456 00010080 00000000")
    for name, expected in (("allocations", allocations), ("limits", limits), ("patching", patching)):
        yield ColdCase("new-ptr-"+name, allocation_disks[name], expected, marker_offset=len(expected))
    for name, instruction in (("uninstalled-os", "a017"), ("toolbox", "a800"), ("reserved-tst", "4a48")):
        yield ColdCase(name, refusal_disk(instruction), bytes(7)+b"\x5a", failure_post=0xd2)
    if pc:
        # The installed decoder deliberately declines immediate writes to SR.
        yield ColdCase("refused-sr", refusal_disk("007c 0000"), bytes(7)+b"\x5a", failure_post=0xd2)
    else:
        yield ColdCase("direct-trap", direct_trap_disk(), struct.pack(">II", mac["set_trap_address_pc"], 0x8300))


def verify(args):
    out = args.output.resolve()
    if out == builder.ROOT or builder.ROOT in out.parents or sha(args.emulator) != EMULATOR:
        raise ValueError("need external output and qualified emulator")
    out.mkdir(parents=True, exist_ok=False)
    builds = [builder.build(args.m68k_bin, args.tool_root, args.assembler, out/name) for name in ("a", "b")]
    record = builds[0]
    bundle = out/"a"
    members = {builder.ROM, builder.INSTALLER, "manifest.json"}
    if (builds[0] != builds[1] or sha(bundle/"manifest.json") != sha(out/"b/manifest.json") or
            any({f.name for f in (out/d).iterdir()} != members for d in ("a", "b"))):
        raise ValueError("developer packages differ")
    for name in record["artifacts"]:
        if sha(bundle/name) != sha(out/"b"/name) or sha(bundle/name) != record["artifacts"][name]:
            raise ValueError("artifact identity mismatch")
    rom, install = bundle/builder.ROM, bundle/builder.INSTALLER
    blank, installed = out/"blank.img", out/"installed.img"
    blank.write_bytes(bytes(DISK_BYTES))
    blank_sha = sha(blank)
    def guest(lane, options, *, cycles):
        cmd = [str(args.emulator), "--machine", lane, "--timeout-ms", "40000", "--cycles", str(cycles), *map(str, options)]
        proc = subprocess.run(cmd, text=True, capture_output=True, timeout=48,
                              creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        row = json.loads(proc.stdout)
        if proc.returncode or row.get("status") != "condition_met":
            raise ValueError(f"{lane} fixture missed stop ({proc.returncode}) {row}")
        return row
    keys = []
    for i, key in enumerate((0x43, 0x31, 0x1b, 0x2c, 0x1c, 0x4b, 0x4b, 0x5a)):
        for delta, value in ((0, key), (10000, 0xf0), (20000, key)):
            keys.extend(["--scancode", f"{10000000+i*100000+delta}:{value}"])
    guest("pc286", ["--floppy", install, "--hard-disk", blank, "--disk-out", installed,
                    "--until-post", "0xac", *keys], cycles=600000000)
    if sha(blank) != blank_sha or sha(installed) != record["pc"]["installed_disk_sha256"]:
        raise ValueError("installer output differs from expected entire disk")
    observations = []
    fixture_source = builder.ROOT/"tests/startup-new-ptr.S"
    fixture_hash = sha(fixture_source)
    allocation_disks = new_ptr_disks(args.m68k_bin, out)
    for lane in ("mac128k", "pc286"):
        pc = lane == "pc286"
        for case in cold_cases(pc, record["mac"], allocation_disks):
            name, logical = case.name, case.disk
            source = out/f"{name}-{lane}.img"
            source.write_bytes(logical.ljust(1228800, b"\0") if pc else logical)
            before = sha(source)
            opts = ["--boot-drive", "hard-disk", "--hard-disk", installed] if pc else ["--rom", rom]
            opts.extend(["--floppy", source])
            marker = 0x39100 if pc else 0x9100
            if name == "retry":
                opts += (["--fail-hard-disk-read-once", "0:0:2"] if pc else ["--mac-floppy-bad-checksum-once", "0:0"])
            if name == "read-error":
                opts += (["--fail-floppy-read", "0:0:2"] if pc else ["--mac-floppy-bad-checksum", "0:0"])
            if case.failure_post:
                opts += (["--until-post", case.failure_post] if pc else ["--until-pc", record["mac"]["startup_stop_pc"]])
            else:
                opts += ["--until", f"{marker+case.marker_offset}:1:{case.marker}"]
            opts += ["--inspect", f"{marker}:{len(case.expected)}"]
            row = guest(lane, opts, cycles=250000000 if pc else 16000000)
            view = row["memory"]
            if case.failure_post:
                observed, wanted = ((row.get("post"), case.failure_post) if pc else
                                    (row["pc"], record["mac"]["startup_stop_pc"]))
                if observed != wanted:
                    raise ValueError(f"{lane} {name} unexpected terminal: {observed}")
            if view["address"] != marker or bytes.fromhex(view["bytes"]) != case.expected or sha(source) != before:
                raise ValueError(f"{lane} {name} guest result or source differs: {view}")
            if name in ("retry", "read-error"):
                if pc:
                    fault = row["hard_disk_read_failure" if name == "retry" else "floppy_read_failure"]
                    if fault["injected"] != 1:
                        raise ValueError("expected one PC firmware injection")
                elif row["mac_floppy_checksum_fault"]["altered_nibbles"] != (1 if name == "retry" else 2):
                    raise ValueError("Mac did not observe expected checksum fault count")
                if name == "retry" and row["cycles"] <= next(o["cycles"] for o in observations
                                                             if o["case"] == "first" and o["lane"] == lane):
                    raise ValueError("injection did not increase the cold-start cycle count")
            observations.append(dict(case=name, lane=lane, cycles=row["cycles"], pc=row["pc"],
                                     marker=view["bytes"], source_sha256=before))
    if (sha(installed) != record["pc"]["installed_disk_sha256"] or sha(args.emulator) != EMULATOR or
            sha(fixture_source) != fixture_hash or
            any(sha(bundle/n) != h for n, h in record["artifacts"].items())):
        raise ValueError("tested runtime or emulator changed")
    report = dict(status="generated-controls-pass", runner_sha256=sha(Path(__file__)), fixture_sha256=fixture_hash,
                  manifest_sha256=sha(bundle/"manifest.json"),
                  installed_sha256=sha(installed), emulator_sha256=EMULATOR, observations=observations)
    (out/"report.json").write_text(json.dumps(report, indent=2)+"\n", encoding="utf-8")
    print(report["status"])


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("m68k-bin", "tool-root", "assembler", "emulator", "output"):
        parser.add_argument("--"+name, type=Path, required=True)
    opts = parser.parse_args()
    for name in ("m68k_bin", "tool_root", "assembler", "emulator"):
        setattr(opts, name, getattr(opts, name).resolve(strict=True))
    verify(opts)
