# IMPL-0003 — Windows presentation and original viewer

Date: 2026-10-07. Codex project lead self-review; human review pending. Owner
authorized the next checkpoint under SRC-0017. SPEC-0002 v1 / TEST-0005.

- Inputs: project policies/contracts, SRC-0017, Microsoft Win32 interface prose
  (SRC-0018), existing pinned Windows headers/import libraries. Core bytes unchanged.
- Paths: platform/windows/pixels.h/c and presenter.h/c,
  apps/surface-demo/windows-main.c, tests/windows-presentation.c, CMake,
  Verify-Presentation.ps1, workflow and associated documentation.
- Original design: validated byte conversion into caller-owned BGRX scratch,
  top-down DIB, centered integer scaling/crop, clipping, saved/restored host DC.
  No allocation in the converter/presenter and no Windows dependency in core.
  Viewer-owned heap/window state selects two separately stored original scenes.
- Native key/resize/DPI/paint/close handling is Windows demo scaffolding. No core
  event queue, arena or clock. DPI awareness is set before UI; exact client sizing
  uses actual window DPI. Modern Windows is the tested development host.
- Tests: original literal bytes/colors/bits, complete rendered RGB pixel comparisons
  and independent expected scale/origin; no implementation converter/viewport oracle.
- Third-party code/assets: none copied or vendored. No Apple implementation,
  reference binaries/systems, fonts/icons/artwork or historical observations used.
  Project-owned code/tests/scenes are GPL-3.0-or-later. Windows startup/runtime/APIs
  and optional sanitizer runtime are hosted developer dependencies, not OS services.
- AI: Codex GPT-6-family authored specification/code/fixtures/tests/self-review
  using only the permitted inputs above. No independently staffed clean-room,
  legal certification or human approval is claimed.
- Results/retained failures: [TEST-0005](TEST-0005-windows-presentation.md).
- Limits: truthful live/non-overlapping buffers, 4096 pixels per axis adapter cap,
  ignored alpha, identity client pixel coordinates. No physical monitor fidelity,
  actual multi-monitor transition, B1, historical compatibility or edition boot.
  Source/text evidence is published; future executable redistribution needs a
  reviewed runtime/dependency inventory. No release binary is added here.
