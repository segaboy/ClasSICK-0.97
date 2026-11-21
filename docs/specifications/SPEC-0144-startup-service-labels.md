<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0144 - Named 68000 service-entry observations

A diagnostic build of the Mac boot path identifies the first admitted service
call that reaches the Line-A vector. Its only outcomes are one named terminal,
`unmatched` for an unlisted entry, or `gateway-not-entered` when the gateway
was never reached. This build adds no functional ROM service.

Use the stacked exception PC for the current call. The Motorola 68000
exception frame and Line-A behavior are covered by SRC-0069 and SPEC-0021.
Apple's published *Inside Macintosh*, 1985, Volume I, printed I-88–89, gives
the two call formats. The OS service number occupies bits 0–7 and the Toolbox
number bits 0–8. The calling convention selects the format with bit 11. OS
flags and the Toolbox auto-pop bit do not change the number. This diagnostic
declines Toolbox words with their reserved bit set. The original directory
can share numbers between formats. The diagnostic identifies only calls made
in the published format for the named service. [Published manual](https://puyoman.net/documentation/Inside_Macintosh.pdf)

The finite [catalog](../../probes/mac128k/service-labels.def) includes public
file, memory, event, manager initialization, dispatch and drawing interfaces
from the same edition, Volume III, Appendix C, pp. III-215–226. The format and
service identities were fixed before a selected disk was tried. Existing
SPEC-0020, SPEC-0025, SPEC-0029, SPEC-0036 and SPEC-0136 supply independent
project context for covered interfaces. These names describe published calls.
Availability and precise behavior in January 1984 require separate evidence.

Use the normal disk-loading code and overlay sequence. On successful load the
diagnostic selects its vector before transfer to the disk. Each catalog entry
has one terminal ROM address. Split the observation into two deterministic
cold runs: establish entry into the gateway, then check the terminal against
the linked symbol set. A direct visit to a terminal without gateway entry
cannot produce a named result. Read-only selected-media output consists of
the allowlisted labels and artifact identities. Admit only the exact generated
qualification report, matching input set, ROM twins and emulator.

Qualify all names on newly generated disks and independently exercise aliases,
unknown OS and Toolbox calls, a reserved bit, misleading adjacent values and
a run that never calls the gateway. The developer ROM and 286 installer have
separate source lists and qualified identities.

## Optional observation of unhandled services

With `--unhandled-only`, retain the product Line-A vector, loader, dispatch
directory and service returns. Wrap only the call to `cs_mac_trap_target`.
Forward its original argument and successful return value. A zero result
from the Line-A dispatcher's lookup reaches the diagnostic gateway, which
classifies the service argument using the same finite catalog and terminals.
Direct calls from guest memory to the helper are outside this observation.
An absent gateway at the bounded horizon or a prior modeled CPU stop yields
`no-unhandled-service-observed`. A service admitted by the resolver can
later report its own error without producing a classification.

Mode-specific generated controls and the selected trial must bind the same
mode, executable twins, source identities and emulator. Qualify the default
first-call mode separately. [Unhandled service trial](../development/startup-unhandled-service-20261121.md).
