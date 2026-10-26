# Timer polling controller and native binding

2026-10-26;SPEC-0075/SRC-0482..0484/IMPL-0083/TEST-0096.

An original separate controller now connects the verified phase model to the
ordered inbox and an opt-in native Mac entry. SCC master and DCD latching remain
off;VIA T1 requests bounded sampling. The normal strict driver is unchanged.
This tests a concrete alternate input path;it does not claim the blocker fixed.

Copilot supplied37 header/244 implementation/285 author-test lines in205.55s,
from20130 frozen bytes,one permitted view,no commands/writes/tests. Lead adopts
the controller with reached-stage/peak diagnostics and nonzero failure-cause
corrections,adds the native binding/build controls and integrates the artifacts.
Original author strict build fails eight byte conversions;permissive build runs
but fails two existing inbox result expectations. Those are corrected. The
original controller already passes the independently frozen88-assertion oracle.

Final author92 and independent88 assertions pass in Clang Debug,GCC64 Release,
GCC32 andClang ASan/UBSan;two integrated CTests per profile pass. Three actually
compiled bad controls (enable SCC master,omit timer recurrence,accept unstable
pair) are detected. The independent oracle's old inbox resume expectation was
corrected before worker output inspection;its original frozen version remains.
No expected value changes after controller inspection. An initial private
mutation-generator match-count refusal is corrected to the unique reset context;
it is not counted as detection of a compiled defect.

Published Synertek SRC-0078 PDF3..6 (printed4..11) supplies only T1/ACR/IFR/IER
interface diagrams/prose. No sample code is used. The reg7-write/compare wording
is ambiguous and excluded;no clock rate or reload formula is inferred. The known
IER read erratum is avoided. Zilog SRC-0363 printed177/PDF184 explicitly describes
unlatched DCD when its WR15 enable is clear;the matching file identity and page
image are rechecked. Manufacturer ISR recipes/listings are excluded. All manual
pages stay lead-only,outside Git;the worker receives paraphrased reviewed facts.

Native normal/timer/timer-idle build,exact default-ROM equality,cost/seal/source
identity and checkpoint audit remain pending committed-source CI. No emulator,
VM,reference,physical or desktop input occurs.68PNG/twoGIF and all originals are
preserved. Prior286 six C16 phase objects/refusals and facade regression remain;
this Mac-only controller does not add a286 hardware backend. Mac motion,PC GUI
dragging,human-use failures and paired release approval remain open.

One reported Copilot premium request;all-four remaining balances unavailable.
No second worker,retry,new chat,Codex subagent,OneNote/wiki or publication.
[Contract](../specifications/SPEC-0075-mac-timer-poll.md),[receipt](mac-timer-poll.json).

First native attempt768141f/CI40374370882 passes Linux host suites and normal ROM build,but timer link fails on implicit memset from the compound-literal initialization. Lead switches to the readonly-zero assignment form already verified in the original motion driver;no libc routine is introduced. Corrected native result remains pending.
