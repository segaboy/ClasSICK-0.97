# IMPL-0130 - Experimental 286 keyboard scan decoder

2026-11-14; SPEC-0103 / SPEC-0051 / IMPL-0129. Original AI-assisted Codex
software maps limited set-1/set-2 make/break streams into pointer samples;
Space repeats do not become extra presses. GPL-3.0-or-later. An opt-in
BootHush-only raw polling fixture assumes controller translation and masks
IRQ1. Human provenance, code and hardware review remain open; no full
controller driver or production OS claim. [Evidence](../../docs/development/mac-pc286-shared-parity-20261114.md).
