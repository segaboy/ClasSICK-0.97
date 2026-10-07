# Portable OS core

Reserved for independently implemented freestanding core code. No OS code exists
at bootstrap. Planned modules: memory arenas, filesystem/VFS, graphics surfaces,
events, generic devices, timers and executable loading.

No Win32/UEFI/Macintosh headers, guest ABI assumptions, direct hardware registers,
fixed display geometry, native MFS requirement or whole-machine emulator dependency.
The first selected module is `graphics/` under SPEC-0001. Core tests use caller-owned
buffers and abstract/fake services before any platform presentation.
