# ADR 0016: Poll the PS/2 keyboard inside an owned interactive loop

Accepted scoped design 2026-10-08. Owner SRC-0048; SPEC-0012 / IMPL-0016 /
TEST-0018; human provenance review pending.

B2 requires real post-exit input affecting the scene. Require an affirmative
ACPI controller indication, then own configuration/reset/set-2 startup with
one bounded polling operation per timer turn. Disable translation, both device
interrupts and the auxiliary interface; isolate byte port instructions in x64
assembly. Use the existing core FIFO for normalized Space/Escape events.

Add a separate interactive successor to preserve SPEC-0011 regression coverage.
Input adjusts the original progress scene independently of the timed progress;
Escape visibly changes it and stops early, explicitly without B2 acceptance.
Use the remaining arena headroom and trace offset 128; no bundle enlargement.
No USB, firmware input, interrupts or full text keyboard in this checkpoint.

Polling permits independent transcript tests without importing another OS's
driver. Manufacturer interface documentation does not qualify a VM device.
Native hardware must maintain A20/mappings through self-test; released keys and
exclusive controller ownership are startup prerequisites. Uncertain firmware
remains ineligible and all edition boots stay open.
