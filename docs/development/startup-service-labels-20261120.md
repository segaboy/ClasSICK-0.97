<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Macintosh ROM service-entry diagnostic

The 64 KiB diagnostic ROM uses the developer ROM's reset and disk-loading
sources. A diagnostic-only wrapper installs its vector after the two startup
sectors load successfully. At hardware Line-A entry it routes the public
service key to an owned terminal. The running build includes 52 names drawn
from published OS and Toolbox interfaces. No functional routine is invoked.
See [SPEC-0144](../specifications/SPEC-0144-startup-service-labels.md) and
the [catalog](../../probes/mac128k/service-labels.def).

For each generated disk, the first cold run must reach the linked vector
entry. A second cold run ends at an immutable named ROM address or the
common unmatched terminal. The host returns the label alone. The ROM does not
store a diagnostic marker in guest RAM. The normal Mac ROM and 286 installer
are separate artifacts built from their own source lists.

## Generated qualification on 2026-11-20

Sixty independently generated disks passed in BootHush. Each of the 52
catalog entries was exercised, along with OS flag aliases, Toolbox auto-pop,
separate unknown OS and Toolbox entries and a reserved Toolbox flag. A
misleading previous operand, stale D0 and following service still produced
the current PBRead label. A loop with a recognizable word in D0 and an
explicit jump to the InitUtil terminal both reported `gateway-not-entered`.

Two independent links produced byte-identical ROMs with 5,380 linked bytes.
The existing linker warning about a missing GNU-stack note also appeared on
both diagnostic links. The report binds the 24 current source, header and
support-script inputs and compiler hashes to both ROMs and the per-control
disk identities. Negative checks rejected an incorrect qualification hash and
an unadmitted generated disk before output creation.

| Artifact | SHA-256 |
| --- | --- |
| Diagnostic ROM | `657f1875598dcb41ad7d5a7d2c206166aa3de16849b8bfbd265c7e7581227173` |
| BootHush | `a00fb8b507d79b0f0b88026c2e9cd613a6a7bd78c3aae10858bb5735e4f6db98` |
| Generated qualification report | `2cc304910948363f5b40d57435dc9dfdc1dca0611c06e4f10636b0322e2f74d2` |

Reproduce on Windows from the checkout with the pinned compiler tools and
BootHush, using fresh external output directories.

```powershell
python -B scripts/Test-StartupServiceLabels.py qualify --m68k-bin $M68kBin --emulator $BootHush --output $Generated
```

For the separately admitted selected-disk trial, the runner first verifies the
exact external DC42 hash, ROMs, emulator, generated report, labels and source
identities. It performs three independent cold comparisons. The selected
report holds only owned label outcomes and artifact hashes.

```powershell
python -B scripts/Test-StartupServiceLabels.py selected --qualification $Generated --qualification-sha256 $ReportHash --emulator $BootHush --disk $SelectedDC42 --output $Trial
```

## Selected System Disk

All three cold repetitions with the admitted 690-5003-C System Disk reached
the owned **NewPtr** terminal. The supplied container retained SHA-256
`1e03b75a05bbf441ae4e1808f70e38ffe0a07e3cc3298de151c0d2775552c647`.
The selected report's SHA-256 is
`cbf2c8f89bb738ae51c3da9db3bdae9e03bb1c4721c884e4b59d630f7a2d41a7`.
This identifies the first missing Mac service observed by this diagnostic.
The terminal stops before allocation. The requested size, flags and caller
state were not observed. The separate [developer ROM and installed 286
trial](installed-disk-startup-20261119.md) retains its own results.
