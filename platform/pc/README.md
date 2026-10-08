# PC platform

First native platform target is a named x86-64 UEFI configuration. This layer owns
post-handoff framebuffer/input/timing and devices; see
[native boot gates](../../docs/architecture/boot.md).

`framebuffer.c` (SPEC-0009) validates a linear 32-bit RGB/BGR-reserved target and
converts clipped core RGBA8 rows with volatile byte stores, a zero reserved byte
and no framebuffer reads. It is portable C, exercised by hosted tests on synthetic
buffers and linked into the unloaded EFI image. `x64/` holds owned exception state.
No native execution, timer, keyboard or diagnostics driver exists yet.
