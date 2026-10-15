# First loaded application: integration - 2026-10-15

SPEC-0035 / ADR-0039 / IMPL-0039 / TEST-0043 / SRC-0187..0190.
[Results](loaded-rom-app-evidence.md). This checkpoint integrates a complete
original application path rather than publishing another isolated prerequisite.

apps/loaded-rom-app/probe.c constructs an original366-byte CODE resource fork.
The normal parser/preflight/image APIs stage and load one CODE1 segment, not a
second loader. Original program: MOVEA.L A5,A0; MOVE.L #5A5A5A5A,-(A0);
MOVEQ #97,D0; RTS. Its assembly encoding is independently checked. Five interpreter
steps include the loaded jump-table JMP. Captured D0/pattern/phase/retirement
values drive apps/loaded-rom-app/scene.c. Native presentation is explicit;
there is no guest QuickDraw/Event Manager or historical startup ABI claim.

personality/system1/code-run.c supplies a dedicated synchronous single-root
session. code-image.c explicitly encompasses its execution participant in the
sole FUTURE/REFUSE image registration, enforces generation consumption and keeps
legacy APIs blocked until quiescent retirement. Existing descriptor/state/API
paths still grant no entry authority. The original caller frame remains intact;
separate report.guest retains the actual stop snapshot. No alternative executor,
service hook, callback, patch, native application entry or heap reset is accepted.

A coordinator-only admitted bus step shares the actual interpreter/staged commit,
allows guarded READ-backed CODE payload/row fetches while preserving original
non-EXEC map/backings, and intercepts only balanced root RTS before token fetch.
Ordinary guarded steps retain EXEC requirements. Pure native policy and lifetime/
complete-inventory/synchronization preconditions are explicit; the native primitive
alone does not authenticate a fabricated C object or mint a lease. Current
bindings/globals/cells/records/protected world are checked through execution.
Guest CPU/policy/budget faults retain effects; integrity faults quarantine.
Retirement validates, scrubs below-A5, records actual scrub count, and hands back
to the original RELEASE checks. It preserves stack and separate stop snapshot.

arch/m68k/loaded-reset.S/loaded-rom.ld and platform/mac128k/loaded-entry.c are a
separate own-ROM boot profile, reusing original startup/native Line-A checks,
service tests, graphics and mouse polling. Original ROM sources/build stay intact.
New native SP8000, stack2100..8000, original native data/BSS B00..2000, dedicated
workspace8000..1A000 and framebuffer1A700..1FC80 are disjoint in128KiB. Guest
logical RAM is translated through separate workspace buffers, not native control
storage. The new optimized ROM uses section garbage collection and twin hashes;
its checked static layout/stack bound is not a complete System1/physical128K proof.

## Worker dispositions and actual scope

Claude authored the portable demo's first126 lines from four approved Reads.
Lead corrected invented permission macro names, independently checked serialized
bytes/offsets, integrated and added negative-source/repeated-generation helpers.
The user-visible result comes from successful actual program execution. Grok
reviewed the selected claim/return/retirement boundary with nine Reads. Its trap
and SR objections omit the shared decoder's refusals/masked precommit check;
real loaded-source and retained guard suites cover those routes. Retaining
quarantine effects and admitted READ-backed fetch are required design choices.
An above-A5 token lies inside the excluded world range; CODE token candidates
also lack raw EXEC and lie in the application heap. No supplied counterexample
meets admission. Those findings are resolved, not an independent safety certificate.
No worker executed a test or wrote the repository. Raw streams/read payloads are
private; metadata records live-file copies separately from read-time returned text.

## Acceptance and next boundary

Hosted execution/result/retirement and a checked original ROM build are separate
from an observed boot. The prepared candidate must boot in an approved isolated
Snow trial, show the APP label/value97/written pattern/six phases, and retain
kernel input/loop activity. No trial is started without explicit owner go-ahead.
A host preview is labeled as such. No original Apple ROM/System/disk is used.

The larger SPEC34 general profile remains incomplete; this one-entry/segment
prototype is its narrow integrated demonstration. QuickDraw/windows/events,
filesystem/disk boot and both required original-System1/ROM directions remain
future work. No native app/SDK, new reference input, automatic OneNote or new
chat/agent. Human/source/UART/physical/128K/parity/edition/B2 gates persist.
