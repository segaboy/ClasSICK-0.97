# ADR-0074 - Implement pure ingress before CPU binding

2026-10-23;SPEC-0065,following SPEC-0064/ADR-0073. Select a caller-owned
16-record inbox and one embedded by-value claim. Pure operations have no
external imports,callbacks,MMIO or interrupt instructions. Foreground/producer
exclusion and memory ordering remain explicit caller requirements;do not call
this an interrupt-safe queue based on hosted tests. Keep existing sequential
capture helper unchanged. Synthetic harness coordinates it outside abstract
exclusion;production foreground coordinator is a separate later slice.

Adopt Claude primary with explicit complete(no claim) STATE interpretation.
Claimed record stays stable while producer stages newer records. Sticky loss
requires foreground discard and successful external cancel before resume;
successful baseline completion cannot erase new loss. Stop/first cause/epoch
exhaustion are explicit. No coalescing,upstream clamping or automatic replay.
Native object/layout/frame checks complement hosted assertions;neither proves
hardware capture timing,CPU entry/return or total128K budget.
