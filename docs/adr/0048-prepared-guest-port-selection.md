# ADR-0048 - Prepared guest port selection

Accepted2026-10-17; SRC-0244..0248; SPEC-0042. Contract only.

Prepare an explicit owner-bound selection capability with up to2 disjoint port
identities and native views. One portable native semantic core serves68000/x86;
the interpreter handles guest serialization only. GetPort/SetPort use the live
guest current cell behind an immutable authenticated A5 anchor. No global cache,
host pointer serialization, hidden pure-handler writes or partial commit.

Keep the fixed-port drawing lane unchanged. The new selection lane initially
refuses drawing until a guest-record adapter exists. An opaque108-byte engineering
extent is not a recovered GrafPort layout. Prepare initial current0 without
claiming InitGraf. Full graphics/global initialization and regions defer.

Implement nothing in this slice. Future shared native core/tests precede guest
integration, which needs explicit header/environment-span/lifecycle amendments.
No guest binding, fixtures, registry admission, compiler/boot/release claim follows.
[Lead dispositions and sources](../development/guest-port-lead-review.md).
