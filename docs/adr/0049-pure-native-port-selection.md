# ADR-0049 - Pure native selection before guest binding

Accepted2026-10-17; SPEC-0043 / SRC-0258..0261.
Use a synchronous borrowed table and value-only plan. Revalidate every active
record/view/backing on get/set, including the nonselected entry. Keep the raw
current value caller-supplied; cache nothing. Validate native span alignment and
pairwise disjointness before effects. Reject a bad supplied value inertly; only
the future admitted owner can classify trusted corruption as quarantine.
This keeps one native68000/x86 implementation without prematurely enabling a
guest ABI, globals installation, heap lifecycle, drawing adapter or hidden writes.
[Actual evidence](../development/native-selection-evidence.md).
