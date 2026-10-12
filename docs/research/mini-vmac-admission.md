# Mini vMac independent-ROM admission review

2026-10-11, Codex. SRC-0073 authorizes preparation for the first Mini vMac
boot; SRC-0074 supplies publisher documentation and SRC-0075 records this
review. **Admission pending: no emulator has been downloaded, cleared or run.**
This is documentation review, not an observed boot failure.

## Publisher-documented behavior

The [stable download page](https://www.gryphel.com/c/minivmac/dnld_std.html)
lists **36.04**. Its standard applications emulate a Macintosh Plus.

The [hardware reference](https://www.gryphel.com/c/minivmac/hardware.html),
sections ROM and Floppy Drives, says the emulator modifies the loaded ROM to
replace the disk driver and disable the guest checksum test. A checksum mismatch
produces a warning and emulation continues. It searches the application folder
first for `vMac.ROM`, then other locations, including a central Windows ROM
folder. The stated Plus image length is 131,072 bytes. Therefore a separate
application folder alone does not establish exclusive loading of our image.

The [options reference](https://www.gryphel.com/c/minivmac/options.html),
sections Model, Memory, ROM Size, Check ROM Checksum, Disable Rom Check,
Disable Ram Test and Minimum Extensions, documents model `128K`, RAM `128K`,
ROM size exponent `16` (65,536 bytes), and switches `-chr 0`, `-drc 0`,
`-drt 0`. These disable the host checks and two described guest-test patches.
They do **not** document removal of the replacement disk driver. Minimum
Extensions retains support for that driver. These are build settings, not
runtime command-line switches. No documented all-patches-off setting was found
in the reviewed user/developer option pages; this is not proof that none exists.

## Candidate to request, not an approved platform

Prefer a **36.04 Windows x86-64 Macintosh 128K variation**, with 128 KiB RAM,
64 KiB ROM, 512 by 342 monochrome screen, default 68000 CPU, 1x speed, no
background execution, AutoSlow disabled, and the three switches above. This
matches the hardware addressed by SRC-0071 and avoids silently adopting Plus
hardware. A larger image can be considered later under the owner's size deferral.

The [variation service directions](https://www.gryphel.com/c/minivmac/var_dir.html)
describe publisher-built variations and a demo without a subscriber code; the
demo overlays a moving label. Availability of a requested build has not been
tested. Do not purchase access or promise an unpatched build on this evidence.

Locally, the owner asked which version to provide and where to put it. A
workstation inbox has been prepared outside Git. Supply only the executable and
publisher build-info/options metadata, with its download origin; omit ROMs,
disk images and source. The inbox is intake storage, not a run folder. Inspect
and hash only the supplied executable, never neighboring owner ROM files.

## Required decision and safeguards

The owner-supplied first-boot handoff says: "If it rejects or patches a non-Apple
image, record it and stop for an owner decision". Publisher documentation
establishes a patching risk before execution, so the lead stops platform adoption
and the trial here. A decision about further black-box qualification is still
needed. None can waive the exclusion of Apple code or emulator source.

A later run needs an exact executable identity/variation/origin/hash and a
separate owner go-ahead. Before launch, ensure the isolated run folder has our
complete image under the documented name, has no links or other ROM/disk files,
and cannot fall through to an owner ROM location. Never study emulator source,
patch routines, extracted guest code or Apple images to resolve the issue.

R-003 remains open. R-011: the installed pinned LLVM-MinGW 20260908 has Clang
23.1.1, no `llc.exe`, and no M68k in `clang --print-targets`. A separate reviewed
68000 toolchain is necessary; no cross compiler/runtime has been installed or
adopted in this review. The 6522 source and Motorola reset locators remain next
specification prerequisites. SPEC-0018, ADR-0021, IMPL-0022 and TEST-0026 are
not allocated by this review; no ROM or new boot code has been built.

PC B2 remains partial solely on its existing firmware-hash criterion. All full
editions, historical compatibility, 128K budget and physical Macintosh gates
remain open. Original prose assisted by Codex; permitted inputs were project
records and publisher user documentation. Human provenance review pending.
