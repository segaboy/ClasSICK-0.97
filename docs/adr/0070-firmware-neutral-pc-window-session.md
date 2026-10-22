# ADR-0070 - Keep the native PC window session independent of firmware

Accepted2026-10-22;SPEC-0061;original engineering continuation of SPEC-0059.

Compose the existing single-owner controller, explicit delivery receipt, shared
window input owner and monochrome row presenter behind a caller-owned native
session. Keep clock acquisition, UEFI handoff validation, arena allocation and
trace serialization in the later boot wrapper. This permits direct transcript
and fault testing of ordering and retained state before a new boot image exists.

Use one post-cancel startup publication, one controller poll per turn and explicit
PACKET/LOST/NONE dispatch. Drain keyboard events without turning Space/Escape into
window clicks or a stop request. Retain partial effects on faults and require
explicit quiescence/detach before storage reuse. No automatic repair or rollback.

One bounded Claude primary supplies the proposed production and test artifacts.
Codex freezes the API, reviews, integrates and executes checks; these tests share
the primary author and are not an independent oracle. Reuse the separately tested
component oracles and add lead checks for concrete gaps. No redundant worker
fan-out or new qualification. All target-trial and release gates remain separate.
