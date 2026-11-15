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
