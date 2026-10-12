# ADR-0022 - Observe terminal faults through owned register exports

Accepted 2026-10-12; SRC-0085 / SPEC-0019. The existing fault handler records
and stops but has not been observed handling a native CPU fault. Calling a
driver during fatal handling adds dependencies and violates its terminal policy.
Reading arbitrary guest memory would broaden our black-box boundary.

Export the finished original record into selected registers and inspect them
through the cleared emulator's published management interface only after own
successful output. Build two separate injection fixtures and retain a normal
control. Route all handler exits to an independent halt label to prevent
re-triggering the fixture. UART and shared core source stay unchanged.

This proves only the two named deliveries and owned-stack behavior on the exact
declared platform. It does not certify firmware provenance, all faults, recovery,
physical hardware or an operating-system edition.
