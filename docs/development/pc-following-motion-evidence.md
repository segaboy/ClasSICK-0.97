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


## Actual native observation (SRC-0433)

Source989f4535ad1712e4ea66016118a68c95ee171bb4; EFI184832B SHA256
83a70fecfe07bcc4615fce9583a7d0bee39099eae3852bddd07ee44f1432864e,
raw media83d28b9a6f6cfdf74fd9c17af4d36d14cf93b5d39f6f99436704024a7e693c54.
Four native twins/27 corruptions/two omitted-probe controls plus writer twins
and filesystem/payload verification pass. Original emitted observer checks
total packet count>=2; exact own spin1002D0A2/1002D0A4 andCSPMOT01 checked.

One cold boot. Initial43.3456813 host seconds,no spin. After capture,first
SKY gesture513,447 to514,447 leaves the pointer at511,341 but does not reach
the later-motion stop. Resume/recapture plus reverse gesture513,447 to512,447
reaches own RIP1002D0A4. Guarded words give packet3/errors0,dx255/dy-255,
right button2,cleared header/X,last rawY1;pointer511,341,window80,70,count0,
dragging0/down0/suppress0. Recapture occurs between gestures,so the exact
individual host sub-action responsible for packet3 is not isolated.

The later packet repeats the mismatch. Initial positioning alone does not
explain the full attempted sequence. Stop repeating this SKY captured-VM
route unchanged;it cannot currently qualify small relative left dragging.
This does not prove a decoder bug,physical behavior or universal host cause.
No input policy,button mapping,admission or normal image was changed.

All175104 logical pixels in each of three captures match the unchanged
shared-core initial/corner-cursor expected-state outputs from SRC-0431;no
window damage found. This comparison is not an independent render oracle.
Only own framebuffer PNGs are [indexed in Git](../evidence/screenshots/2026-10-24/README.md),
61PNG/two unchangedGIF total. An initial occluded UI capture is excluded.
No successful-drag GIF produced. No public host-app/desktop pixels.

VM powered off/unregistered;existing inventory/running states and raw media
roundtrip preserved. No host preference change. External diagnostic stop is
not semantic session shutdown. CI40098551352 native job passes;Windows job
is still running at this observation. No release qualification/freeze.

Next: choose a supported relative-input route or explicit human qualification
for E3,with E2 device controls kept distinct. Independent Mac motion work can
continue from its existing trace;this PC result does not solve Mac sampling.
No new emulator platform,installation,physical/reference run,release or
compatibility waiver is authorized by this diagnostic result.
