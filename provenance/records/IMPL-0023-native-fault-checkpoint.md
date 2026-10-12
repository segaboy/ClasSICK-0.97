# IMPL-0023 - Native PC fault fixtures and terminal observation

2026-10-12, SPEC-0019 / SPEC-0008 v2 / ADR-0022 / SRC-0085, SRC-0043.
Codex original assembly/script implementation; lead self-review, human review
pending. Permitted inputs: project contracts and source, reviewed Intel published
CPU prose, VirtualBox published management interface and own output only.
No third-party implementation, Apple input, firmware extraction or copied asset.
UART implementation/specification/test bodies were neither edited nor reviewed;
existing module is used through its contract and aggregate checks only.

Changed native paths: `platform/uefi/transition.S`, `platform/pc/x64/exceptions.S`.
Completed records are exported into terminal registers without calls/devices.
Test-only preprocessor profiles select UD2 or out-of-GDT MOV DS; handler exits
use a separate halt label. Normal build contains no probe instruction symbol.
Image guards check bytes, relative destinations and profile separation.

New scripts: `Verify-FaultCheckpoint.ps1`, `FaultObservation.ps1`,
`Test-FaultObservation.ps1`, `Prepare-FaultTrial.ps1`, `Review-FaultVM.ps1`,
`Observe-FaultTrial.ps1`. Existing EFI wrappers gain explicit profile selection;
VM preparer gains a separately fingerprinted SPEC-0019 normal admission.
The host requires successful own completion before selected-register queries,
validates the frame against map RVAs/owned layout, then checks a second stable
observation. Only a newly prepared project UUID is mutated; no guest memory read.

All checks/trials and unresolved limitations are recorded in TEST-0027 and the
separate dated evidence. This checkpoint neither completes B2 criterion 6 nor
adds historical OS services, recovery, physical validation or an edition.
