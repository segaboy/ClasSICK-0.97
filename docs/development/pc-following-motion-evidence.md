# Following native PC motion packet

2026-10-24,SRC-0432 / SPEC-0073 / IMPL-0075 / TEST-0088.
Baselinea987881. First captured packet255/-255/right-button2 may include
initial positioning. A later packet has not yet been measured; no decoder
change or general host-cause conclusion follows from that first observation.

Supported tool-interface review: clicks select a button; drag exposes
window-relative endpoints, with no button/relative-delta selector. No tool
implementation or emulator source inspected. Do not invent unsupported calls.

The explicit motion-later build sets MOTION=1 and SKIP_FIRST_PACKET=1. It
ignores total decoded-packet counts0/1, then captures the first nonzero PACKET
whose total count>=2, after unchanged dispatch. This is not necessarily the
second motion packet. Existing magic/word schema/backend remain; no mutable
history, extra IO or runtime input-policy change. Header/X remain post-reset
state, only last rawY retained. Default loss/motion semantics stay unchanged.

One Copilot Sonnet5.5 low primary returns12 exact replacements in6files in
55.23s, one frozen-public read/no writes/commands/tests. Lead adopts them,
executes verification and records results; no routine second reviewer. Author
tests are not an independent implementation review. Reported one premium
request; remaining all-four provider allowances unavailable.

Actual13 legacy/101 motion/87 later checks pass in Clang Debug/GCC Release/
ASan-UBSan (201 per profile,three CTests). Three compiled faulty controls
(too-early,too-late,swapped axes) fail;four invalid macro combinations refuse;
two motion-later-without-probe profiles and one inconsistent package refuse
before output/media write. These cases executed; worker proposals are not
counted. Parser-field fixtures verify packing, not full reachable owner state.

Normal native image still byte-identical SHA256
de02e416fc92bc69fb4e07e41eb8a79a2f1cd00493e12104e1e47dba55e55d0f,
four twins/27 corruption controls. Committed-source motion-later native build
and target trial pending. Mac motion and PC GUI dragging remain unqualified;
no release freeze/publication or new GIF. Prior286 evidence/gates/originals
preserved. [Machine-readable receipt](pc-following-motion-evidence.json).
