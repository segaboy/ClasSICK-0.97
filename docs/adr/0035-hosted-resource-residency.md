# ADR-0035 - Own resident resource copies through explicit host leases

Accepted engineering design, 2026-10-14; SRC-0155..0158 / SPEC-0031.
Contract only, unimplemented; lead self-review, human provenance review pending.

The immutable reader exposes file spans but owns no heap lifetime. The bounded
heap lifecycle retires application handles and refuses unknown retained owners.
A loader cannot safely keep unchecked resource pointers across allocation,
release, reset or execution merely because a lookup returned bytes.

Choose one borrowed immutable source, at most16 locked application cache entries
and32 explicit native lease tickets. Same-key acquisitions share residency but
issue separate tickets. Release ends one borrow without disposal; eviction and
close require zero leases. Saturating identifiers and the captured heap epoch
qualify native ownership; raw guest handle numbers remain reusable.

OPEN/BUSY/FAULTED service state is FUTURE/REFUSE to the existing lifecycle.
Close before resetting, without adding a guessed global scrub or implicit
resource teardown to SPEC-0030. Allocator success can compact other application
objects; retained post-allocation/disposal faults are distinct from unchanged
precommit refusals. No rollback, automatic repair or callback occurs.

Defer historical guest Resource Manager APIs, system-resource search/resolution,
attribute execution semantics, purging and dynamic segment loading. File flags
are metadata except the explicit system-heap/reference refusals. Host-owned
copies use current original manager capabilities; no Apple resource-map or heap
structure is reconstructed. A data lease supplies no executable authority.

Alternatives rejected for this first scope: returning bare long-lived pointers;
using generic manager initialization for cleanup; blindly deleting resources
during reset; treating release as historical disposal; silently following bit7
references or binding GetResource before its independent calling/error contract.
Full reference search/purge/re-entry semantics may follow reviewed later contracts.

Consequences: explicit capacity and source-buffer charges; callers must evict/close,
root execution is not admitted, and a future loader must define its continuation
and ownership boundary. This is a prerequisite with original semantics, not
historical compatibility or a final128K proof. No code/tests/build or VM run.
