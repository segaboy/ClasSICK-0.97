# IMPL-0032 - Owned bounded guest sessions

SPEC-0028 / ADR-0032 / TEST-0036 / SRC-0132/0133; 2026-10-13.
Author: Codex-assisted project lead; lead self-review, human provenance review
pending. Project-owned GPL-3.0-or-later, no third-party implementation or assets.

Original core/cpu/m68k-session.c/.h composes the existing returning-call service
without changing its API, CPU semantics or patch executor. Caller-owned session
state, explicit root stop/report and trusted exit request reuse validated tokens,
stack, depth and ancestor budgets. All caller registers restore; completed guest
effects remain. Returning callbacks use the owned session executor; no-return
signals from those callbacks are explicitly refused in this profile.

The 18-byte apps/guest-session-probe fixture and tests/m68k-session.c are original
OS/core conformance inputs, not application files or Claude's launch fixtures.
The push/pop/BSR/RTS encoding and two-byte stack effect are independently checked.
No resource, loader, native application, heap lifecycle, Toolbox signature, boot
binding or historical behavior is supplied. Interface/provenance facts reuse
the already reviewed project contracts only; no new manual/sample/executable/
emulator source, copied asset or private implementation consulted.

Verification machinery: strict CMake tests/freestanding object, compiler/profile
and endian audits, identical debug executable twins, original68000 complete closure
and assembly encoding/partial storage/frame measurements. Final execution evidence
belongs to SRC-0134 and the separate checkpoint report; planned research cases
and preflight results are not substituted for accepted final-source evidence.

All eight earlier research originals and concurrent Claude heap-lifecycle work
are preserved. No UART body work, delegation, chats, automation, OneNote or
VM/emulator/reference trial. Native owner gate and historical/physical/final128K/
parity/three editions/B2criterion6 remain open.
