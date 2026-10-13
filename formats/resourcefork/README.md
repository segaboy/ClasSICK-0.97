# Resource-fork reader

Original bounded read-only metadata service, `classick-resource-reader-v1`.
[Contract](../../docs/specifications/SPEC-0024-resource-fork-reader.md),
[decision](../../docs/adr/0028-bounded-resource-fork-reader.md),
[implementation](../../provenance/records/IMPL-0031-resource-fork-reader.md) and
[conformance](../../provenance/records/TEST-0032-resource-fork-plan.md).

`cs_rf_open` validates an immutable caller buffer; `cs_rf_find` returns exact
numeric type/signed-ID metadata. Offsets are checked file positions, never
native pointers. Requested ambiguity is refused; bit-7 references have metadata
and no local payload. No allocation, I/O, payload execution or Resource Manager
loading/search chain. Optional strict audit, writing and guest adapters remain
future contracts. Use truthful disjoint live objects and constructor-produced
immutable views as described in reader.h.

[Verified checkpoint](../../docs/development/resource-reader-evidence.md).
Fixtures/model are original and generated; no Apple file or resource is an input.
Engineering conformance does not establish January1984 behavior or a native boot.
