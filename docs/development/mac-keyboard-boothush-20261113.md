<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Headless Mac keyboard transport and release status

Owner authorized building the Mac keyboard driver and connecting it to the ROM;
headless BootHush remains the only authorized emulator. SPEC-0100 / IMPL-0120 /
TEST-0167 add an **opt-in experimental** transport to the native-window ROM.
The driver is original project code using the published/model interface facts
identified in SRC-0071, SRC-0602 and SRC-0606; it does not incorporate game or
emulator implementation. Earlier SPEC-0099 parser evidence is unchanged.

Foreground code owns the VIA shift register, polls command and reply completion,
decodes Instant-command transitions and retains an eight-event diagnostic ring
and current key-position bitmap. Polling is nonblocking. Each transfer phase
times out after eight advancing VBL ticks; a timeout latches until reboot.
This delivers transitions to the diagnostic consumer, not to an Event Manager
or an application. It does not yet translate characters, recover after unplug,
prove rollover, or establish physical timing. A stopped/saturated VBL counter
cannot advance the timeout. See the contract for ownership and limits.

## Actual checks

The original 68000 cross-build produced two byte-identical 65,536-byte ROMs,
SHA-256 `b3a50ad806b589fdc02a4de579e6d72700c9107fb273d9ca73026320bebed807`.
Build manifest and compiler output are under
`C:/ClasSICK/mac-keyboard-20261113-b`. The source inputs are hashed in that
manifest; the base checkout was `87158a9` and the keyboard changes were in the
working tree during the build. The ROM is an experimental candidate, not a
published release. Its image uses 48,344 ROM bytes and 552 bytes of own native
data/BSS in this profile; the build's conservative native stack bound is 5,180
bytes. Full 128K acceptance remains separate.

Windows Clang hosted checks passed for the new transport and SPEC-0099 parser;
the transport check also passed with AddressSanitizer and UndefinedBehaviorSanitizer.
The transport test checks four replies, empty and invalid replies, preserving
unrelated ACR bits, a stuck outgoing transfer, the latched failure and an
incoming timeout across counter wrap. The hosted test uses a fake VIA; it is
not hardware validation.

BootHush checkout `cf3c1777c8bb7704fc11bf89557dbab684019179`, executable
SHA-256 `7b4c32e24d63432b2103f38c9c1844480a64b1b2bdda4342896e24a19d7c98c7`,
ran the candidate headlessly. An initial bounded keyboard witness accepted and
delivered `01 81 03 83` with zero trace drops or rejected inputs, and the ROM
recorded positions 0 and 1 pressed and released in order. No keyboard timeout
or invalid reply occurred.

The repeatable [integration harness](../../scripts/Test-BootHushMacKeyboard.py)
ran mouse-only, mixed keyboard/mouse, and overlapping-key scenarios. In each,
the mouse sequence contained two drags, releases and a content click; the
result was pointer `(150,110)`, window `(112,46)`, three presses, pattern 1,
no held button, no reported hard loss/omission and no guest fault. Four key
transitions reached the KBD1 record in each keyed case, including overlapping
holds, and both witnesses had zero dropped/rejected rows. Final framebuffers
matched mouse-only output. This is a bounded modeled input sequence, not a
general concurrency or physical-device result. Full reports and witnesses are
in `C:/ClasSICK/mac-keyboard-20261113-validation-b`.

The mixed run recorded 69 BootHush guest-only video frames. An independently
converted GIF is at `C:/ClasSICK/mac-keyboard-20261113-a/mixed2.gif`, SHA-256
`dad98ce64c332ab8dcc859c42f1327d3271810a6513269ad0ae59389b647bfc5`;
it uses the same ROM and sequence. The final framebuffer is pixel-identical
to the previously indexed [headless drag](../evidence/screenshots/2026-11-12/boothush-headless-drag.png).
No new screenshot is needed to illustrate the same pixels; a still cannot prove
keyboard delivery. GIF is kept outside Git under the existing media policy.

The same candidate's independent mouse-speed matrix passed all four 4,096-cycle
directions but failed all four at 2,048 cycles: 29 omitted passes and a one-pixel
opposite movement for each modeled 32-pixel request. An 8,192-cycle run without
VBL guard also missed one pixel. The checked result is at
`C:/ClasSICK/mac-keyboard-20261113-a/mouse-probe.json`. This repeats the existing
model-specific blocker; no physical rate or fault attribution is inferred.

## BootHush additions that would help

1. Add opt-in, deterministic keyboard disconnection/stalled-command and
   delayed-reply fault injection, with explicit guest-cycle application and
   witness outcomes. Exercise the command and reply phases independently and
   verify normal runs are unchanged when disabled. This would allow target tests
   of our timeout and recovery policy; the current checks are hosted fake-VIA
   checks. Also support explicitly scripted malformed reply bytes without
   presenting them as actual M0110 behavior.
2. Continue the earlier independent SCC interrupt, phase and timing review for
   the 2,048-cycle omission. Keep ordered RAM/interrupt witnesses, range and
   dropped-row reporting and traced/untraced noninterference checks. Do not
   tune the model just to make the ClasSICK guest pass.
3. Supply an actual headless x64 CPU and UEFI machine capable of booting our
   EFI image, GOP, timer, PS/2 input and guest diagnostic inspection as detailed
   in the [prior developer report](boothush-developer-report-20261113-c.md).
   BootHush `--x64-capability` in this session returned `0x80070005`,
   `implemented:false`, `guest_executed:false`; it was not a boot.

Mac fast input, physical/human evidence, x64 headless boot and paired release
qualification are open. No release was published, no other emulator or desktop
test was used, and no interoperability claim is made.
