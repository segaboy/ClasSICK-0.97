<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# 128K RAM aliases on the installed 286 memory path

The optional post-overlay scalar adapter presents 32 copies of one 128 KiB
region below 0x400000. The published description of the original 128K/512K
address map is in Apple, *Inside Macintosh*, 1985, volume III, printed
III-17 and III-45. See the [published volume](https://puyoman.net/documentation/Inside_Macintosh.pdf)
and [SPEC-0139](../specifications/SPEC-0139-mac128k-ram-aperture.md).

On BootHush a native USE16/80286 fixture wrote 0xDECAFBAD at 0x01FFF0 and
read it at the equivalent offset in all 32 windows, including 0x03FFF0.
Each window wrote a longword straddling a physical 32 KiB bank boundary.
A write at 0x03FFFE stored its two words at physical RAM offsets 0x1FFFE
and zero. A distinct alias read both back. A write at the last word of
the full 4 MiB aperture refused and preserved the previous value. The
independent upper read-only region retained its address and permissions.
The fixture also checked generic backend behavior, malformed map binding,
stale lifetime and a read-only canonical region.

BootHush inspected every loaded payload byte and five physical locations
after the fixture reached POST B8. The external reads observed F1 23 at
0x30000, 12 34 at 0x37FFE, 00 1F at 0x3A000, DE CA FB AD at 0x55FF0 and
AB CD at 0x55FFE. These are physical PC addresses for the four scattered
RAM banks chosen by this test. The loaded 5,012-byte fixture took 3,465,033
modeled cycles including startup. This is component evidence on an original
fixture; installed startup uses its own separate evidence.

The report is `C:/Repos/.task-temp/ram-aperture-20261119-a/report.json`,
SHA-256 `3306eed6e994ce9596b75eafb5c43cd8e38e82821ef6df49d7ac2193f52a7256`.
It includes the source, payload, floppy and BootHush identities, native
compiler commands and the observed machine state. Reproduce from the
repository using a fresh external output directory:

```powershell
& C:/Users/segab/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe -B scripts/Test-PC286Mac128kRam.py --tool-root C:/ClasSICK/pc286-toolchain-20261023-a/toolchain --emulator C:/Repos/.task-temp/boothush-floppy-writes-20261118-b/a/boothush-active.exe --output C:/Repos/.task-temp/ram-aperture-fresh
```

The subsequent installed-runtime transfer profile (SPEC-0141) overlays
a 1–64 byte atomic write on the same scalar map. With `--transfer` and
a new output directory, the fixture writes every length beginning at the
final byte of an interior RAM window, checks the final byte via its physical
address, and checks read-only and outer-aperture refusal before publication.
The original scalar checks and all five independent physical inspections
also passed with this profile. Report
`C:/Repos/.task-temp/ram-transfer-20261119-a/report.json`, SHA-256
`3a3c64f7825dc2c8b06b4f691cb2593bd84d27a5b0a59c1fd4baea1fb08559f5`,
records 5,706 loaded payload bytes and 10,610,376 modeled cycles.
