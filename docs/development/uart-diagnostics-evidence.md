# UART diagnostics verification snapshot

Date 2026-10-08. SPEC-0013 / ADR-0017 / IMPL-0017 / TEST-0019.
Fresh implementation-agent scope; original synthetic fixtures and owned source.
No real ports, firmware, VM, native serial delivery or edition boot observed.

Final full local verification uses `Verify-UARTDiagnostics.ps1 -Sanitizers`,
never-reused `C:\ClasSICK\uart-verification-20261008-b` and adjacent
`C:\ClasSICK\uart-verification-20261008-b.log`. The complete wrapper passed
with exit zero under Windows PowerShell 5.1. Development and failed fixture logs
remain under
`C:\ClasSICK\uart-development-a`. Prior keyboard evidence/fingerprints are immutable.

Nine complete CTest configurations pass, 792 checks total: Clang x64 100 per
Debug twin/Release/ASan+UBSan, actual i686 76; scoped GCC x64 85 per Debug
twin/Release, actual i686 61. Prior debugger, core-link, loader, exceptions,
presenter, timer, interactive keyboard and 60-second behavior remain covered.
Twelve additional Clang/GCC portable/native O0/O2 object imports/frame audits and
two AArch64 LE/BE portable UART object audits pass. Four unloaded EFI twins pass
59 named symbols, three DIR64 relocations, one eight-byte writable image slot,
256 vectors, 23 corruption rejections and ten omitted-object links. No runtime
helpers, external-library imports or mutable globals are introduced by the UART
C sources.

The full [fingerprint manifest](uart-diagnostics-fingerprints.json) contains all
59 printed comparison labels / 53 distinct fingerprints. All 55 prior labels
remain: 51 match and four change (EFI O0/O2 and both native keyboard executables);
four UART labels are new. Of the preceding 49 distinct fingerprints, 45 remain
unchanged. The verified compiled/test/script inputs were staged/reviewed before
the run and are committed with this snapshot. Local `verified-inputs.json` and
`uart-aggregate.json` retain input hashes and the exact aggregate comparison;
subsequent changes are documentation/manifest evidence only. Final UTF-16 log SHA-256:
`71b7ef11904493f4cd828daa270bafa04ef64aeb29c5235d5a98f31e2f9e34c4`.

Compiled/test/script revision `3b44cb9510a400397a33cd1cc0802f45c36c8110`
passed pinned [Windows CI 37827697279](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37827697279)
by workflow dispatch on the existing `claude/ci-staging` branch before main
advanced. CI passes the same nine configurations and audits, plus two fresh
100-check bootstrap builds. All 59 local/CI comparison labels / 53 distinct
fingerprints match exactly, including the committed fingerprint manifest.
The UTF-8 CI log is retained at `C:\ClasSICK\uart-ci-37827697279.log`.
This follow-up records aggregate verification/publication only; it changes no
compiled source, tests or scripts from the verified revision. Implementation
and its review remain the owner-approved fresh agent's work; the lead handles
results and headquarters. Human provenance review remains pending.

| Comparison label | SHA-256 |
| --- | --- |
| EFI SHA-256 clang-64-O0 | `aaedda7e2c809203637e2ea46c7762339b6dc4500f3681fa284d4bc479f197d0` |
| EFI SHA-256 clang-64-O2 | `8e25d6947677b538d84bc17f98ff44d056dcd3d7769d1389676072286ad831f2` |
| UART SHA-256 clang classick_uart_tests.exe | `2747e2699ad81f36256f71db6765c461264c3a8a27447517ddfa9bd7c5c47fb7` |
| UART SHA-256 gcc classick_uart_tests.exe | `4778849ce445eed514cba4ec0a34a2749fc7b6abf9541b39169f7580a697b660` |
| UART SHA-256 clang classick_native_uart_tests.exe | `d818383de4cb1a87b3df488a2d76aa5a8762883d79705bc86825a0aface88791` |
| UART SHA-256 gcc classick_native_uart_tests.exe | `f2e96b679818da3f9d9521ae3bb9d33eeb46916f5882c1bcc168fb79d144808d` |

EFI file sizes are 55,808 bytes O0 and 37,888 O2; in-memory spans are 69,632 and
53,248 bytes. The payload remains a directory tree, not formatted boot media.

First full root `uart-verification-20261008-a` and adjacent log are retained:
normal Clang x64 100 checks per Debug twin/Release and i686 76 passed, then
the sanitized build rejected a 2216-byte instrumented host frame against the
native 2048-byte limit. CMake now applies that frame limit to uninstrumented
libraries/objects and native O0/O2 audits; sanitizer host instrumentation is
outside the native budget. The complete final matrix uses a distinct fresh root.

Behavior: original bounded register startup, 512-byte atomic queue and readable
ASCII scene/input/termination diagnostics. RAM retains serial startup/stall/line
failures and whole-record loss while keyboard work continues. Final drain uses
independent finite bounds and TEMT. New trace at 256 is 640 bytes; original records
and 512-KiB allocation persist. Native entry selects the diagnostic successor
after successful exit and owned-ready gates; native execution remains unverified.

TI provides scoped register semantics, not PC/VirtualBox mapping or clock proof.
Machine qualification, eligible exact firmware, formatted boot media and B2 remain
required. Real Macintosh/mini vMac, three physical-edition gates and historical
parity remain separate/not-started or not-tested. Human provenance review remains
pending; passing tests and matching CI do not prove native delivery or eligibility.
