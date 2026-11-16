# TEST-0189 - Windows 68000 optimization comparison

2026-11-14; SPEC-0072 / SPEC-0096 / SRC-0609. Two original window-ROM twins
per compiler option passed build audits. The `-Os` and experimental `-O2`
images passed the same slower BootHush Mac/286 state, pixel and replay checks;
both failed the faster input schedule, with different observed omissions.
These are bounded model tests, not physical performance or release readiness.
[Evidence](../../docs/development/windows-m68k-toolchain-20261114.md).

Five-million-cycle startup sampling and a separate stop-on-stage run observed
the `-O2` profile reaching its ready stage earlier in this model; the latter
is an instruction-boundary observation, not a physical-performance claim.

The later modeled VBL-guard comparison kept `-Os` and tried four guard values
at the faster input schedule; all failed at the first movement checkpoint.
The slower schedule passed with guard zero. Exact conditions and retained
failure records are in the same evidence; no physical speed claim follows.

2026-11-15: a scratch experiment coalescing diagnostic copies passed the
slower paired scenario but still failed the faster one. The experiment was
not adopted; its retained source, builds and outcomes are in the evidence.
Another scratch experiment skipped repeated empty foreground polling until a
new producer service. It too passed the slower scenario and failed the faster
one; the source was removed after recording the retained result.

2026-11-16: SPEC-0097 checked arithmetic/helper inlining passes 70,242 inbox
checks (including a new 6,561-case wide-arithmetic oracle), 30 focused CTests,
native ROM twins and the slower eight-state Mac/286 full-pixel/replay comparison.
BootHush's new bounded mask/instruction witness shows shorter measured SCC
spans; the fast paired case still fails. Two analyzer tests reject incomplete
and inconsistent trace evidence. Five other compiler/source experiments were
preserved and not adopted. [Canonical identities, commands, results and limits](../../docs/development/windows-m68k-toolchain-20261114.md#interrupt-mask-and-instruction-cost-investigation-2026-11-16).

The source-type guard also passes three isolated-index controls: original
`.asm` source is accepted, NUL data and executable extensions are rejected,
and the real index remains unchanged. This restores validation of the existing
original 286 assembly without admitting binary payloads; see the same receipt.

2026-11-16 follow-up: zero-operand refinement passes five rebuilt inbox tests,
native Mac twins, traced/plain comparisons and the slower eight-state paired
full-pixel/replay test on the new BootHush build. Fast +X still fails.
[Canonical receipt](../../docs/development/input-followup-20261116.md) owns
identities, commands, costs and limits; no release gate is closed.
