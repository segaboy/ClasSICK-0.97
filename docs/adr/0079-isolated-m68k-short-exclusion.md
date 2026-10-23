# ADR-0079 - Isolated supervisor short exclusion

2026-10-23; accepted original engineering, SRC-0394..0398 / SPEC-0068.

SPEC-0066 needs an actual compiler/CPU exclusion primitive before a later Mac
capture binding. Choose two stateless GNU C callbacks: snapshot SR then raise
IPL to7, and restore the saved word. Require supervisor state, trace off, exact
paired tokens, a levels1..6 producer and no nonparticipant touching the inbox.
Owner and rendering remain outside the section. Level7, exceptions and DMA are
outside this exclusion proof. No constant global enable, whole-render mask,
SCC acknowledge command, vector, handler entry or RTE is selected.

Adopt the fresh worker's original code and bounded compiler audit after lead
review. Native object checks and a host SR model have different scopes. The
source is isolated and changes no boot input. The paired release remains primary;
the independent C16 assertion/chunk/offset facade advances the bounded286 lane.
No target trial, helper runtime, production core adaptation or release follows.
