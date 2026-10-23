# TEST-0087 - Injected Mac chip model

2382 checks and32 frozen-table phase/inversion cases pass against injected register IO and the real inbox. Includes unstable baseline, pending mismatch, overflow/recovery, button edges, stale epochs, unsupported status, marker/alias faults, stop/no-effect refusals. Synthetic verification only; native assembly/ROM/trial pending. Initial harness expected OK from resume; fixed to its documented BASELINE return without production change.

[Combined evidence](../../docs/development/release2-integration-evidence.md).


2026-10-23 follow-up: original DCD-mask readback replaces unused baud-register
markers.4549 checks/32 existing phase cases; bounded read/write transaction
oracle proves both masks set before either read, B disabled through readback,
both enabled before arm and MIE off. A/B wrong-read and aliased-control refusals
retain master-off first causes. Three actually compiled faulty controls rejected.
Local GCC/Clang/sanitized checks pass; native/target results separate.
[Follow-up](../../docs/development/boot-input-followup.md).


SRC-0424:5750 checks/32 phase cases, three local profiles and four compiled faulty original controls verified for first-loss ordering, retention, reset, reasons and hard-fault exclusion. Native/target verification separate. [Receipt](../../docs/development/mac-first-loss-evidence.md).
