# First native PC motion diagnostic

2026-10-24,SRC-0429/0430 / SPEC-0073 / IMPL-0075 / TEST-0088.
Prior runtimeb3cb3fe now passes BOTH native/Windows jobs in CI40042117791.
Mac coherent motion and PC GUI drag remain release blockers.

The optional first-LOST probe has not triggered during prior GUI failures.
An explicit motion mode instead stops after the first successfully decoded
PACKET with nonzero dx/dy, using the existing readonly post-dispatch observer
and eight-register backend. It returns without effects for NULL/NONE/LOST/
zero-motion packets. Normal images exclude both probe objects. No decoder,
owner, presentation, controller, loss or GUI-acceptance policy is changed.
One packet cannot alone establish a host cause or complete drag qualification.

One Copilot Sonnet5.5 low primary supplies original code/tests in41.16s;
one frozen-packet view, no writes/commands/tests. Lead adds numeric-mode
validation (including empty/identifier refusal), an unexpected-backend guard
for deterministic test failure, build/package mode controls and provenance.
Legacy13 and new35 actual checks pass in Clang Debug/GCC Release/ASan-UBSan.
Three compiled faulty controls are rejected; three invalid macro values and
five build/package mode mismatches refuse. The first PS5 child fails policy
before tests; normal configured PS7 runs the controls without a bypass.
Author tests are literal oracles, not an independent implementation review.

Both normal and motion profiles build four matching twins, with27 actual
corruption controls each and two omitted-probe controls for the motion image.
Normal O2 SHA256 de02e416fc92bc69fb4e07e41eb8a79a2f1cd00493e12104e1e47dba55e55d0f
is byte-identical to the existing candidate. Default first-LOST word oracle
remains unchanged. New diagnostic source-committed rebuild/trial pending;
proposed tests are never counted as execution. No release/System1/physical/
native-app qualification.286 evidence retained, no new backend/target run.

Worker reports one premium request; remaining Copilot/Claude/Grok/Argon
balances unavailable. No repeated qualification, second reviewer or net
Codex-quota-saving claim. [Receipt](pc-first-motion-evidence.json).
