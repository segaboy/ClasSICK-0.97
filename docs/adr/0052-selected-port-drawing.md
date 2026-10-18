# ADR-0052 - Explicit selected-port drawing profile

Accepted2026-10-18; SPEC-0046 / IMPL-0048 / TEST-0053; SRC-0270..0273.
PRF2 shares the complete prepared-port owner and canonical retirement. An explicit
entry preserves PRF1 selection-only behavior. All four slots stay unsupported;
the local coordinator resolves current afresh and draws via a root-local view
snapshot after every fallible guest/control check. Rect bytes are copied before
any native pixel or overlapping scratch store. Successful helper completion is
followed only by total scratch/frame stores. No pixel rollback or private guest
GrafPort reconstruction. Both architectures compile the same native C.
No target run or historical behavior inferred. [Receipt](../development/selected-drawing-evidence.md).
