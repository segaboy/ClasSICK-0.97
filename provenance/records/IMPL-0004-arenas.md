# IMPL-0004 — Bounded native arenas and viewer integration

Date: 2026-10-07. Codex project lead self-review; human provenance review pending.
Owner continuation: SRC-0019; design contract SPEC-0003 v1; tests TEST-0006.

- Permitted inputs: project policy/architecture, existing surface/presentation
  contracts, owner requirement, C11 interface semantics SRC-0020 and pinned tools.
- Paths: core/memory/arena.h/c, tests/arenas.c, viewer buffer initialization and
  cleanup, CMake, generic core audit script, Verify-Arenas.ps1 and Windows workflow.
- Original design: caller-aligned storage, checked monotonic cursor, byte spans,
  constant-work failure/reset, no backing reads/writes or host imports. Core never
  converts a pointer to an integer. Caller lifetime/disjointness remain preconditions.
- Test oracle: literal layouts and linear candidate-offset scan, separate from
  implementation bit rounding; full-buffer guards and failure-state comparisons.
  Native typed-object test uses allocated undeclared backing only. Host uintptr_t
  alignment inspection stays in tests, never in the portable allocator.
- Host integration: one malloc-owned region, three core reservations, host-side
  initialization, destroyed window before span retirement/reset/free. Native key
  handling remains outside the core. Development pool is not a Mac RAM budget.
- AI: Codex GPT-6-family specification/code/test authoring and self-review using
  only permitted inputs above. No third-party implementation/assets copied,
  Apple implementation/reference payloads used, staffed clean-room separation or
  legal certification claimed. Project-owned changes are GPL-3.0-or-later.
- Results: [TEST-0006](TEST-0006-arenas.md). No historical Memory Manager,
  relocatable handles, guest heap, m68k execution, B1 or native boot certification.
- Mac boot goal: independent OS/replacement firmware on declared hardware and
  mini vMac validation. Tool startup/provenance/configuration remains research.
