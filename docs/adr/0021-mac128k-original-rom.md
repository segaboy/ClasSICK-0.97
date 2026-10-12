# ADR-0021 - Original 68000 ROM and Snow first-boot profile

Accepted for development/owner-directed trial, 2026-10-11; SRC-0076,
SPEC-0018, IMPL-0022, TEST-0026. No System 1 parity or M0.3 closure.

The Windows LLVM-MinGW package has no M68k backend. Use pinned Ubuntu GCC
13.3.0 cross tools and binutils 2.42, relocated outside Git into the workstation
workspace and invoked through its existing Ubuntu 24.04 WSL environment. Only
compiler/header/tool dependencies are used. Build with -mcpu=68000, explicit
68000 assembler enforcement, freestanding/no-builtin/no-PIC flags and -nostdlib.
The linker places reset vectors/code at 0x400000, own data/BSS at 0xB00, and
produces a sealed 64-KiB raw ROM. Implement the needed runtime helpers ourselves;
do not link or inspect the target libgcc implementation. Package hashes and
host library identities are recorded separately under SRC-0081.

This avoids adding an unreviewed Windows cross-compiler distribution or
depending on a backend the installed LLVM package does not contain. The earlier
GCC size measurement is indicative; this image gets its own measured map and
stack report. A 64-KiB demonstrator envelope does not resolve the complete OS's
128K feasibility budget, which the owner deferred rather than removed.

Choose Snow v1.5.0's Windows x86-64 executable for black-box testing only
(SRC-0077/0082). Its user documentation describes hardware emulation without ROM
hooks and a ROM-validation override, giving a simpler candidate than Mini vMac's
documented loaded-ROM changes (SRC-0074/0075). Exact behavior remains a trial
result; MIT metadata is not a provenance certification. Never read emulator
source or use an Apple ROM. No fork is needed or authorized by this ADR.

Use the actual 128K model, normal speed, no disks/network/reference images, no
persistent PRAM, and only our sealed image. Isolate the child app's settings
storage and the run folder. The owner's "OK, do it!" followed the explicit plan
to build the replacement ROM and test it in Snow; ordinary preparation and that
scoped trial are authorized. Any changed platform or physical writing remains
separate. Real Macintosh and later Mini vMac validation remain future evidence.

Review this choice if the own-image check fails, Snow rejects the image despite
its documented override, measured startup needs unsupported hardware behavior,
or the complete OS reaches a capacity limit. Record failures without reading
emulator patch code or Apple internals. Human provenance review remains open.
