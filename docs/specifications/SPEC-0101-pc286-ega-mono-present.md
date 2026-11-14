<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0101 - 286 monochrome plane publication

2026-11-14; original target adapter contract derived from the owner's
[Mac/286 paired direction](../development/mac-286-paired-direction-20261114.md)
and SPEC-0059. Status: experimental implementation and emulator tests; human
review, hardware admission, production boot and release qualification remain.

The caller supplies two stable, valid, nonoverlapping buffers: a 21,888-byte
512x342 packed MSB-first canvas (one means black), and a 28,000-byte 640x350
packed plane (one means white). The latter can be a far pointer in the pinned
16-bit profile. The caller owns display-mode initialization, plane setup,
buffer validity, concurrency, and any recovery after a hardware fault.

`cs_pc286_ega_publish` refuses null pointers without writes. Otherwise it clips
the half-open input rectangle to the logical canvas; an empty result succeeds
without writes. It copies every byte touched by the clipped rectangle, rounded
out to byte boundaries, to `(64,4)` on the target, inverting bits. Bytes
outside those row spans, including all margins, are unchanged. Rounding means
neighboring pixels in a touched byte are also republished from the canvas.
The caller must provide a complete, coherent canvas for those bytes. The
function does not allocate, initialize a display, claim physical EGA timing,
or promise fault-atomic I/O. Coordinates use `int`; all 16-bit inputs and the
host's larger integer range are accepted and clipped before arithmetic.

Implementation, source identities, independent host and BootHush checks, and
limitations are in [the shared-window evidence](../development/mac-pc286-shared-parity-20261114.md).
