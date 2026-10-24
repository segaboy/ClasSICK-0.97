# Original PC first-motion diagnostic,2026-10-24

Only original native ClasSICK framebuffer pixels, GPL-3.0-or-later.
512x342 monochrome logical canvas centered unscaled in1024x768.

- [Original PC motion diagnostic before input; count0000, window80,70.](pc-first-motion-initial.png)
- [Original PC diagnostic after initial capture attempts; no nonzero movement packet yet.](pc-first-motion-capture.png)
- [First decoded motion stops the diagnostic: dx255/dy-255/right button2, cursor clipped to511,341; no drag.](pc-first-motion-stopped.png)


[Actual packet, source identities and limits](../../../development/pc-first-motion-evidence.md).
[PNG identities](../manifest.json). One diagnostic stop, no successful drag or
release claim. VM closed; existing inventory/media preserved. No new GIF.


Following-packet trial (SRC-0433),same product-pixel rights:

- [Original following-packet diagnostic before input.](pc-following-motion-initial.png)
- [After first one-pixel gesture:cursor at corner,no later-packet spin.](pc-following-motion-first.png)
- [Later packet3 repeats255/-255/right-button2;window intact,no GUI drag pass.](pc-following-motion-stopped.png)

[Actual following-packet evidence and limits](../../../development/pc-following-motion-evidence.md).


- [Mac unchanged-ROM button1;motion remains unqualified.](mac-motion-audit-button.png)

[Actual RAM comparison and limits](../../../development/mac-motion-audit-evidence.md).
