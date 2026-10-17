# ADR-0046 - One8042 owner and an explicit native mouse profile

Accepted2026-10-16; SRC-0233..0237 / SPEC-0040 / IMPL-0044 / TEST-0048.
Reuse the existing keyboard startup and pure READY byte router; one coordinator
owns subsequent I/O, source-specific command replies and live input. No competing
poller, keyboard FIFO mouse encoding, IRQ, firmware or universal hardware fallback.
Require the strict standard PS/2 profile; startup failures stop before drawing.
Recover after an explicitly detected mouse fault by taking an uncredited baseline.

`CLASSICK_PC_MOUSE=1` compiles the existing native producer with the new bus;
default0 keeps the tested keyboard-only contract. Both never link together.
Keep112 trace bytes; v2 identifies combined inputs and bus diagnostics. The
unchanged serial carrier's legacy `space=` label means combined input in v2.
Do not change UART implementation/test bodies or grow caller trace capacity.
Keep the UEFI ABI x64-only; pure bus code supports both widths/byte orders.
Mac sources/ROM inputs stay byte-identical. No new target run or release.
[Evidence](../development/pc-mouse-evidence.md).
