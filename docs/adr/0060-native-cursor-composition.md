# ADR-0060 - Bounded original native cursor composition

Accepted engineering contract2026-10-19;SRC-0316..0318 / SPEC-0052.
No implementation,artwork,fixture,test execution,build,target trial or release.

Use one caller-owned cursor state and a separate bounded save-under payload
over an ordinary-RAM full-frame surface. Original mask/image art,logical512x342,
centering/clipping and exact background restore preserve both native targets.
Keep explicit lifecycle/token/repair ownership: never restore stale scene data,
reuse live identities or roll back an already accepted pointer transition.
The proposed API and all acceptance cases remain unimplemented/unexecuted.

[Contract and TEST-0062 case inventory](../specifications/SPEC-0052-native-cursor-composition.md),
[lead dispositions/worker receipt](../development/cursor-composition-review.json).
Mac motion M1-M7,PC GUI/human input and existing compatibility/source/release
gates remain open. Actual ROM/RAM/ABI/stack costs require later verification.
