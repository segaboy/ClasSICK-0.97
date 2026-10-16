# Standing display target

Owner requirement, 2026-10-16, SRC-0210; continues until the owner changes it.

All PC work targets a logical512x342 image, matching the Macintosh canvas.
Center it in the platform's available framebuffer. At640x480 the origin is
(64,69); other supported framebuffers use their reported dimensions. White
margins, monochrome content, no stretching or requirement for a512x342 hardware
video mode. The hosted development window may retain integer presentation zoom
while keeping this logical size. Small surfaces are clipping conformance cases.

Use the shared SPEC-0037 product renderer and original monitor/C branding for
the current boot/input scene. Keep hardware presentation and input in adapters;
equal scene state must produce equal logical pixels on68k and x86. This rule
also applies to future PC display work; it is not a historical Macintosh claim.

[Contract](../specifications/SPEC-0037-shared-product-scene.md) and
[verification](shared-product-scene-evidence.md). Release assets remain frozen;
new builds and future trials retain their independent identities and gates.
