<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# First unhandled service during Macintosh disk startup

**PBMountVol was the first service refused in all three cold observations
with the supplied System Disk.** The classifier stopped at the owned
PBMountVol terminal. The parameter block, drive selection, mount result
and subsequent System/Finder execution were not observed. The original
DC42 hash matched before and after the trials.

The [SPEC-0144 diagnostic](../specifications/SPEC-0144-startup-service-labels.md)
has a second mode for identifying the first service the developer ROM's
resolver declines. The default mode identified NewPtr as the disk's first
request on [November 20](startup-service-labels-20261120.md). An accepted
lookup now runs the real ROM routine or guest-installed patch before the
diagnostic observes any subsequent refusal.

The opt-in link wraps `cs_mac_trap_target`. A zero result from a call made
inside the linked Line-A dispatcher enters the existing catalog gateway.
The two deterministic cold runs per observation establish gateway entry
and then identify its terminal. Direct calls from guest RAM to the helper
preserve its ordinary return value. The wrapper adds a four-byte argument and
a four-byte return address while the resolver runs, and the diagnostic has
its own linked ROM addresses. The 12-million-cycle bound applies per
run. A fault, an independent terminal or a loop before a declined lookup
can yield `no-unhandled-service-observed`. This mode cannot identify a
failure inside an admitted service, distinguish its parameters or establish
that startup completed.

Use `--unhandled-only` for both the generated qualification and selected
trial. The [runner](../../scripts/Test-StartupServiceLabels.py) checks the
qualified mode, executable twins, sources, compiler, emulator and exact disk
identity. Selected-media receipts contain only named outcomes and identities.

The generated controls exercised allocation with a nonzero size, wrote and
read a byte through the returned pointer, and then requested PBRead or an
unlisted OS service. Another disk installed a RAM handler for PBRead, called
it, verified its return value, and reached the PBWrite refusal. Separate
controls exercised a Toolbox call, stale values, unknown calls, a run with
no service, a fault before dispatch, a direct helper call and a direct jump
to a terminal after a successful GetTrapAddress. Using the other mode's
generated receipt in either direction was rejected before trial output.

The final Mac sources passed all 12 opt-in controls and all 60 existing
default controls. Each mode produced two byte-identical links. The opt-in
diagnostic used 6,140 linked bytes in its 65,536-byte ROM; the default used
6,124. The linker reported its existing missing GNU-stack note warning.

| Artifact or receipt | SHA-256 |
| --- | --- |
| Supplied System Disk DC42 | `1e03b75a05bbf441ae4e1808f70e38ffe0a07e3cc3298de151c0d2775552c647` |
| BootHush executable | `a00fb8b507d79b0f0b88026c2e9cd613a6a7bd78c3aae10858bb5735e4f6db98` |
| Default first-service ROM | `34da4f56683a3d888870d87ef47dc6251706e40c601d404df3907fbfbe098798` |
| `startup-first-service-20261121-c/report.json` | `fb7222edf23be423656795e87aa21b3c5c15d64857d4889675539e3738e5f588` |
| Unhandled-service ROM | `b03ffe7e67cad08beb5b166b56e53e8c081b80a3255ef211df7dd446963e9027` |
| `startup-unhandled-service-20261121-c/report.json` | `1cd06a7c620ec3ccc458d51e6acb39ba1503033b5d3ddabc946be243001761e9` |
| `startup-unhandled-service-selected-20261121-a/report.json` | `9c158b9348ee636374057b89f17319465f2fe5bf0e8957dd93b34c4a79576134` |
