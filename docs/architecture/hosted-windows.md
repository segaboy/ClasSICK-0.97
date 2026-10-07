# Windows hosted development

Status: first execution platform; SPEC-0001 headless conformance verified on x64/x86.
SPEC-0002 adds presentation and an original color/mono viewer. Normalized
events and clock remain planned; B1 has not been met. SPEC-0003 now supplies
bounded arenas for the viewer's buffers, verified independently by TEST-0006.

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

Debug symbols and LLDB integration are planned, but a debugger must actually
launch, set a breakpoint, and inspect state before being called verified. Sanitizer
support is target-dependent: the pinned x64 package now detects deliberate heap
overflow and signed overflow probes, then passes surface tests under ASan/UBSan
through `Verify-Surfaces.ps1 -Sanitizers`. Other sanitizer targets remain unverified.
Availability in upstream tool documentation is not local test
evidence. Fuzzing starts with format decoders against original synthetic bytes.

Windows process isolation and diagnostics improve development safety but are not
kernel services. Core code cannot call the host heap, filesystem, or threading
API directly. Hosted-only helpers must have explicit adapter ownership.

Linux/macOS hosted adapters and a second compiler become portability checks after
Windows core contracts stabilize. The build architecture permits them now, but
bootstrap does not claim they are tested. Bare-metal x86-64, m68k, and ARM64 need
separate startup/linker/runtime strategies; Windows-targeting compiler support does
not automatically establish those toolchains.
