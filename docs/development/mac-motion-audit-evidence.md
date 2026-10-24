# Mac motion audit and controlled-input limits

2026-10-24,SRC-0434/0435 / SPEC-0072 / IMPL-0074 / TEST-0087.
One Copilot Sonnet5.5 low audit,37013B frozen reviewed-public packet,32.95s,
one read/zero writes,commands or tests. No supported defect or counterexample.
Lead adopts F1/F4 with changes,defers F2/F3;the JSON contains each disposition.
Changing sampled bits correctly yields UNSTABLE. No claim of a physical
double-axis edge,a particular live/latch view,or an established shared host
cause. An idle/slow success would not prove injection-rate causality.

Lead initially proposed removing RR0 selector writes,then checked the complete
function:it already skips them. That proposal was dropped before any code edit.
Runtime,strict sample admission,acknowledge order,spacing and recovery unchanged.

The current [publisher input guide](https://docs.snowemu.com/manual/input)
describes relative capture/release and speed controls. Installed Snow1.5.0-317c188
is separately observed:hardware-relative mode is selected,but its stopped
Input menu does not show those capture/speed controls. Do not apply current
online interface details to this version without verification. Absolute
memory-patching mode was not used;no emulator implementation was inspected.
Windows Settings/Control Panel exposed no controllable window. Original
right-primary flag1 remained unchanged. Help-only check yielded no output or
lingering Snow process;no CLI capability inferred. No tool install/upgrade.

## Actual unchanged-ROM comparison

Reused original sourceb3cb3fea29f510129b2c74f77ad5cd546d9fa119,65536B ROM
SHA256673a2cca57d1ec9ffd842f4c858fa4aafbcb042ec6f86445ae5717d4816c4965.
23096B used/304B BSS/964B graph/4996B conservative stack bound unchanged.
Current native inputs match the archived manifest;ROM hash/seal rechecked.
Prior twins/12 controls/three veneers reused,not counted as newly executed.
One isolated Early128K boot,no disks/sharing/PRAM/extension,normal speed.

Known RAM MWP1 address0xB70,128B/version2 and diagnostic0xBF0 magic are verified
before decoding. Pre-gesture control:VBL4601/SCC1/services4602,baseline1/loss1,
deliveries0,presses0,pointer256,171/window80,70,fault0. The toolbar pause may
have injected movement,so this is NOT a clean idle failure.

After resume and two explicitly selected clicks one host pixel apart,plus12s
settling:VBL20233/SCC3/services20236,baselines3/losses3,peak1/deliveries2,
presses1,pattern1,pointer256,171/window80,70,fault0. This confirms one button
press and its two delivered records,not displacement or a second accepted
press. Menu/pause/positioning inputs prevent attributing each loss to an
individual gesture. First-loss record remains UNSTABLE2,RR3=9,
AF4C4C/9F4444/9F4C4C,previous01000000,entry READY3. No sample admission fix
is justified. Direct IRQ and foreground fields are not timing measurements.

The unchanged shared-core button-state replay matches all175104 normalized
pixels of the [clean original framebuffer](../evidence/screenshots/2026-10-24/mac-motion-audit-button.png),
press1/pattern1/no movement. Source PNG uses34/238 gray levels;the original
file is preserved,normalization is analysis only. This is a same-core state
comparison,not an independent renderer oracle. Initial helper compilation
rejects misleading indentation;the helper is corrected before the actual run.
Captured pixel bytes also match the earlier ABI-corrected button screenshot;
this new trial has its own source/RAM/identity record.62PNG/two unchangedGIF.

Own Snow process closed;ROM and host primary preference preserved. No new
runtime/native build or feature-test suite,emulator platform or release.
Source CI40098551352 now passes BOTH jobs,settling the prior pending entry.
Mac motion and PC GUI drag remain open;286 evidence retained unchanged.
All source/role/edition/physical/128K/B2/System1/native-app gates persist.

Next useful development slice:an observation-only automatically frozen idle
checkpoint before mouse-driven pause,with an explicit memory/ABI budget and
native validation. No such checkpoint is implemented by this audit. Do not
repeat this same comparison unchanged or weaken loss checks to obtain a pass.
One reported Copilot premium request;all-four remaining balances unavailable.
No routine second worker or measured Codex-quota-saving claim.
[Full receipt](mac-motion-audit-evidence.json).
