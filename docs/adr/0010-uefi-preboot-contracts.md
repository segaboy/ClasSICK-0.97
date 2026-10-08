# ADR-0010 — Validate preboot data before native firmware calls

- Date: 2026-10-08
- Status: accepted for SPEC-0006 and hosted validation; VM/firmware profile proposed
- Sources: owner SRC-0033, UEFI SRC-0034, VirtualBox SRC-0035, ACPI SRC-0036

B1 and current-core link/compiler checkpoints pass. B2 requires exact native
firmware/loader ownership, not just a UEFI application drawing before exit.
Start with independently implemented bounded framebuffer/map/owned-span validation
and a finite exit-transaction model. Keep physical values distinct from native
pointers, external byte layouts explicit and core modules unchanged. Hosted tests
prove these contracts before actual firmware calls or device operations.

Use our own minimal x64 UEFI declarations and PE application entry when the loader
is implemented; no GNU-EFI/EDK2 library or example implementation is adopted.
Keep the final map immutable and prohibit allocations/logging/protocol operations
between its capture and exit. After a stale-key failure, permit only recapture
into the preallocated buffer and another exit attempt; halt after three attempts.
This is stricter than the UEFI retry allowance and avoids partial-shutdown calls.

Propose one isolated VirtualBox 7.2.16 PC profile with one CPU, EFI64, PIIX3,
ACPI, direct GOP framebuffer, polling PS/2 keyboard, ACPI PM timer and bounded
16550 diagnostics. Hardware initialization/ISA/register specs remain leaf work
before those drivers are implemented. See [the profile](../development/native-pc-profile.md).
No VM is created, existing configuration queried or firmware executed in this step.

Firmware is not yet adopted: Oracle's license inventory lists Apple-attributed
portions in an OVMF/Bhyve component. Exact selected x64 binary/source closure is
unresolved; that notice does not prove they are in the installed image. Official
tag/module/packaging metadata was inspected only for eligibility, not used as
implementation. The inspected VBoxPkg.dsc is ARM/AARCH64 and cannot validate x64.
No firmware binary/source implementation was fetched, extracted or inspected.
No blanket dependency approval follows from a permissive license or product name.

Alternative: implement the complete loader before validating descriptors. This
would leave malformed-map and partial-exit failures entangled with hardware startup.
Using firmware callbacks after handoff would fail B2. A BIOS path is a possible
future alternative if reviewed UEFI provenance cannot be established; it would
need its own startup contract and an explicit updated gate, not silent substitution.

Revalidate when profile bounds, UEFI revision/layout, compiler/ABI, ownership or
device selection changes. Human provenance review, firmware eligibility, exact
boot-media recipe, native entry/stack/exceptions, device drivers and B2 remain open.
Real Macintosh/mini vMac and physical three-edition gates are unchanged.
