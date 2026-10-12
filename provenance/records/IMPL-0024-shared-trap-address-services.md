# IMPL-0024 - Shared trap-address services

2026-10-12, SPEC-0020 / ADR-0023 / SRC-0088-0090 / TEST-0028.
Codex original C, probe and tooling, lead self-review; human provenance review
pending. Permitted inputs: project contracts/code, published interface facts,
Motorola TST prose. No Apple implementation, equates, caller samples, compressed
table algorithm, emulator source/assets or uncertain discovery excerpt used.
The coordinator did not inspect or change UART implementation/test bodies.

`core/services/traps.c/.h`: flat native u32 directory, configuration and logical
address validation, complete A-word decoder, checked address services, explicit
format/flags/unsupported/needs-execution results. Intrinsics are selected by
symbolic tokens; no guest-address-to-host-pointer or callback conversion.
`apps/service-probe/probe.c/.h` tests ten operations through that boundary.
`tests/services.c` supplies independent exhaustive/boundary/frame/state fixtures.

PC entry uses a newly named 4-KiB owned loader-tail slice. A tiny original
assembly helper exposes its probe result after normal loop completion. Failed
descriptor construction now actually halts before service/device access, as
SPEC-0008 already requires. Existing no-call exception handling remains intact.
Mac entry uses static BSS below 0x2000, appends diagnostics, retains existing
TRAP0, 8-KiB stack, VIA/scene paths and original helper runtime. ROM image audit
checks the expanded bound and source closure; EFI audit requires service symbols,
exact observer bytes and additional omitted/corruption rejects. General A-line
CPU entry, guest patch execution and full historical services remain later work.
