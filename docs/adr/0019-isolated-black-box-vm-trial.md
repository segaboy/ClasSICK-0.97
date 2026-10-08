# ADR 0019: Qualify one isolated VM with owned boot media

Accepted scoped design 2026-10-08. SRC-0060 / SPEC-0015; SRC-0059's black-box
firmware decision remains binding. Human provenance review pending.

The owner approved the first trial after original media passed independent
checks. Use one fresh project machine with the declared EFI64/PIIX3/PS2/UART
profile and a round-trip-verified VDI of SPEC-0014. Existing owner VMs are not
test fixtures. Explicit settings and retained argv/configuration bind observations
to the machine; public CLI documentation supplies the management interface.

Start headless for screen/serial qualification and independent timing. A visible
operator session is needed for real keyboard acceptance; injected scan codes
remain synthetic. Cold-start attempts retain separate evidence. Do not infer
exit/ownership, native device conformance or B2 from a screenshot or elapsed host
time alone. Firmware-side failures remain black-box observations.

No external firmware internals or third-party implementation is a debugging
input. The VirtualBox platform remains uncertified; no Apple material is adopted.
No guest/core/driver code changes are part of this configuration contract.
All historical parity, Macintosh 128K/replacement startup, real Mac/mini vMac
and three-edition gates remain independently required.
