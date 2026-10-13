# Bounded Memory Manager checkpoint - 2026-10-12

SPEC-0023 / ADR-0027 / IMPL-0027 / TEST-0031 / SRC-0101-0104.
Compiled source `58a1ca5ef680e2e7f7eed85d3880bbed42eb675f`; lead self-review, human provenance review pending.

The shared core now implements eleven bounded pointer/handle/size/lock/copy
services. Caller-owned regions and an original 32-record table hold allocation
state; four-byte guest cells keep handles stable while unlocked payloads move.
CLEAR/SYS allocation, guest-written zone/limit globals, idempotent locking,
prefix-preserving resize and overlap-safe BlockMove are explicit contracts.
Requests and placements are checked before committing data; failed plans preserve
the manager and RAM. [Final contract](../specifications/SPEC-0023-bounded-memory-manager.md)
defines engineering choices and historical questions separately.

The separate guest-register adapter uses the reviewed eleven trap numbers and
per-routine result conventions. It preserves other registers/PC/A7 and tests
D0.low16, including the long-size word-test quirk. It is not installed in the
existing trap-address directory or CPU exception path. The
[interface registry](../interfaces/system1-routines.json) records thirteen guest
contracts, including Get/SetTrapAddress, with unknown introduction releases;
it generates no native application headers/SDK.

## Verification

The full Windows chain passes 1,245 CTest checks in nine configurations:
151/151/151/124/151/136/136/136/109. Twelve new behavior suites plus three
object audits run per configuration, including compiler widths, GCC/Clang and
sanitizers. Independent interval/guard/model checks cover 1,500 mixed operations,
every surviving payload pattern and fixed/locked identity, transaction failures,
corrupted cells/globals, copy overlap and 65,448 unsupported register words.
The common original probe passes 20 checks/status 0/signature 48454131 across all
eleven routines. Thirteen interface records/eight rejection controls pass.

Thirty extra new object audits pass for Clang/GCC 32/64 O0/O2 and ARM64
little/big-endian. New hosted executable twins match. Existing guest/CPU/service/
EFI/media/Mac checks remain passing. GNU 68000 GCC 13.3 O0/Os passes ten object
audits and two complete relocatable links, with no external runtime imports or
mutable module globals. Native manager metadata is 672 bytes, each record 20,
guest master cells 256 total. Largest reported function frames 608/604 are compiler
facts, not a measured stack high-water or final linked-ROM/RAM budget.

[Implementation CI](https://github.com/segaboy/ClasSICK-0.97/actions/runs/38328896626) passes both jobs. Its complete manager manifest matches
local exactly: both compiled core fingerprints, every input, function-frame list
and storage budget. Six existing EFI profiles match local/prior fingerprints.
CI rebuilds the four existing own ROM twins and controls; both profile hashes and
every ROM input match the prior admitted local baseline. No new local ROM rebuild
was needed because all those inputs were unchanged. No old native observation is
extended. [Sanitized results](memory-manager-evidence.json) retain full checksums
and bounded facts, with no private paths, images, VM IDs or reference pages.

## Scope and next step

This is a bounded engineering profile informed by published 1985 interfaces,
not confirmed January 1984 behavior. Both original Claude drafts are now preserved
unchanged as versioned research; [lead review](../research/memory-manager-lead-review.md)
and the final contract take precedence over proposals/OPEN questions. The owner's
published copy was identity-checked and selected API/constant pages visually
reviewed. Ancillary public layout/example declarations were not used as an
allocator, copied structure or implementation input.

Fixed zone identities are opaque tokens, not historical Zone records; 32 live
objects and conservative compaction limit capacity. No purging, automatic zone
growth, grow-zone callback or linked Pascal MemError global is implemented.
Raw stale addresses cannot be distinguished after reuse. All C/register address
arguments remain strict logical values; explicit 68000 data-address normalization
and typed guest trap binding are next, followed by original guest heap probes.

The new routines are not linked into either boot entry. No emulator/VM or
operator session was performed; a future run needs the owner's go-ahead. X86
application/loader/SDK work remains gated until his explicit complete-68k
confirmation; own native core/test tools are unaffected. Historical reference
validation, physical hardware, full budgets/parity/three editions and formal B2
criterion 6 remain open. No Apple/Microsoft code/assets, emulator implementation,
firmware internals, UART body work, delegation or OneNote operation.

## Retained development history

Focused and full code suites passed without a failed code-test run. Review
parenthesized native-pointer offsets before compilation. The initial PDF
renderer package was unavailable; bundled PDFium completed the selected-page
review. The first helper import generated one local bytecode cache; it was
removed and bytecode writes disabled. Earlier local manifests remain retained;
the final adds source/tool/script fingerprints and the finalized boundary text.
No protected page, source sample, binary cache or reference executable is published.
