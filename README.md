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

Project bootstrap began on **2026-10-07**. No OS subsystem has been implemented.
The initial build will contain only development-toolchain probes. Those probes
do not boot an OS and do not demonstrate Macintosh compatibility.

The historical internal-version claim above is a founding scope statement;
edition-specific primary evidence and reference behavior remain research tasks.

## Start here

- [Clean-room policy](docs/clean-room/POLICY.md)
- [Contribution rules](CONTRIBUTING.md)
- [Provenance rules](provenance/README.md)

Architecture, reproducible setup, milestones, and verification records will be
added during bootstrap. Public source repository:
[segaboy/ClasSICK-0.97](https://github.com/segaboy/ClasSICK-0.97).

## Licensing

**No project license has been selected.** Publication on GitHub is not a general
grant to reuse, modify, or redistribute this work. License selection is reserved
to the owner. Do not call the project open source until that decision is made.
External code contributions are deferred until inbound and outbound licensing
terms are established. Public issue discussions must follow the clean-room policy.

ClasSICK 0.97 is an independent project and is not affiliated with Apple.
