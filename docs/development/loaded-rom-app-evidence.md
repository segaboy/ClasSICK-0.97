# First loaded application verification - 2026-10-15

SPEC-0035 / ADR-0039 / IMPL-0039 / TEST-0043 / SRC-0187..0190.

The original resource/CODE loader now runs a separately authored 68k application in a dedicated guarded session, captures its result, explicitly retires its image, and supplies the boot screen with the observed output. Hosted execution returns D0=97 and writes 0x5A5A5A5A in five interpreter steps. The separate 64 KiB ROM is built; its actual boot has not yet been observed.

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

## Boot acceptance still pending

CI results will be added after the compiled revision passes. The host preview
is not an emulator screenshot. A fresh isolated Snow trial needs explicit owner
go-ahead; acceptance is APP/097, the written pattern and six completed phases,
with the kernel loop still alive. No Apple ROM, System, disk or resource is used.

General SPEC-0034, QuickDraw/windows/events, disk boot and both required original
System 1/ROM directions remain incomplete. R-001/AL-2, native-application,
physical/128K/parity/edition/B2 and human provenance gates remain unchanged.

[Machine-readable evidence](loaded-rom-app-evidence.json);
[implementation and limits](loaded-rom-app-implementation.md).
