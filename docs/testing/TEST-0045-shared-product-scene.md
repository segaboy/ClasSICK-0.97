# TEST-0045 - Shared product scene

2026-10-16; SPEC-0037 / IMPL-0041; original cases and independently generated
expectations. Executed results are in the linked evidence, not inferred here.

- product-scene.parity: actual shared original probe on both preparations;
  every512x342 mono pixel equals centered640x480 RGB/BGR output, including real
  status/phase/image and input1234. White margins, pitch/tail guards and no
  hidden allocation beyond the declared128KiB PC arena.
- product-scene.updates: mono/RGBA512x342,640x480 and641x483, padded rows/tails;
  UINT32_MAX input update equals full redraw and preserves pixels outside the
  translated input-counter region, including shared boundary bytes.
- product-scene.errors: null/malformed descriptor, invalid frame/band/state;
  no output mutation. product-scene.freestanding plus50 compiler object audits.
- Existing scene oracle/bands/budget/errors and native framebuffer/presenter/
  keyboard/timer/qualification/serial regressions, updated original geometry
  expectations where necessary. UART bodies unchanged and not read.
- Hosted viewer lifecycle, both monochrome pixel formats, logical512x342,
  real accepted Space counter, flash deadline, literal memory offsets and
  workspace accounting. Both compiler widths/optimizations/sanitizers and twins.
- Unloaded original EFI O0/O2 twins, corruption/omitted-object controls; original
  GNU68000 ROM twins, assembly encoding, seal/reset/exception/stack/layout checks.

No emulator/VM/reference trial, historical or human-input result follows.
[Evidence](../development/shared-product-scene-evidence.md).
