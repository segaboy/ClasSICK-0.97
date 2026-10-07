# IMPL-0002 — Bounded portable graphics surfaces

- Date: 2026-10-07. Author/reviewer: Codex project implementation lead, self-review;
  owner authorized implementation/publication. Separate human review pending.
- Inputs: SRC-0001, SRC-0016, finalized SPEC-0001 v1, ADR-0001/0002, repository
  clean-room/build/provenance policies and existing pinned tooling. No external
  graphics implementation, Apple code/assets, reference binaries or observations.
- Paths: `core/graphics/surface.h`, `core/graphics/surface.c`, `tests/surfaces.c`,
  CMake integration, freestanding audit and surface verification scripts, explicit
  sanitizer detection probe, workflow and associated documentation.
- Test: TEST-0004. IDs TEST-0001 through TEST-0003 remain the bootstrap records;
  none was reassigned. Source/results are versioned with this record; subsequent
  build registration pins the source commit without a self-referential commit ID.
- Profile: independent generic native-core contract, not QuickDraw/System 1 ABI.
- Original design: validated native descriptor, explicit byte stores, comparison
  clipping before offsets, per-pixel bit masks and channel extraction. No heap,
  mutable global, platform API, mandatory floating point or guest-pointer cast.
  Construction is transactional. Full-row spans and a conservative PTRDIFF_MAX
  bound reject unrepresentable objects. Byte access handles unaligned storage.
- Independent oracle: literal hand-authored mono/RGBA byte vectors plus a test
  oracle visiting every stored bit/channel and selecting by coordinate membership.
  It does not call implementation helpers, reuse clipping or capture Apple pixels.
- Third-party code/assets: none copied or vendored. Existing hash-pinned compiler,
  CMake and Ninja remain developer tools with their own notices. Hosted test code
  links Windows startup/stdio and optional sanitizer runtimes, not OS core services.
- License: GPL-3.0-or-later, project-owned original code/tests/docs; no new runtime
  distribution or third-party adoption in this assignment.
- AI assistance: Codex GPT-6-family agent authored contract, code, synthetic tests
  and self-review using the above permitted inputs. No prohibited material entered
  prompts/research. No independently staffed clean-room or legal certification is
  claimed; automated/self-review cannot replace human provenance review.
- Results: [TEST-0004](TEST-0004-surfaces.md). Windows x64/x86 tests, x64 validated
  ASan/UBSan, strict warnings and no-import freestanding checks pass. ARM64 endian
  checks compile only. The core debug archive contains only function symbols and
  tool feature metadata; include dependencies are stddef/stdint headers.
- Limits: live pointers/true lengths/descriptor non-overlap are caller obligations.
  Performance and Macintosh 128K RAM/ROM/helper budget are unmeasured. No m68k,
  ARM64 runtime, second compiler, Windows presentation, native link/startup,
  historical compatibility or edition boot has been verified. COFF object timestamps
  prevent identical static archives; only the headless executable repeatability
  is established. Surfaces alone do not complete B1/M0.1.
