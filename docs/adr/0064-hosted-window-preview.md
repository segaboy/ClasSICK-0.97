# ADR-0064 - Separate hosted preview with unscaled presentation

Accepted2026-10-21;SPEC-0056;SRC-0332..0334.
Keep the old viewer/presenter and both boot workloads unchanged. Bind the shared
owner through one opt-in Windows adapter with explicit capture/cancel semantics.
Wrap the legacy presenter in a fixed-size viewport so physical640x480 and logical
512x342 stay unscaled at every client size. Synthetic API/pixel evidence stays
separate from GUI/human and exact-image boot qualification. No asymmetric release
or external native-application gate change. [Contract](../specifications/SPEC-0056-hosted-window-preview.md).
