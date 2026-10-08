# ADR 0014: ACPI PM timer as the first owned native time source

- Accepted scoped original design, 2026-10-08; owner SRC-0045 continuation.
- SPEC-0010 / IMPL-0014 / TEST-0016; ACPI 6.6 SRC-0036/0046, UEFI SRC-0038.
  Human provenance review pending. Builds on ADR-0010–0013.

## Context

B2 needs visible progress for at least 60 seconds from an owned timing source,
with no firmware timer or Stall service after exit. The proposed VirtualBox
profile enables ACPI. After exit, the RSDP can only be found through the UEFI
configuration table, so its address must be captured before ExitBootServices.

## Decision

Capture only the ACPI 2.0+ RSDP address before exit, as a plain handoff value.
After exit, walk the RSDP, XSDT and FADT through a reader that refuses any range
outside one firmware-reserved/ACPI map descriptor. Accept only I/O-space 24- or
32-bit PM timers. Extend the counter in portable C, convert it with helper-free
long division, and isolate the single `in` instruction in x64 assembly. The
first native use is a bounded probe recorded in owned trace storage; the
cooperative 60-second loop is the next step.

## Alternatives

TSC, HPET and the local APIC timer need frequency calibration or MMIO mapping
contracts that are not specified yet. The PIT is not part of the reviewed sources.
Legacy RSDP scanning of BIOS areas is not guaranteed on UEFI systems. A memory-space
PM timer is valid ACPI but needs a separate MMIO and mapping review.

## Consequences

The loader hosted-test fingerprints change, because the handoff gains a field.
Real firmware placement of tables, counter behavior in a VM, and wall-clock
agreement remain unobserved until a B2 attempt.
