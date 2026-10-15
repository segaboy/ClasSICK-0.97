# ADR-0038 - Guarded single-root execution before wider application services

Accepted engineering contract, 2026-10-15. SRC-0179..0182 / SPEC-0034.
Unimplemented; no code, fixture, TEST/IMPL/build or executed case.

The verified SPEC33 loader owns nonexecutable data. Its EMPTY-only external gate,
fixed map/inventory and zero-world retirement assumptions do not admit a session
over that image. Existing bounded CPU/session/typed-Toolbox services supply useful
mechanics, but neither map permissions nor a null dispatch hook supplies ownership
and write enforcement. The legacy A-line fallback must be denied explicitly.

Choose a new bridge-aware lease with one generation-bound root, strict24, bounded
budget, balanced RTS completion, immutable code/table/cells and controlled writes
to below-A5 globals/stack. Enter the first row's JMP at row+2; protect its ID and
extension entry points, segment headers and logical-length padding. Keep token RX
storage in the original map before image initialization. The guarded transaction
checks accesses, instruction class and proposed A7/SR before staged commit.

Expected guest/budget/CPU faults stop without resume while retaining diagnostics;
invariant/ownership failures quarantine without recovery. Explicit quiescent
handback restores only writable globals to canonical zero, preserves stack and
stop snapshots, reconciles the sealed inventory through a new typed API, then
uses existing release. No guessed digest, metadata patch or post-write rollback.

Defer all traps/services, callbacks/patches/interrupts, DATA24, MM operations,
dynamic segments, Finder/launch globals and arbitrary native executors. Existing
typed root-only ExitToShell is implemented but intentionally excluded until it
has this policy-aware closure. No historical startup, full CPU/OS, boot or128K
fit follows. X-1 guarded transaction checks are the first future implementation.

Claude authors the first draft, Grok checks one risk, Codex adjudicates and is the
sole repository writer. Two scoped CLI calls use small repo briefs/ranges; no
truncation/retry. Word overruns and one unnecessary EOF read are recorded, not
hidden. [Review](../development/owned-image-execution-review.md). Prior compiled/
build/edition/source/UART/native/physical/parity/B2/human gates and24 originals/
copies remain; both System1/ROM boot directions stay untested. Git wiki queue only.
