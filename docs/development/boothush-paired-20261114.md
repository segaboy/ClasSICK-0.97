<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# BootHush integration: Mac faults, 286 loader, and remaining work

Date: 2026-11-14. Owner directed headless BootHush-only ClasSICK work, intake
of Not Super Tilt Bro evidence, 286 advancement, and an emulator improvement
report. This file is the downloadable report and evidence receipt. It does not
authorize or claim a release, physical test, or historical compatibility.

## Reviewed inputs

BootHush checkout `f41ddc55beddb4acf11643fc3263372a469c0d6d`, executable
SHA-256 `1f306c427d1a8c42b9d547b2b25a424e4b4e6a389a52ae5861d030e0fe43bc25`.
Checkout and executable identities were verified separately; their relation is
reported by the other project's evidence, not independently rebuilt here.
Current interface docs add single-occurrence keyboard fault injection and a
selected 286 floppy-read failure. The x64 software probe and media mapper are
still not a UEFI machine or x64 boot.

Not Super Tilt Bro checkout `3db162d178f2706c324c19d7e52cd7597a6fe189` was
clean. Its movement-evidence document (SHA-256
`773b76bb6d45fbbe709f9b4cdcb906da806f086c8027654e625e14c896ce4163`) reports
that its different experimental Mac guest resumed input after one stalled
command and one suppressed reply. The project adapted its startup from ClasSICK;
its movement slice is source-game-derived. This observation suggests useful
recovery scenarios but does not prove ClasSICK recovery, historical timing or
physical behavior. No game code, art, NES media, emulator implementation or
Apple implementation was used. SRC-0607; byte-identical document intake and
hash manifest are at `C:/ClasSICK/boothush-intake-20261114-a`.

## Mac keyboard

Using the 65,536-byte ClasSICK SPEC-0100 candidate SHA-256
`b3a50ad806b589fdc02a4de579e6d72700c9107fb273d9ca73026320bebed807`, the new
[fault harness](../../scripts/Test-BootHushMacKeyboardFaults.py) ran eight
scenarios twice, with and without the keyboard witness. The guest state and
stop cycle/PC matched in each pair; the requested witness ranges were reached
without dropped or rejected rows. All injected faults triggered.

| Scenario | Guest observation |
| --- | --- |
| Normal | No invalid reply or timeout |
| Command stall, suppressed reply, 1,500,000-cycle disconnect | Timeout latched once |
| Reply delayed 991,993 cycles / 991,994 cycles | No timeout / timeout latched |
| Malformed byte zero | One invalid reply; no timeout |
| Malformed byte followed by press/release | One invalid reply, both subsequent transitions delivered in order, final key up |

The delay boundary belongs only to this guest/model combination. The malformed
reply case demonstrates continued polling after this invalid byte, not recovery
from a timeout: timeout remains latched until reboot. Traces and exact JSON are
in `C:/ClasSICK/mac-keyboard-faults-20261114-b`. A separate 12-frame BootHush
video and a lossless PNG are in `C:/ClasSICK/mac-keyboard-faults-20261114-a`;
the frame is pixel-identical to the already indexed
[ClasSICK scene](../evidence/screenshots/2026-11-13/boothush-queued-key.png).
The image itself does not establish keyboard delivery.

## Mac mouse

A fresh sweep of the same candidate on this BootHush executable reproduced the
existing speed-dependent failure: 0/4 directions passed at 2,048 and 2,560
cycles per edge; 4/4 passed at 3,072, 3,584 and 4,096. These are modeled cases,
not a physical rate envelope. The 8,192-cycle guard-zero control again failed.
The [prior model witness analysis](boothush-developer-report-20261113-c.md)
remains relevant; cause has not been assigned. Exact output:
`C:/ClasSICK/mac-mouse-sweep-20261114.json`.

## 286 work

The [two-track diagnostic loader test](../../scripts/Test-BootHushPC286Loader.py)
now forces first-track (`0:0:2`) and second-track (`0:1:1`) failures separately
for an original 7,169-byte payload. Each injected one BIOS status `0x20`,
reached guest POST `0xEE`, retained the expected full 7,169-byte RAM window,
and left the floppy hash unchanged. In the first failure the window remained
zero; in the second, the first 7,168 bytes were loaded and the next byte was
zero. Normal payload sizes 6, 7,168, 7,169 and 14,848 and four bad input
refusals also passed. Results: `C:/ClasSICK/boothush-loader-faults-20261114-b`.
TEST-0169. This is an original diagnostic loader, not an installer or OS boot.

The actual 16-bit planner-to-span-to-capability diagnostic was rerun on the new
BootHush binary: baseline `A9` and wrong-expectation controls `C6`/`C9` reached
their expected stops, with checked backing/payload and the near-data refusal.
Results: `C:/ClasSICK/boothush-286-chain-20261114-a`. The 286 production
boot path, installed system, application compatibility and physical performance
remain open.

## Improvements requested from BootHush developers

1. **x64 boot, highest priority.** Implement and connect an actual bounded
   headless x86-64 machine and guest-callable UEFI environment for the original
   ClasSICK GPT/FAT32 EFI disk. Include proper application arguments, memory
   map and `ExitBootServices`, GOP, the required timer, PS/2 input, guest RAM
   inspection, capture and deterministic scheduling/replay. Prove each stage
   with original synthetic tests and then the identified real guest; report
   actual stage and limitations. The existing mapper/software probe are not a
   boot. See [the detailed prior handoff](boothush-developer-report-20261113-c.md).
2. **Mac mouse timing.** Independently investigate SCC latch, phase,
   acknowledgement, CPU interrupt acceptance and VBL ordering at 2,048–4,096
   cycles. Retain the current ordered, bounded witness and traced/untraced
   comparisons. Show whether a model defect exists with an independent test;
   do not tune behavior merely to make ClasSICK pass or claim physical timing.
3. **Keyboard repeated faults.** Existing one-shot injection works and was used
   above. A bounded list of independently selected one-shot faults per run,
   including their individual trigger/outcome records, would help test repeated
   faults and a future explicit ClasSICK recovery contract. Preserve queue
   semantics, no-fault behavior, and witness loss reporting. A stopped-VBL
   control would help distinguish guest-clock timeout dependencies if it can be
   provided without changing the ordinary machine model.
4. **286 storage failure breadth.** Selected starting-CHS floppy failure now
   works and exercised both diagnostic branches. Future installation work would
   benefit from opt-in fixed-disk read/write errors and selection by operation
   ordinal or bounded range, with documented retry semantics, atomicity, status,
   and immutable original-media checks. This is a future test feature, not a
   report that normal BootHush storage is defective.

The paired release remains blocked by Mac fast-input uncertainty, missing x64
BootHush execution, and the existing physical/human and qualification gates.
No other emulator, reference system, or desktop session was used.
