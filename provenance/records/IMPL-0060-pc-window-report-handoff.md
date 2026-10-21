# IMPL-0060 / TEST-0068 - PC window report handoff

2026-10-21;SRC-0336/0337;SPEC-0057;ADR-0065.
Original stateless native C validates complete decoded report ranges,converts
Y under an explicit engineering profile,preserves button bits and calls the
shared owner exactly once. Owner results/retained faults pass through unchanged.
No I/O,counter,decoder or allocation is added. Existing drivers/cores/boot
workloads remain unchanged.

316-test Clang64 regression run passed;focused final checks also cover Clang64,
ASan/UBSan and GCC32. Report suite:3938 checks,148 complete-frame comparisons and
46 decoded reports per run;stub boundary suite:196 checks/49 single deliveries.
Original raw-byte/logical-sample pairs check partial reports,held/release/content,
drag/clamp/final motion,all button bits,loss during drag,suppression/release,
saturated counters and invalid/refused input. Existing renderer is reused in the
whole-frame comparison;no new independent renderer or device-direction claim.
Three compiled faulty adapters are rejected.12 width/endian/EFI-ABI object audits
pass,including delayed full-file-identical twins and no mutable global/imports
beyond cs_window_input_sample. Windows and68000 regression CI pass at983823a.
[CI](https://github.com/segaboy/ClasSICK-0.97/actions/runs/39646760468). New PC adapter is not bound to either boot image.

Test-only corrections:RGBA enum spelling;right edge expected288=512-224;
saturation fixture must update its actual rendered snapshot as well as count.
The final saturation assertion requires an accepted down/drag at UINT32_MAX,
so matching refusals cannot masquerade as a pass.

Direct Codex implementation/test/integration;no external worker calls,retries or
qualification overhead. Existing qualifications reused;all-four remaining
allowances unavailable,no quota-saving measurement. Original GPL-3.0-or-later
code/fixtures only,no new manual,third-party implementation or research input.

The future bus-to-UI boundary needs explicit per-poll packet/loss delivery:
saturating totals cannot identify freshness. No new controller poller,boot/guest
binding,emulator/VM/hardware/reference run,media or release. Mac M1-M7,PC GUI/human
and all existing source/role/UART/historical/edition/physical/full128K/B2/native-app/
Linux/mini-vMac/both System1-ROM gates persist.24 research originals,held brief,
old rows/assets/tags/retired wiki preserved. [Receipt](../../docs/development/pc-window-handoff-evidence.json).
