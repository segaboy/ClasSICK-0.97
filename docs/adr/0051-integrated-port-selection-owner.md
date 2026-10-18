# ADR-0051 - One integrated prepared-port owner

2026-10-17; accepted engineering decision; SPEC-0045 / IMPL-0047 / TEST-0052.
Owner continuation SRC-0266 authorizes the previously planned integrated slice.
The native C coordinator reuses actual image ownership, canonical integrity and
RELEASE checks. GetPort/SetPort use the existing pure selection core, bounded
owner-made snapshots and pinned total commits. Legacy default/fixed paths keep
their own zero-header contract. Scalar-only stopped state retains complete first
stop/fault evidence and consumed-generation protection through cleanup handback.

The same OS code compiles for68000/x86; legacy application instructions alone use
the compatibility interpreter. No global trap identity or selected-port drawing
ABI follows. Unresolved synchronous lifetime prevents cleanup/reuse. Original
trusted fault schedules exist only in conformance builds,never production.
Independent test author receives frozen interfaces/contracts/old fixture helpers,
with new implementation withheld. Lead corrections distinguish oracle errors from
implementation defects. [Actual evidence](../development/port-owner-evidence.md).
