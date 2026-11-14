<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0103 - Experimental 286 keyboard make/break mapping

2026-11-14; original software experiment under SPEC-0051 and the
[Mac/286 direction](../development/mac-286-paired-direction-20261114.md).
It does not admit a physical controller, replace the reviewed PS/2 driver, or
qualify a product keyboard path.

An initialized state tracks a logical Space-held bit and scan prefixes.
The caller chooses one stream format per session: set 2 (`29` Space, `6b`,
`74`, `75`, `72` arrows, `f0` break, `e0` extended), or translated set 1
(`39` Space, `4b`, `4d`, `48`, `50` arrows, high-bit break, `e0` extended).
Arrow make/repeat produces a 16-pixel sample; arrow breaks produce none.
Unextended Space make produces one press only when released, repeated makes
produce no sample, and a matching break produces one release. Extended Space
and unknown codes produce no sample. `e1` skips the next seven bytes for set 2
or five for set 1. Prefixes and skipped bytes do not emit samples. Unsupported
codes and formats require caller-level handling; this narrow decoder is not a
complete keyboard implementation.

Invalid state, null pointers or byte `00`, `ff`, `fc` or `fd` returns -1
without changing state or output; a caller must stop or cancel and recover.
Zero means no sample and one means a valid sample. State and output are valid,
nonoverlapping objects. Controller initialization, translation detection,
timeouts, event queue overflow, interrupt sharing and recovery belong to the
caller. A missing break or dropped byte cannot be inferred by this decoder.
Physical key repeat and rollover remain unqualified.

An opt-in diagnostic disables interrupts after BIOS video setup, polls the
controller output and assumes translated set 1, matching the observed built-in
BootHush firmware configuration. It halts on reported status or decoder
errors. It cannot safely serve as a full OS input driver: it does not configure
or verify the controller, handle other interrupt sources, schedule fairly, or
recover. [Evidence](../development/mac-pc286-shared-parity-20261114.md).
