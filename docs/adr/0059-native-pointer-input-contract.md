# ADR-0059 - Native pointer/input semantics and source boundary

Accepted2026-10-19,SRC-0310..0313;SPEC-0051 core contract only,no implementation.

Choose a caller-owned native pointer state and complete sample/event values on logical512x342,center start256/171,+xright/+ydown. Use checked signed clamps rather than overflowing accumulation;lower3 buttons are telemetry,only LEFT creates actions. Motion precedes edges. Begin-held and cancel suppress until a released baseline;cancel never counts a press. One owner publishes each transition once to a separate compatible saturating counter mirror and future window/cursor consumers. Validation refusals leave state/output bytes unchanged;accepted state survives downstream faults. No hidden exactly-once replay/security/invalid-pointer guarantees.

Cursor composition remains the next API contract. It must bound save-under/art/metadata,restore before underlying mutation,repair old overlay areas,retain truthful live bindings and reject stale/exhausted revision identity. No full-screen copy or silent wrap/repaint recovery.64KiBROM target and measured native footprint remain requirements,not a128K System claim.

Distinguish E1hosted,E2device,E3host-GUI and E4human. Previous headless input pass is E2. PC capture/focus/GUI startup causes are unproved. Mac motion-source lane is held under the source policy pending owner disposition of one unrelated manual declaration;independent engineering work has no manual exposure. No hardware facts or algorithms from that peek are adopted.

[Lead review](../development/pointer-input-contract-review.md),[contract](../specifications/SPEC-0051-native-pointer-input.md),[inventory](../development/pointer-source-and-input-inventory.md). All68 case identifiers are unexecuted;no header/code/test/build/target/release change. Later contracts must resolve binding/source/renderer details before admission. Existing source/role/edition/physical/full128K/B2/human/native-app/Linux/both System1-ROM gates persist.
