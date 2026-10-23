# Ordered native capture/drain implementation

2026-10-22;SRC-0366..0368;SPEC-0063/ADR-0072;IMPL-0065/TEST-0076.
The original bounded helper is implemented and locally verified. It queues each
normalized sample in order,delivers once through the existing input owner,and
handles visible loss/cancellation/rebaseline/stop without replay. It is sequential,
not interrupt-safe hardware capture. Native68000 CI is pending at this checkpoint.

One Claude Opus5.5 low primary returns259 C lines in53.97s from a complete
22,167B/439line original-contract/public-type packet. Zero tools,edits,tests,
retry,second worker or requalification. Lead adopts sound code,fixes the stop
path and adds a power-of-two assertion. Tests were authored from the frozen API
before implementation inspection and remained unchanged through initial integration. A preexisting CMake
executable name required renaming the new static library;no existing target edits.
Non-ready owner STATE preflight is clarified without changing the intended API.
Reported$0.2354344 list/session cost is not account balance or Codex savings.
[Assignment](tasks/window-capture-claude.md);raw captures/builds outside Git:
C:/ClasSICK/mac-capture-helper-20261022-a. Current quota sample remains private.

Actual final local verification:331 Clang64 regression CTests plus3 focused checks
each on ASan/UBSan,GCC64 Debug,GCC32 and GCC64 Release,total343 profile CTests including3 new GCC64 Release checks. Contract oracle1143
assertions/real-owner integration89 per profile;these are assertions,not1232
distinct scenarios. Real pointer boundary reversal,drag/content/counter and
held-button cancellation/recovery pass. Three actual compiled defects reject:
original erroneous stop skip,premature overflow discard,and diagnostic saturation
used as freshness. Unmodified control passes. Native compile/import/data audits:
12 width/endian/EFI-ABI objects,delayed twin byte equality,no mutable globals,
only two allowed original-owner imports,512-byte frame warning enforced.
Descriptor is256B on x64,248B on x86-32;68000 layout/stack/closure pending CI.
These are helper costs,not total ROM/RAM/stack/128K feasibility.

[TEST-0076](../../provenance/records/TEST-0076-ordered-window-capture.md) maps each
old planned MP case to actual scope. MP5 lacks a newly executed dragged-window
boundary-reversal subcase;MP11 is sequential,not IRQ publication;MP12 tests only
explicit software uncertainty,not hidden hardware loss. Earlier TEST-0075 stays
an accurate unexecuted plan. Native source-pinned CI result will be appended.

No MMIO,pin normalization,interrupt entry,CPU exclusion or Mac framebuffer/ROM
binding,boot-input change,target run,release or publication of new binaries.
The same C helper builds for native68000/x86-64;it does not make the Mac scene
move yet. No GIF is made because this slice has no visible product change.
Existing32-bit audits are portability checks,not a32-bit PC OS announcement.
Preserve24 originals,held brief,UART bodies,old records/assets/tags and retired
wiki. M1-M4/M6-M7 and the hardware part of M5,PC GUI/human/CWP1,source/role/
historical/edition/physical/full128K/B2/native-app/BIOS/Linux/both System1-ROM
gates remain. Next:the exact Mac SCC access/polarity/interrupt binding contract;
no further implementation or target trial is granted by this completed slice.
[Machine receipt](window-capture-evidence.json).


CI correction:run39880414054/source62926d1 passes68000 but fails the GCC Release
contract oracle. The lead's structure-value snapshots/by-value comparison did
not preserve unspecified padding. The same failure was reproduced locally at
GCC-O3. Raw byte-array snapshots replace those copies;the full-byte no-effect
assertions and1143 expected assertions remain. Corrected oracle passes release
and all previous profiles;all three compiled defective controls still reject
and the valid control passes. No production/helper change or new worker call.
The test source is therefore no longer unchanged from the initial author-blind
freeze;this is an explicit harness correction,not relaxed no-effect behavior.
New normal source-pinned CI pending;first failure and passing68000 manifest kept.


Verified source `d6e851fc42743430db371c0f599862231662defe`: [Windows and68000 CI](https://github.com/segaboy/ClasSICK-0.97/actions/runs/39882048452) both pass.
Retained classick-capture-build manifest matches every local normalized source/
header hash:78 target object compilations,6 closures,no imports/mutable data,
matching twins atO0/O2/Os. Actual68000 descriptor248B;
largest helper function frame32B,not a whole-call-graph
or System128K certificate. No target execution. Follow-up modifies metadata only.
