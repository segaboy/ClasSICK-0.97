# ADR-0063 - Retain accepted input and recover its unfinished prefix

Accepted2026-10-21;SPEC-0055;SRC-0329..0331.
A synchronous exclusive owner publishes each pointer event once to counter,
window and compositor. A later refusal retains accepted state and the last
successful screen snapshot,stops new input and reports the completed prefix.
Explicit repair finishes only undelivered consumers,cancels the gesture and
fully repaints. It preserves the count and content toggle rather than replaying
accepted presses. No allocation,callback,new framebuffer or platform binding.
The caller may explicitly retire/rebind an exhausted cursor before repair;
bindings/revisions never wrap. [Contract](../specifications/SPEC-0055-window-input-owner.md).
