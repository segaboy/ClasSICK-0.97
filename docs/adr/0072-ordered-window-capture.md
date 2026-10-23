# ADR-0072 - Preserve ordered records before hardware admission

2026-10-22;accepted engineering choice;SPEC-0063,SRC-0366..0368.
Use a16-record,caller-owned sequential helper at the native input-owner boundary.
Preserve each normalized sample because clamped movement is nonlinear. Visible
overflow/uncertainty blocks admission;foreground discards stale prefix,cancels,
then rebaselines with actual buttons. Held left stays suppressed until release.
Epoch tokens refuse stale sessions and restart refuses wrap. Consume a record
before its one owner call;retain fault prefix,never replay or repair implicitly.
Stop ends admission before cancellation and remains ended on cancel failure.
Fresh begin/preflight and explicit restart are separate lifetime operations.

The fixed layout is <=256B;native-frame goal512B,measured/checked separately.
No dynamic queue,displacement coalescing,second compositor or hardware/history
assumption. This is not an IRQ-safe publication mechanism;MC68000 masks/critical
sections and SCC/VIA ownership/polarity/timing remain later admission contracts.
Native68000/x86-64 code parity is preserved;boot graphs/releases remain untouched.
[Executed checks](../development/window-capture-evidence.md).
