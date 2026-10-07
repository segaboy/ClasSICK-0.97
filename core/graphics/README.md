# Portable graphics

First implemented subsystem: bounded surfaces from
[SPEC-0001 v1](../../docs/specifications/SPEC-0001-surfaces.md), using caller-owned
storage with MSB-first 1bpp and explicit RGBA8 bytes. `surface.h` exposes construction,
clear and clipped rectangle fill. Headless conformance: TEST-0004.
It defines our generic core contract, not historical QuickDraw semantics.
Presentation, guest decoding and hardware memory mapping belong to other layers.
