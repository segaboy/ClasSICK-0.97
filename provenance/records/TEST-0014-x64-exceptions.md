# TEST-0014 — Hosted x64 descriptor bytes and unloaded fault paths

Date 2026-10-08. SPEC-0008 / ADR-0012 / IMPL-0012; original fixtures only.
No privileged instruction, firmware, device, VM or OS image executed.

Protocol Verify-X64Exceptions.ps1 retains the complete TEST-0013 wrapper and adds
four serializer suites plus optimized freestanding object audit at both host widths.
Clang/GCC Debug twins and new hosted artifact comparisons; ASan/UBSan under the
validated Clang configuration. All vector/gate addresses and reserved bytes are
decoded independently. Seventeen unaligned guarded layouts, 512 varied full-width
layouts, seven top-of-range cases, touching ranges, rejection stability/overlap and
independent instances. Hosted tests do not pretend to inject hardware exceptions.

Own EFI image audit now retains 22 named symbols and all 256 32-byte stubs, including
CPU-error vectors and immediate 255. It checks exact LGDT/far-CS/segment/LLDT/LTR/
LIDT/ready instructions, active pointer targets, locked claim, frame fields, CR2,
publication and terminal destinations. Exactly one writable/non-executable 8-byte
image slot is permitted; tables live in the bundle. Twenty corrupted images and
omitted transition/exception-object links must reject. Four O0/O2 twins remain
unloaded; payload is a directory, not a boot disk.

Initial five-suite Debug development pass and fresh image-only replay pass.
The first image audit correctly rejected the new writable slot under the previous
all-read-only profile; its reviewed bounded 8-byte data allowance succeeds.
Complete source-pinned matrix and CI results are recorded in
[the snapshot](../../docs/development/x64-exception-evidence.md).

Hardware descriptor effects, injected faults/nested capture/IST stack use and
native ABI are unverified. Profile assumes one CPU, identity mapping, CPL0,
CET/FRED disabled and truthful live permissions; installation is not atomic.
No B2, historical System 1 behavior, Mac/mini vMac or physical-edition result.
