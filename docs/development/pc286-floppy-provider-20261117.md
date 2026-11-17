<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Native 286 floppy transport and MFS metadata integration

2026-11-17; SRC-0610/0612; SPEC-0111/0112/0113; IMPL-0143; TEST-0202.
The new native 16-bit provider reads a declared logical subrange from original
PC floppy media through BootHush's documented firmware. It supplies the shared
block service and the new MFS metadata reader with actual disk bytes, replacing
the RAM provider for this diagnostic. No released image is changed.

## Contract and implementation

`platform/pc286/boothush-floppy-read.[ch]` binds a positive subrange of the
modeled 2,400-sector, 80-cylinder/two-head/15-sector floppy. A caller-owned
512-byte bounce buffer is checked against the native segment, physical 64 KiB
boundary and conventional-RAM limit. One-sector reads map the logical block to
CHS, invoke the documented INT13 read and copy to staging only on carry-clear.
There are no writes, resets or retries. SPEC-0111 provides whole-output
atomicity across multiple reads. Stable DS and truthful disjoint native objects
remain preconditions; a stored-segment mismatch guard cannot validate arbitrary
rebinding of near pointers.

The original inline wrapper saves/restores DS, ES, SI, DI, BP and FLAGS,
captures returned carry before restoration, declares AX/BX/CX/DX clobbers and
returns with DF clear. This is the specified modeled boundary, not a general
physical BIOS guarantee. Runtime checks directly cover DS, enabled-entry IF
preservation and DF clearing; they do not independently seed every saved register.

The reviewed public BootHush disk-interface document has SHA-256
`032a3b4e4bea065531199418ac5e689a4f422573f6b0a4dcfc7e29f9407934f2`.
Only its documented interface was used; no firmware/emulator implementation,
protected disk, manual sample or third-party bootloader was imported.

## Executed checks

Six native runs pass on BootHush commit
`5d613614dd4f17d605de75ecc3f38c9129079488`, executable SHA-256
`7adb8a74cef009b7c532df24458c3a5db6b77725356fdb31e5ac4966d7ed971a`.
Every run verifies the entire loaded payload, unchanged original floppy and
source identities, expected stop/POST and no runtime-library modules.

| Case | POST | Payload bytes | Modeled cycles |
| --- | --- | ---: | ---: |
| Sector contents, CHS transitions and bounds | B8 | 1674 | 856466 |
| Second-sector firmware failure, output preserved | B9 | 1754 | 593673 |
| Deliberately wrong head mapping rejected | E2 | 1676 | 240803 |
| Deliberately ignored carry rejected | E4 | 1750 | 568743 |
| Actual-media MFS header and known entry | BA | 2406 | 197345 |
| MFS header firmware failure, output preserved | BB | 1915 | 108723 |

Execution uses real native DS=1000h. Numeric buffer tests cover exact FE00h,
physical 64 KiB crossing and conventional-RAM limits without dereferencing
fabricated pointers. Both injected faults occur exactly once. The provider-only
diagnostics use 2578 BSS bytes; the MFS integration uses 1930. These are linked
diagnostic sizes, not whole-system memory/stack budgets or physical timings.

The final report, full commands, source/tool/artifact identities and retained
output are in `C:/Repos/.task-temp/pc286-floppy-provider-20261117-d/`.
The original provider-only results remain in the earlier `-b` directory.
An initial harness run exposed a same-module OMF EXTDEF/PUBDEF accounting
error; the verifier now subtracts defined symbols. No provider code correction
was needed. Earlier reports are retained rather than replaced.

Reproduce with pinned Watcom tools and the current active-profile executable:

```powershell
python -B scripts/Test-PC286FloppyRead.py --tool-root WATCOM_ROOT --emulator BOOTHUSH_EXE --model-contract ../boothush/docs/pc-at.md --mfs-integration --output NEW_DIRECTORY
```

Use the report's exact argument vector for the complete six-case run; generated
output must go to a fresh directory outside the implementation checkout.

## Original metadata integration

The diagnostic declares physical sectors 60 through 187 as its 128-block
logical device. Header LBA 2 maps to physical 62 / CHS 2:0:3. Known-directory
LBA 4 maps to physical 64 / CHS 2:0:5. The caller-selected entry is at byte
offset 20, even-rounded extent 60. The directory description occupies relative
blocks 4-5, outside allocation storage beginning at block 6.

The guest verifies asymmetric timestamps and file number, all attribute bits,
version distinct from Finder information, every opaque Finder byte, counted
names containing zero/high bytes, zeroed unused name tails and canaries.
Both forks are described as absent, with zero starts and lengths. The fixture
intentionally contains raw metadata anomalies (including a clump value not a
multiple of allocation size); successful extraction is not validation of those
values as a complete filesystem. Allocation maps and record enumeration are
not consumed. A header read fault leaves the entire guarded volume output and,
for this no-transfer fault, workspace unchanged.

The MFS-success payload SHA-256 is
`23922cdcef542d304efef13533c397e9b37ef8fb7794a6d507174d044eff4ce7`;
its generated floppy SHA-256 is
`e48484d8938a149e271d8376fd20a491371bfb18f2abd629c9de191215d2dfbd`.
All other artifact hashes are retained in the report.

## Review and remaining limits

The lead reviewed the source and contract before trials. A separate bounded
Codex reviewer checked C16 arithmetic, address admission, publication, assembly
boundary and the report's 13 current source identities; no additional core or
provider defect was found. That reviewer did not execute tests. Worker inputs
were reviewed public/original files only; private HQ, protected media, held UART
and excluded research drafts were not supplied. Tool/provider resource usage
was not measured, and no efficiency claim is made.

This establishes a modeled native transport plus bounded metadata integration.
Mac GCR handling, physical BIOS behavior, directory enumeration, allocation-map
and fork validation, File Manager services, 286 legacy-code integration and
System/Finder boot remain open. No owner-selected disk was used. Human provenance
and physical validation remain separate. See the
[current milestone](system-disk-boot-20261117.md).
