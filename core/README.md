# Portable OS core

Independently implemented freestanding core code. Bounded graphics surfaces are
implemented and headless-tested under SPEC-0001. Planned modules: memory arenas,
filesystem/VFS, events, generic devices, timers and executable loading.

No Win32/UEFI/Macintosh headers, guest ABI assumptions, direct hardware registers,
fixed display geometry, native MFS requirement or whole-machine emulator dependency.
The first selected module is `graphics/` under SPEC-0001. Core tests use caller-owned
buffers and abstract/fake services before any platform presentation.
