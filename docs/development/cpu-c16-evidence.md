# CPU exclusion and C16 facade checkpoint

2026-10-23; SRC-0394..0398 / ADR-0079. [Machine receipt](cpu-c16-evidence.json).
Paired-native-preview-2 remains primary; the286 is a bounded secondary lane.

Release prerequisite: [SPEC-0068](../specifications/SPEC-0068-m68k-short-exclusion.md)
adds isolated original supervisor exclusion callbacks, not an ISR or driver.
The token records SR at the asm snapshot; C caller condition codes are not
promised across the ABI. IPL7 does not exclude level7 transitions, faults or
DMA. Hooks must bracket only the short inbox operation, never owner/rendering.
Lead fixes the audit's blank-line gap and comment-based clobber false positive,
bounds compiler calls and retains own disassembly. Source-pinned [Windows/68000 CI](https://github.com/segaboy/ClasSICK-0.97/actions/runs/39989825970) passes at `f69f27f97238c9666a21d8c7df1ee1c172dd43db`.
Actual native checks: six objects, matching O0/O2/Os twins, no imports or mutable
data; largest reported callback function frame 4 bytes. Retained own
disassembly is reviewed for snapshot/mask and token-to-SR flow. O2/Os function
text is14+8=22 bytes. The optimized .su value0 is not zero stack use: arguments,
return addresses and caller/interrupt frames remain. These are compiler facts,
not execution, full-stack, latency or whole128K proof.

Actual local verification:341 full ClangDebug CTests, plus one GCCRelease and
one ClangASan+UBSan facade run:343 final profile checks. The facade has77
assertions per profile; the audit has9 host test methods and a256-state abstract
SR model.22,176 boundary combinations verify the [surface width proof](c16-surface-width-proof.md).
The privileged callbacks have never been executed. Initial tests are lead
controls after primary read, not final author-blind tests.

286 result: owned C99 assertion and checked chunk/flag/guest-offset facade,
plus a pinned optional [delta reproducer](../../scripts/Probe-PC286C16.py).
Final9 compiler calls yield6 helper-free own objects and3 deliberate assertion
refusals. Initial3 calls hit a harness diagnostic-name assumption; it now checks
the exact source line and E1020. No link/286 run. Small/medium/large remain
candidates, -s stays diagnostic, general C alignment and segmented views remain
open. No production guest/surface source changed or compiler helper admitted.

Source review: narrow Motorola1992 PRM SR/instruction interfaces and Motorola1993
UM6.3.2 interrupt prose; GCC compiler-clobber prose. Web parsing only: screenshot
pixels could not be inspected and direct publisher PDF downloads were refused,
so no new local PDF hash or visual-page claim. An unrelated compiler example
was excluded, affected work stopped and the owner explicitly admitted a fresh
unexposed worker from reviewed interface facts. No example/manual was sent or
reused. This is hardware/compiler interface evidence, not1984 OS behavior.

One fresh Claude Opus5.5 low call:104.66s,11924-byte117-line packet,276 artifact
lines/522 contract words, no tools/edits/tests/retry. Its extra trailing JSON
brace is removed as packaging; code/audit adopted with lead corrections.
Codex directly handles the small independent C16 delta, integration and checks.
No redundant provider or requalification. Reported$0.2924904 list/session cost
is not a balance or net Codex saving; all-four remaining allowances unverified.
Raw captures/builds/objects stay outside Git at C:/ClasSICK/cpu-c16-20261023-a.

No boot binding, ISR/SCC/VIA/vector/ack/RTE implementation, target/VM/emulator/
hardware run, release, screenshot or GIF. M1-M7, whole128K/stack/latency, GUI/
human/CWP1, physical/edition/historical/B2/native-app/BIOS/Linux and both
System1-ROM directions remain open or partial. Preserve24 originals, held brief,
UART bodies, old records/assets/tags and retired wiki. No OneNote, new chats,
subagents or automation. Next: exact ISR/SCC source admission and cost accounting;
286: per-model alignment evidence and checked segmented guest views.
