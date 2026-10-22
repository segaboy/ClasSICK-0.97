# ADR-0071 - Bind the PC window through a separate gated entry and trace

Accepted2026-10-22;SPEC-0062;SRC-0356..0358;IMPL-0064/TEST-0073.
Use an explicit mutually exclusive WindowDrawing profile,existing owned bundle/
arena,and one160-byte CWP1 record at offset2048. Reuse the neutral session without
old drawing/UART entry behavior. Validate full ranges/aliases,real exit/readiness,
owned image/map/window and capability gates before arena/session effects. Keep
current-turn publication separate from last complete frame counts;no deadlines,
implicit repair/detach or new hardware ABI. Legacy linked units remain under the
current no-gc image policy;native entry closure is audited separately.

Adopt Claude's original355-line wrapper with narrow lead gate/clock/ACPI fixes.
One missing-header refusal gets one corrected call;no new worker fan-out. Lead
integrates frozen-API tests/parser/compiler/EFI/media checks. Tests are not a
strict final author-blind oracle after integration. Native2048 frame limits do
not apply to ASan-expanded diagnostic frames. Correct TEST-0072 keyboard coverage
explicitly;do not overwrite old source-pinned observations. Unbooted candidates
and E1 traces never qualify GUI/human/hardware,Mac parity or BIOS support.
[Actual evidence](../development/pc-window-boot-evidence.md).
