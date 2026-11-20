<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0236 - Quick arithmetic, clear and test

SPEC-0145 / IMPL-0175. The hosted operand suite checks all widths, all eight
quick amounts, both operations and eight arithmetic boundary values with an
independent 64-bit oracle (384 combinations). Separate CLR/TST cases verify
flags and preserved data for both initial X values. An operations carry
through bit 16 and wrap through bit 32. Memory cases cover read-only TST,
read-before-write CLR, guard refusal and rollback, predecrement/postincrement,
the byte-size A7 stride and nine excluded encodings.

Four original defects (quick zero means 0 rather than 8, narrow An add,
TST writing, CLR skipping its memory read) each failed the expected checks.
All 30 hosted invocations in `scripts/Test-M68kStartup.py` and all nine native
address/stack regression cases passed with the combined decoder. The O3 and
ASan/UBSan bus runs include all SPEC-0142 cases. Full regression report SHA-256
`83d7755fa07d1805a7868bfc2b8b9c8b954567cb045915394e98bee3efb567f4`;
decoder SHA-256
`6dec13a53da91f0f4095a661d28708637a72a1c444fa35fffe8d275a66c3a431`.

The generated development builds separately ran disk-supplied instructions
through the normal Mac ROM startup and installed 286 boot path. The reported
eight-byte quick/unary result was `fffffe008000fff8` on both targets. The
selected-disk 286 diagnostic still stopped at guest PC 0x808E after two
successful steps. No private instruction identity was used to design or test
these families. Timing and physical hardware remain unqualified.
