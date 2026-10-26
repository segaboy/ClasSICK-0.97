# Separate input-model candidate and286 compiler progress

2026-10-26;SPEC-0074/SRC-0479..0480/IMPL-0082/TEST-0095.

One supervised worker supplies an original pure phase decoder and author tests.
Lead adopts the implementation unchanged;the header's C11 assertion is corrected
for C99,and one PB-only Y expectation is corrected with an actual reverse-edge
case added. begin's successful return is explicitly EMPTY. Original C99 compile
and author failures are recorded;the original decoder already passed the frozen
oracle. No hardware driver change is implied by this model's success.

The independent literal transition oracle covers4096 axis/button/inversion
cases,40475 assertions per profile. Author/oracle suites and two integrated
CTests pass in Clang Debug,GCC64 Release,GCC32 andClang ASan/UBSan. Three compiled
bad controls (accept two-bit jump,emit PB-only movement,retain phase after loss)
are detected. The oracle's redundant unspecified-return check is removed before
worker inspection;later only line breaks change for GCC's indentation warning.
Frozen artifacts/hashes and failed logs remain outside Git;expected values did
not change after seeing the implementation.

The model has40-byte state/12-byte output,zero retained pointers,mutable globals
or source-level library calls. It is not linked to a ROM or PC boot path. An
isolated seven-object68000 audit is wired for O0/O2/Os twins,layout,imports,
mutable-data and frame checks;actual native result remains pending source CI.

286 progress:the pinned WCC16 compiler produces six helper-free80286/USE16
objects across small/medium/large models and unoptimized/size settings;three
named bad-size controls reject. The old C16 facade's nine-call/six-object/three-
refusal regression also passes. Objects are unlinked and unexecuted. The flat
numerical span premise is not a segmented-pointer backend;the-s diagnostic
stack-check suppression is not a production waiver. No speed/ISA-completeness,
native286 app or hardware claim follows.

The next binding decision needs validated live/coherent phase sampling,a latency
bound and cancellation/baseline integration. Endpoint Gray validation cannot
detect missed full/three-step cycles. No SCC settings are guessed here,and no
existing admission check is weakened. Release motion/dragging and human-use
failures remain open. No desktop interaction or target run occurs in this slice;
68PNG/two priorGIF and all originals/old evidence remain.

Copilot Sonnet5.5 Low:54.20s,9567 frozen reviewed-public/original bytes,one permitted
view,zero edits/commands/tests,254 artifact lines. Lead integrates usable output
and runs verification;no second worker or provider retry. One reported premium
request;all-four remaining balances unavailable,no measured Codex saving claim.
[Contract](../specifications/SPEC-0074-polled-phase-model.md),[receipt](mac-polled-phase-model.json).
