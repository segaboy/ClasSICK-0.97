# Portable OS core

Independently implemented freestanding core code. Bounded graphics surfaces are
implemented and headless-tested under SPEC-0001. `memory/` now supplies bounded
caller-owned arenas under SPEC-0003, including viewer integration. Planned modules:
filesystem/VFS, events, generic devices, timers and executable loading.

No Win32/UEFI/Macintosh headers, guest ABI assumptions, direct hardware registers,
fixed display geometry, native MFS requirement or whole-machine emulator dependency.
The first selected module is `graphics/` under SPEC-0001. Core tests use caller-owned
buffers and abstract/fake services before any platform presentation.

Bounded keyboard FIFO and portable clocks are implemented under SPEC-0004/0005.
The [complete-link checkpoint](../docs/development/core-link-evidence.md) combines
all four modules in original unloaded fixtures, excluding startup/default runtime
libraries under Clang/LLD and GCC/GNU ld. This scoped profile does not establish
native boot, firmware ABI or runtime independence for future code/targets.
