<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0100 — experimental Mac keyboard transport

An opt-in native-window profile gives the foreground exclusive ownership of
the VIA shift register and ACR shift-mode bits. VIA shift interrupts remain
disabled; the existing VBL and mouse interrupt paths retain their ownership.
Other ACR bits are preserved. The foreground issues Instant (`0x14`), waits
for shift completion, acknowledges it by reading SR, switches to external
shift-in, and reads one completed reply. No call spins or waits. A phase not
completed within eight advancing VBL ticks latches a timeout and disables the
shift mode; recovery requires a fresh boot. This is an engineering timeout,
not a physical timing claim. A stopped or saturated VBL counter cannot time
out; the rest of the foreground still runs. The result is decoded only in
the Instant-reply context of SPEC-0099. Invalid replies are counted.

Each decoded transition is delivered to the foreground's `KBD1` diagnostic:
event, invalid and timeout counts; two position-state bitmap words; and a
ring of the latest eight position/pressed pairs. It is not an Event Manager,
character translator, application keyboard API, nor a lossless event queue.
The ring overwrites older entries, with total count retained until saturation;
after saturation the last ring slot alone is overwritten.
This bounded model does not claim startup/reconnection handling, electrical
timing, keypad support, rollover, or physical Mac compatibility.

Interface facts: admitted published *Inside Macintosh* (1985), SRC-0071,
III-29–31 and III-41–42, and the documented BootHush M0110/VIA model.
BootHush's model is an experimental test target, not historical proof.
Physical behavior, packet loss and concurrent sustained mouse traffic remain
release prerequisites, not conclusions of this contract.
