# ADR-0042: one monochrome 512x342 product scene

2026-10-16: [ADR-0043](0043-coupled-native-mac-pc-parity.md) supersedes the
blanket native-application deferral below. Shared renderer/branding/canvas
decisions remain; the existing interpreted fixture is compatibility coverage,
with native workload parity still an open item.

Accepted 2026-10-16, owner SRC-0209/0210/0211; SPEC-0037.

Share the renderer, original branding geometry and original 68k probe across
Macintosh and native-PC builds. The PC centers its 512x342 logical canvas in
the available framebuffer. Preserve the physical display mode and each existing
input backend. PC128KiB arena holds the probe workspace plus bounded RGBA bands;
the Macintosh keeps its fixed mono surface and counter-only refresh. The native
application gate remains closed: portable ClasSICK bootstrap code interprets
the existing independently authored 68k fixture.

The supplied monitor/C mark is approximated with original monochrome rectangles.
No binary artwork enters Git. A new candidate build does not update the frozen
published alpha or authorize an emulator/VM/reference run.
