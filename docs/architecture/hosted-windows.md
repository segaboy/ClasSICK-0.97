# Windows hosted development

Status: generic surfaces, bounded arenas, normalized keyboard and clocks plus
Windows adapters pass 34 checks per configuration. SPEC-0002 supplies an original
color/mono viewer. The x64 debugger workflow passes TEST-0009; B1 acceptance
remains a separate audit, with no OS boot or historical compatibility claim.

Use a normal native Windows process linked to the same core library planned for
bare-metal builds. The hosted adapter supplies memory arenas, Win32 window/input,
surface presentation, monotonic clock, local file/block devices, and logs. Begin
without SDL, a GUI toolkit, WSL, Docker, or an emulator dependency.

Current viewer: [SPEC-0002](../specifications/SPEC-0002-windows-presentation.md).
platform/windows owns host conversion/GDI; apps/surface-demo owns one host heap
region, supplies it to the core arena, and owns window/native key scaffolding. See
[viewer instructions](../development/windows.md) for Space, resize and Close.

A Win32 window is a presentation adapter. Core graphics write an abstract surface;
the adapter converts/presents it. Input events pass through a normalized queue,
allowing headless tests to inject the same event traces. Deterministic fake clocks
and RAM-backed devices allow tests to avoid real-time or floppy dependencies.

## Working cycle

1. Use hash-pinned portable tools prepared by `Setup-Windows.ps1`.
2. Build out of tree; founding local output goes in `C:\ClasSICK`.
3. Run headless contracts and behavior tests before interactive tests.
4. Run a hosted scene with independently authored artwork, input, and debug logs.
5. Preserve safe test summaries and artifact hashes with exact source revision.
6. Reuse tests across adapters and target configurations as native builds arrive.

The bundled LLDB now launches x64 Debug executables, resolves mapped source,
stops inside core/viewer functions, inspects typed state and stacks, steps over/
in/out and resumes to exit zero. The [debug guide](../development/debugging.md)
and TEST-0009 give a repeatable workflow. A WOW64/i686 trial failed with unavailable
original frame state; it is outside the verified debugging scope. Debugger timing
pauses do not establish physical timer accuracy or a scheduler guarantee.

Sanitizer support is target-dependent: the pinned x64 package detects deliberate
heap/signed overflow controls, then passes all current core/adapter checks under
ASan/UBSan. Other sanitizer targets remain unverified. Availability in upstream
documentation is not execution evidence. Fuzzing starts with original format bytes.

Windows process isolation and diagnostics improve development safety but are not
kernel services. Core code cannot call the host heap, filesystem, or threading
API directly. Hosted-only helpers must have explicit adapter ownership.

Linux/macOS hosted adapters and a second compiler become portability checks after
Windows core contracts stabilize. The build architecture permits them now, but
bootstrap does not claim they are tested. Bare-metal x86-64, m68k, and ARM64 need
separate startup/linker/runtime strategies; Windows-targeting compiler support does
not automatically establish those toolchains.
