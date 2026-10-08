# PS/2 keyboard verification snapshot

Date 2026-10-08. SPEC-0012 / ADR-0016 / IMPL-0016 / TEST-0018.
Original controller/scan/ACPI fixtures and guarded buffers only. No real port,
firmware, VM, native output or keyboard event has been observed.

The full fresh local `Verify-PS2Keyboard.ps1 -Sanitizers` run uses
`C:\ClasSICK\ps2-keyboard-verification-20261008-b`, with its adjacent log.
The wrapper completed successfully (exit zero): Clang x64 91 checks per Debug
twin, Release and ASan/UBSan, actual i686 71; scoped GCC x64 76 per Debug twin
and Release, actual i686 56. Prior debugger/core-link/loader/presenter/timer
controls remain. Four unloaded EFI O0/O2 twins pass 53 named symbols, three
DIR64 relocations, one eight-byte writable slot, 23 corruption rejections and
eight omitted-object links. AArch64 LE/BE keyboard import audits pass.
Immutable source and CI identities will be added after remote verification.
The first root (-a) stopped on GCC test-file formatting warnings; its log is
retained. Corrected tests pass GCC's new ten-check development selection.

The earlier fresh local progress-loop baseline at f1e7a09 passed and matched
CI 37812572045: 49 comparison labels / 43 distinct artifact fingerprints. Its
original logs and hash records are retained; one Debug directory was subsequently
reused for development and its rebuilt binaries are not baseline evidence.

The wrapper prints 55 comparison labels representing 49 distinct artifact
fingerprints. Compared with the prior 43 fingerprints, 33 remain unchanged,
ten change (ACPI tests, native gate/timer/loop tests and EFI images), and six are
new. ACPI's appended presence function changes binaries that link that object.

| Artifact | SHA-256 |
| --- | --- |
| EFI O0 | `8bbf185bf5d2f54e038475bbeff2e4f10e0d429fddfcc5764463986c0f37a72c` |
| EFI O2 | `4c7df6a77f4ec3240a46669fcf264f776a2ff17ef9ec11376b222a4173f4369a` |
| Clang PS/2 | `ec4f60b7d22ecca0a1ecb2843529a0ebb9042aa8898bf1cf36d14f31d2532818` |
| GCC PS/2 | `8bfceb5ae1e75ca719ee51e7f7266fef0a04d40987ac6de6f3b7a5fc336ac027` |
| Clang ACPI controller | `e6f7cf02971aafc6670098a58bfe30c4be35fbbdc4d1d63ee2707d6acc148451` |
| GCC ACPI controller | `2032a14fd2fbf2bf667fdda570e578d42b0db79b200d1f942c32e221ded938d3` |
| Clang interactive loop | `56e831550b69344402e25fed688520fa2b4caad1ac03ed62b64a8b2ab0885284` |
| GCC interactive loop | `8f821ef25535655b36323a3156e4ee19f97f16a1b1fcc6395ea0625ca2b4d181` |

EFI files are 48,640 bytes (O0) and 33,280 bytes (O2); in-memory image spans
65,536 and 49,152 bytes. Payload is a directory tree, not formatted boot media.

Scope: polling startup/ACK/resend/error/set-2 state, normalized Space/Escape,
bounded diagnostics, ACPI presence gating and interactive scene behavior. Native
entry uses the successor with trace offset 128; SPEC-0011 remains separately
tested. B2, firmware self-test/A20 qualification, UART, media, Macintosh startup,
mini vMac and all physical edition gates remain open. Human provenance review
pending; lead self-review does not certify independent staffed separation.
