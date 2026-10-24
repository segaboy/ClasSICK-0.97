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


## Completed native observation (SRC-0431)

Committed source `999f5bb7af939b4d4288ba6d351c8300924519fb` passes both
CI jobs in [run40094774719](https://github.com/segaboy/ClasSICK-0.97/actions/runs/40094774719).
Fresh verified diagnostic EFI184832B SHA256
`bf0b1026598ae00eb7bee19f7fe21c8b28dd770af06307141636f891115e8939`;
raw disk `a8aabdf2471cd0853685d0f9e11ac7cbacd0d7b110f6c17a03b4f93032f34094`.
One isolated visible trial; only own spin RIP1002D092/1002D094 plus
CSPMOT01 magic permits mailbox interpretation. The captured RIP is1002D094.

After actual capture, a requested one-pixel default-button SKY drag produces
packet1/errors0: dx+255,dy-255,buttons2 (right). After PC Y normalization,
the pointer correctly clips to511,341; window80,70,inputs0,dragging0,
down0/suppress0. Host right-primary preference was observed, not changed.
The packet supports an injection-path mismatch; one observation does not prove
every host cause, establish a decoder defect or qualify GUI dragging. A
coordinate click initially failed to capture; an accessibility canvas click
with a fresh UI confirmation captured successfully. No successful left drag.

Lead correction: the original brief incorrectly called r10 retained raw
header/X. The decoder resets them before PACKET returns. This word contains
cleared header/X state and retained last raw Y=1. Decoded deltas/buttons and
post-dispatch positions are valid. Synthetic nonzero parser-field test inputs
prove packing only. The code and native image stay unchanged; the SPEC and
brief appendix now describe the actual lifetime. No reconstructed header/X.

A pure-RAM shared-core replay matches all175104 logical pixels of the initial
and stopped final screenshots. A separate counter2/pattern0/corner-cursor
final-state construction also matches every pixel of the prior GUI snapshot.
The previously unclassified4x7 change is expected cursor restoration; no window
damage found. This is a same-implementation state comparison, not an independent
renderer oracle or reconstruction of the older input sequence. Initial image
analysis polarity was corrected to Mono1MSB bit1=black before accepting counts.
The original helper and logs remain outside Git at
`C:/ClasSICK/pc-first-motion-20261024-a/`; helper hash is in the JSON receipt.

Three [pure framebuffer screenshots](../evidence/screenshots/2026-10-24/README.md)
are indexed in Git (58 PNGs/two unchanged GIFs total). The VM is powered off
and unregistered; original inventory/running states and raw media roundtrip
match. No host-setting change. The diagnostic spin is not a semantic session
stop. No new GIF: this is a diagnostic packet, not a successful drag demo.

Next: verify the automated gesture's button and relative-motion behavior with
a bounded public input-interface check before another native trial. Keep E2
device tests distinct from GUI qualification. Mac motion remains independently
unqualified; this PC packet does not explain or fix the Mac loss behavior.
No release freeze/publication. Existing286 evidence and all gates persist.
