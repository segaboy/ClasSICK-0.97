<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0099 — experimental M0110 key reply parsing

This original, pure software profile interprets a **key-transition inquiry**
reply using the documented BootHush model and the independently observed
Not Super Tilt Bro `NST2` probe (SRC-0606). It is not evidence of physical
keyboard timing, rollover, January 1984 behavior or every keyboard variant.
Do not apply it to responses to model-number, self-test or other commands:
different commands can return the same byte with a different meaning.

`cs_m0110_reply` accepts a numeric byte and a writable event output. A null
output or input above 255 returns ARGUMENT. `0x7b` returns EMPTY without
changing output. Other even bytes and odd bytes whose position exceeds `0x39`
return INVALID without changing output. For other odd bytes, position is the
low seven bits divided by two and pressed is one when bit 7 is clear, zero
when it is set; EVENT publishes both fields. Unknown values are not treated as
key transitions. Argument checks precede all output. The function allocates
nothing, does no I/O and retains no state.

This is a narrow engineering parser, not an M0110 driver, key mapping, event
manager, state tracker or complete protocol. The caller owns the command and
reply stream, timing, rollover, errors, serialization and output lifetime.
Observed `01 81 03 83` press/release replies are a subset of the model profile,
not proof of its whole range. Wiring this parser into a ROM requires a separate
hardware-binding contract and target verification. No release gate is closed.
