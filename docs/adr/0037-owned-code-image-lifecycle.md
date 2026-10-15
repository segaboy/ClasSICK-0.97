# ADR-0037 - Exclusive eager CODE image ownership before execution

Status: accepted engineering contract, 2026-10-15. SRC-0171/0172; SPEC-0033.
Unimplemented; no historical or executable application admission.

Read-only CODE preflight produces copied metadata, while the resource cache owns
shared data tickets and blocks execution/reset unless closed. Neither establishes
exclusive code lifetime or an atomic multi-segment allocation transaction.

Choose an original host-only eager image owner. PREPARE stages exactly selected
CODE bytes in bounded caller storage; LOAD creates exclusive locked application
handles and populates reserved A5 data only. The resource cache remains closed;
no handle transfer or duplicated cache copies are assumed. The first successful
allocation commits guest effects; ordinary later OOM retains PARTIAL for explicit
release, while unexpected faults remain quarantined. Neither promises rollback.
Native-only cancel precedes allocation;
explicit inactive release clears owned world data and disposes only owned handles.

All segments, including CODE1, remain pinned through the entire image lifetime.
Destination regions and their physical aliases must be non-EXEC in this profile.
The image external gate permits EMPTY only in this profile. Actual execution
requires a later fetch/write/manager/re-entry admission bridge; no existing gate
is silently weakened. Dynamic loading/unloading, purging, full launch globals,
Finder and heap-reset composition are deferred. Original System1/ROM boot targets
remain required/unverified, independently of this hosted foundation.

Alternatives deferred: cache-ticket promotion without a transfer contract;
multi-allocation rollback despite compaction; free-byte totals as placement proof;
global RWX permissions as code ownership; individual unload with unknown returns.
This contract selects none. Human provenance review, edition/historical/full128K/
physical/native/source/UART/B2 gates and24 unchanged research originals remain.
