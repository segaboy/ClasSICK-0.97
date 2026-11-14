<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0104 - Experimental 286 keyboard configuration query

2026-11-14. This original software contract supplements SPEC-0103; it is not
a physical controller specification or a production input driver. The caller
owns a quiescent controller and masks IRQ1 while performing the query.

A bounded state machine requests a status sample, then command `20h` to port
`64h`, then one response from port `60h`. It refuses pre-existing output
without reading it; it refuses status errors or auxiliary output, and bounds
the number of polls separately before command and before response. It accepts
the response only when bit 6 is set and bit 4 clear. The query does not change
the configuration. Terminal outcomes remain stable and invalid calls refuse.
A concurrent input byte can be indistinguishable from the query response;
masking IRQ1 alone does not make the controller quiescent.

The status and command conventions are applied consistently with the
project's existing PS/2 research and independently authored driver. The
BootHush model's response is an emulator observation, not a hardware fact.
Timeout recovery, concurrency, controller initialization, exit restoration
and physical qualification remain separate work. See the [evidence record](../development/mac-pc286-shared-parity-20261114.md).
