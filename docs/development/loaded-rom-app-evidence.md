# First loaded application verification - 2026-10-15

SPEC-0035 / ADR-0039 / IMPL-0039 / TEST-0043 / SRC-0187..0190.

The original resource/CODE loader now runs a separately authored 68k application in a dedicated guarded session, captures its result, explicitly retires its image, and supplies the boot screen with the observed output. Hosted execution returns D0=97 and writes 0x5A5A5A5A in five interpreter steps. The separate 64 KiB ROM is built; one explicitly authorized isolated Snow cold boot now passes.

## Final local evidence

All 2,118 CTests pass across nine final-source Clang/GCC configurations, including
32-bit, optimized and sanitizer builds. Six behavioral suites cover actual
PREPARE/LOAD/execution/retirement, single-use and budget effects, denied guest
accesses and forged returns, quarantine and result-derived pixel rendering.
The underlying full regression remains required; case counts are not a measure
of historical compatibility. All 20 additional native-object audits pass on
Clang/GCC 32/64-bit and ARM64 little/big endian. Debug executable twins match
within each compiler.

The inherited full verification chain passed before the first new object audit
found unwanted compiler memcpy/memset imports. Original explicit byte loops and
field initialization fixed those portability failures. All nine configurations
were then rebuilt and completely retested, followed by 20 passing new object
audits. Initial Python discovery and ROM-script import-call errors were also
corrected; failed artifacts/logs remain private and are not passing evidence.

GNU 68000 produces identical optimized ROM twins, SHA-256
`6e726ec3a5947ec60f16170be60262ab461625ac6f3698200daf221d2a0df256`.
The image is 65,536 bytes, of which 42,688 are used. Native data/BSS is 2,168
bytes; the dedicated translated guest/control workspace is 44,496 bytes at
[32,768,77,264). The native stack reserve is 24,320 bytes. Summing all reported
nonrecursive function frames plus 192 assembly/exception bytes gives a
conservative 9,388-byte bound; the largest individual frame is 604 bytes.
Seal, reset/exception/Line-A encodings, disjoint static layout, no undefined link
imports, original 12-byte assembly encoding and eight corruption controls pass.
This is a static profile check, not an observed complete System 1 or physical
128K result. The previous ROM profile sources are unchanged.

## Scoped worker contribution

Exactly two supervised inference calls, no retry: Claude supplied the original
portable demo's first 126 C lines from four public/original Reads; Codex corrected
permission names, checked all serialized offsets/bytes and integrated it. Grok
performed one selected claim/return/retirement review from nine Reads. Its five
objections are resolved against the shared decoder/commit checks and actual
admission rules; none supplies an admitted bypass. Quarantine retention and
READ-backed owned fetch are deliberate requirements. This was a bounded review,
not a whole-system safety certificate. Neither worker executed tests or wrote
the repository. Returned Read payloads and raw streams are privately preserved;
archive-time files are separately labeled, not asserted to be read-time copies.
Reported usage is recorded without a billing, quota or savings claim.

All 24 research originals remain unchanged/untracked with byte-identical private
copies. UART bodies were not read or edited. No HQ, raw research, manual,
credential, prohibited input or emulator source was sent to either worker.

## Authorized boot acceptance - passed

One fresh isolated Snow v1.5.0-317c188 Windows x86-64 Macintosh128K cold
start used the exact verified own ROM and no Apple ROM, disks, saved machine
state, shared directory or serial bridges. The owner explicitly answered
"Yes—run the isolated Snow boot trial" (SRC-0191). The admitted Snow executable
SHA-256 remains `32cf5285582c0f509e4f1ac38d86bfb90440bae9da43b7823bfb0841dea588e6`.

Its reset is logged at19:05:47 UTC. A partial redraw is visible at19:06:10.647;
the full APP/097, expected32-bit guest pattern and six completed phase markers
are visible by19:06:32.315. One automated alternate-button event advances the
kernel counter from0000 to0001. The complete result remains visible at19:07:30.093.
This checks live kernel input, not application Event Manager or human input.
These are coarse snapshots, not a calibrated timing or sustained-run test.
Full redraws visibly take time and reveal intermediate drawing; the prototype
is not yet a responsive application experience. Rendering latency needs a focused
follow-up before a larger interactive application.
The trial was closed normally at19:07:43; no overnight continuity is inferred.

The relative CLI workspace path initially failed before guest startup. Opening
the same prepared workspace by absolute GUI path succeeded. Accessible value
replacement did not update Snow's custom editor; focused literal typing did.
Automation needed the previously observed alternate mouse-button mapping.
No system input settings changed. Screenshots and original profile/log identities
are retained outside Git; the earlier preview remains labeled as hosted.
No second cold start or additional reference trial is claimed or authorized.
## Published compiler and regression checks - passed

Compiled source `e8a238b4dfb64d0847cb583ba55c75cac59bab21` passes both jobs in [CI run38790620845](https://github.com/segaboy/ClasSICK-0.97/actions/runs/38790620845).
All2,614 CTests pass: the complete2,118-check matrix plus496 checks in two fresh
bootstrap profiles. New object audits and inherited checks pass in CI too.
The downloaded loaded-ROM image and complete sanitized build manifest match
local exactly. The previous own-ROM profiles and every input fingerprint remain
unchanged; comparison sorts stack-frame rows only, without dropping fields.
The actual trial image is byte-identical to this CI artifact. No emulator or
reference system runs in CI. This evidence revision changes documentation only.


General SPEC-0034, QuickDraw/windows/events, disk boot and both required original
System 1/ROM directions remain incomplete. R-001/AL-2, native-application,
physical/128K/parity/edition/B2 and human provenance gates remain unchanged.

[Machine-readable evidence](loaded-rom-app-evidence.json);
[implementation and limits](loaded-rom-app-implementation.md).
