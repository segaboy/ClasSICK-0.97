# PC platform

First native platform target is a named x86-64 UEFI configuration. This layer owns
post-handoff framebuffer/input/timing and devices; see
[native boot gates](../../docs/architecture/boot.md).

`framebuffer.c` (SPEC-0009) validates a linear 32-bit RGB/BGR-reserved target and
converts clipped core RGBA8 rows with volatile byte stores, a zero reserved byte
and no framebuffer reads. It is portable C, exercised by hosted tests on synthetic
buffers and linked into the unloaded EFI image. `x64/` holds owned exception state.
`acpi.c` / `pmtimer.c` (SPEC-0010) walk RSDP/XSDT/FADT through a bounded reader
and extend the 24/32-bit PM timer; `x64/io.S` holds the single port read. No native
execution, keyboard or diagnostics driver exists yet.
