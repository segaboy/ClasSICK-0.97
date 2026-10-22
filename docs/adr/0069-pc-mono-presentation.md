# ADR-0069 - Convert the fixed monochrome canvas through one RGBA row

Accepted2026-10-22;SPEC-0060;SRC-0349..0352;IMPL-0062/TEST-0071.

Use one stateless internal native API with caller-owned initial/update sequencing.
Validate full spans and aliases before effects. Reuse existing framebuffer APIs,
never bind cursor storage to MMIO,and expose exact row progress after refusal.
Keep the512x342 canvas independent of physical dimensions;one bounded row replaces
a full physical RAM image. This advances SPEC-0059 without a boot/session binding.

One Grok primary implementation attempt times out without usable code;Codex writes
the small frozen-contract helper directly. Claude provides the distinct independent
whole-target/padding/fault oracle without the implementation. No retry/fan-out or
new qualification. Lead handles integration,negative controls and actual checks.
No new image,device source,historical claim,target trial or release.
[Actual evidence](../development/pc-mono-present-evidence.md).
