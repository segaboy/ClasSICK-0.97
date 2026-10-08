# ClasSICK 0.97

ClasSICK 0.97 is a new, independently implemented, portable operating environment
whose first compatibility target is the January 1984 Macintosh System 1.0
environment, identified in the project brief as System 0.97 internally.

The OS will run natively on each supported architecture. A contained Motorola
68000 execution engine may translate **legacy application instructions** on
non-68000 hosts; it will call the same independently implemented System 1 APIs.

This project is not a redistribution of Macintosh System Software, a patched
Apple system, a reconstruction of Apple's source, or a whole-Macintosh emulator.
It contains no Apple executable code or ROM code. Original graphical resources
will be replaced by independently created assets. Macintosh 128K hardware is a
planned target, not the definition of the portable OS.

## Current state

Project bootstrap began on **2026-10-07**. The first subsystem now implements
[SPEC-0001 bounded graphics surfaces](docs/specifications/SPEC-0001-surfaces.md):
caller-owned 1bpp MSB-first and RGBA8 storage, clear, clipped rectangle fill and
stable validation errors. Independent headless tests pass on Windows x64 and x86,
including validated ASan/UBSan on x64 and freestanding import checks.
This verifies our generic contract; no OS boot or Macintosh compatibility is proven.

[SPEC-0002 Windows presentation](docs/specifications/SPEC-0002-windows-presentation.md)
adds a native color/monochrome viewer with original geometric scenes. Space changes
views; resizing uses sharp integer scaling. See
[build, run and review instructions](docs/development/windows.md). The remaining
hosted-core acceptance gate is recorded in the [B1 audit](docs/development/hosted-start-audit.md).
Bounded [SPEC-0003 arenas](docs/specifications/SPEC-0003-arenas.md) now reserve all
viewer buffers from one host-owned region; independent boundary/ownership tests
and the full sixteen-check Windows matrix pass. See [arena results](provenance/records/TEST-0006-arenas.md).

[SPEC-0004 keyboard events](docs/specifications/SPEC-0004-input.md) now routes
Space/Escape press, release and repeat through one bounded portable queue.
Synthetic and Windows-message input share the viewer consumer. The current
24-check matrix passes; [input results](provenance/records/TEST-0007-input.md)
remain generic contract evidence, with full-input/native-driver work open.

[SPEC-0005 portable time](docs/specifications/SPEC-0005-clock.md) now supplies
checked elapsed-time arithmetic, monotonic observations and a controllable clock.
The Windows provider and original viewer's timed strip pass the full 34-check
matrix. See [clock results](provenance/records/TEST-0008-clocks.md). The owner's installed
VirtualBox is recorded as a future native-PC test option.

The x64 [debugger workflow](docs/development/debugging.md) is now exercised:
source breakpoints, typed core/viewer state, stepping, call stacks and clean resume.
The viewer accepts parsed launch arguments, including a debugger's trailing space.
Six scripted x64 sessions and three invalid-argument rejections pass; all 34 core/
adapter checks still pass. A 32-bit WOW64 debugger trial failed and is outside this
verified workflow. Native boot work remains open.

**B1/M0.1 hosted start now passes (2026-10-08).** The combined startup, input,
timing and clean shutdown test passes, and the owner completed a separate live
keyboard session. All 36 checks in five configurations and the x64 debugger replay
pass locally and in [Windows CI](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37726362180).
This verifies a Windows development start; Mac/PC OS boot and
historical compatibility remain future work. See the [audit and repeat instructions](docs/development/hosted-start-audit.md).

The owner reaffirmed real Macintosh booting and mini vMac validation. Those native
images must use independently implemented OS/replacement firmware code. Emulator
startup support, physical boot and exact historical 0.97 identity remain unverified.

The founding Windows setup has been exercised with built-in PowerShell. Two fresh
native Windows probe builds passed CTest and produced identical executable hashes.
See [verification evidence](docs/development/bootstrap-evidence.md) and
[current project status](docs/status.md).

Every completed version requires original-hardware, native-PC without Linux, and
Linux-PC editions. Original/native-PC editions target the same behavior; Linux
extensions require explicit versioned divergence. All three boot gates are pending.

The historical internal-version claim above is a founding scope statement;
edition-specific primary evidence and reference behavior remain research tasks.

## Start here

- [Clean-room policy](docs/clean-room/POLICY.md)
- [Contribution rules](CONTRIBUTING.md)
- [Provenance rules](provenance/README.md)
- [Project charter and success criteria](docs/charter.md)
- [Architecture and subsystem boundaries](docs/architecture/overview.md)
- [Windows setup and build](docs/development/windows.md)
- [Milestones and first implementation](docs/roadmap.md)
- [Native boot gates and dependency graph](docs/architecture/boot.md)
- [Architecture decisions](docs/adr/README.md)
- [Research questions](docs/research/questions.md)
- [Testing and compatibility](docs/testing/methodology.md)
- [Approved license and option comparison](docs/development/licensing.md)
- [Founding assignment coverage](docs/bootstrap-assignment.md)

Public source repository:
[segaboy/ClasSICK-0.97](https://github.com/segaboy/ClasSICK-0.97).

## Licensing

ClasSICK 0.97 is licensed under the **GNU General Public License, version 3 or,
at your option, any later version** (`GPL-3.0-or-later`). The owner approved this
choice on 2026-10-07 for the project's own code, documentation, and assets.
See [LICENSE](LICENSE), [copyright and scope](COPYRIGHT.md), and
[contribution terms](CONTRIBUTING.md). Third-party tools retain their own terms.
All public contributions and discussions must follow the clean-room policy.

ClasSICK 0.97 is an independent project and is not affiliated with Apple.
