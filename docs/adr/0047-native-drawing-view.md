# ADR-0047 - Native borrowed drawing view

Accepted2026-10-17; SRC-0240..0243; SPEC-0041.
Windows need distinct local coordinates and drawing bounds. Use a native caller-
owned view borrowing the existing fixed port, with signed16-range translation
and a physical rectangular clip. Validate before effects; empty/inverted bounds
remain empty; reuse the old rasterizer exactly once, including counted no-ops.
No heap/global state, guest layout or historical GetPort/SetOrigin equivalence.

Keep the old helper, guest fixture and released assets unchanged. The real native
workload exercises offset/clipping while preserving its existing image and failure
phases; compile the same C for68000/x86. Adopt reviewed worker artifacts and an
implementation-independent inverse-mapping pixel oracle. New compiled candidates
need fresh boot observation; no VM/release permission follows from this decision.
