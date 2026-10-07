# Bounded native memory

[SPEC-0003](../../docs/specifications/SPEC-0003-arenas.md) defines caller-owned
monotonic arenas. The core reserves aligned byte spans in a fixed region, reports
exhaustion and rejects arithmetic overflow. It never calls a heap or clears bytes.
Reset retires every span; individual free and historical handles are outside v1.

Backing lifetime, max_align_t alignment, truthful capacity, disjoint ownership and
synchronization are caller responsibilities. No guest address is a native pointer.
Tests use original bytes; the Windows viewer's host owns its single backing pool.
