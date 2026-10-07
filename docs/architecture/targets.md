# Target matrix

No OS target is implemented at bootstrap. Distinguish verified tooling from a
planned supported target; do not infer hardware support from a directory name.

| Target | Stage | Execution strategy | Status / release gate |
| --- | --- | --- | --- |
| Windows 11 x86-64 hosted | First development execution | Native process; explicit host adapter | Headless SPEC-0001 tests verified in debug/release and ASan/UBSan; B1 pending |
| Windows x86 hosted | Native size-boundary check | 32-bit process on Windows x64 | Headless SPEC-0001 tests verified, including 32-bit row overflow; no OS adapter |
| x86-64 UEFI PC | First native boot, M0.2 | Native loader/kernel; generic firmware handoff; own post-handoff drivers | VM firmware, USB/PS2 driver choice, PE/COFF/link and compiler runtime pending |
| Original 68000 Macintosh 128K | M0.3 | Native core/personality; independently created replacement ROM/startup | RAM/ROM feasibility and hardware specification gate pending |
| mini vMac Macintosh validation | M0.3 development check | Intended runner for our independent native Mac image/replacement firmware | Owner-requested; exact emulator provenance/configuration and startup support unverified; physical boot remains separate |
| ARM64 hosted | Portability proof before 1.0 | Native core; optional 68000 application interpreter | Surface little/big-endian ELF compile/import checks pass; execution/runner unverified |
| ARM64 bare metal | Later staged target | Native core, concrete board/firmware and drivers | No board chosen; do not promise generic ARM64 hardware support |
| Linux/macOS hosted | Secondary development | Native process, platform-specific adapter | Planned; not installed or tested on founding workstation |
| RISC-V | Future option | Native architecture/platform ports | No milestone commitment |

The first boot VM may emulate PC hardware while the OS executes native x86-64.
That testing tool is distinct from emulating a Macintosh under the OS. A Macintosh
test machine/VM must never supply Apple ROM fallback for our native boot criterion.
User-owned reference systems for black-box measurements are a separate activity.

Do not assume QEMU, firmware, cross compilers, SDKs, or real hardware are installed.
Their purpose, distribution rights, source provenance, versions, and setup must be
reviewed when the milestone requires them.
