<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0143 - Native register traps in the disk-startup ROM

2026-11-20. IMPL-0173 / TEST-0234. Adopt SPEC-0017's published register
convention (*Inside Macintosh* I-94–95, II-383–384), SPEC-0020's directory
and SPEC-0021's MC68000 ordinary exception frame (SRC-0088–0090, SRC-0093).
This profile does not settle edition differences or enumerate every ROM trap.

Once the startup disk is loaded, reuse its native read workspace for a
512-entry directory. Initialize Get and Set to their actual callable ROM
routine addresses. The supported Get words are A146 and A046. Set is A047.
D0 low word supplies an index from 0 to 511; A0 supplies Set's new target.
Any other uninstalled OS service, invalid input, unsupported mode or Toolbox
trap stops in the existing terminal ROM handler. A guarded trap with a RAM
replacement admits only register-based OS words A0xx/A1xx under this profile.
Patch pointers use the same configured 64 KiB RAM window as the 286 profile.

Vector 10 takes an actual MC68000 supervisor frame. Read the faulting word at
its even saved PC, select the target before running it, increment the frame's
PC by two, then call that target with a native return address on the stack.
The handler may execute more traps or JSR to a previously saved Get/Set or RAM
routine. The mediated return restores D1/D2/A1, restores A0 for bit 8 clear,
and supplies NZVC from TST.W D0, preserving the caller's X. D3-D7/A2-A6 are
callee-saved. RTE restores the original stack pointer and saved SR/advanced
PC. The named Get/Set entries also admit direct JSR in this supervisor-mode
profile; they preserve CCR and other registers except for Get's A0 result.
The low-memory trap table layout, Toolbox, interrupts and system-error UI are
outside this contract.
