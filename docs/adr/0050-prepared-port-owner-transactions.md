# ADR-0050 - Prepared-port owner transactions

Accepted engineering contract2026-10-17, SPEC-0044 / SRC-0262..0265.
Use one inactive native admission transaction: validate canonical ownership and
all translated destinations before publishing the claim and installing g/current0.
Keep exact-g integrity exclusive to a fully bound owner profile; no generic dirty
flag or pure-handler hidden span. Separate logical current-cell RMW from absolute
native-control/backing disjointness. Borrow views for the root; after proved
quiescence cleanup uses scalar owned layout and original image-owned maps only.
Canonical retirement keeps consumed-generation and nonreentry protection through
RELEASE success/failure. Unresolved borrows/quarantine grant no cleanup or recovery.
These are prospective rules, not implemented services or historical observations.
[Dispositions and limits](../development/prepared-port-owner-lead-review.md).
