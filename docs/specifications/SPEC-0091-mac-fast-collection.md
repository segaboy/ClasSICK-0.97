<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0091 - Experimental Mac fast collection

An opt-in interrupt-driver experiment batches the existing SCC/VIA collection in one native callback with constant register selections. The normal profile is unchanged. This is a performance experiment, not a new mouse-decoding rule.

Read RR3 first. If it has any bits outside the two owned external/status bits, return immediately to the existing fault path. Otherwise read PB, RR0A, RR0B twice; acknowledge external status on each pending channel, then acknowledge IUS on A; read PB, RR0A, RR0B once more. Keep the existing access-spacing guards after every SCC access. The caller retains all existing entry checks, triple-stability tests, event validation, queue behavior, button safety, loss and recovery rules. Callback output is borrowed only for the call. The experiment cannot be combined with timer polling.

Host fixtures must exercise the unchanged core suite and compile the actual collector body against instrumented SCC/VIA accessors to check read/write order, conditional acknowledgements, early failure and refusal of a missing collector. The core and collector share the relevant SCC constants. Native compiler output and real target observations are separate checks. No claim of timing improvement, host mouse operation, physical Macintosh behavior or release readiness follows from the flag or a successful build.
