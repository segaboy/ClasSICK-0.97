# Native PC window boot and dragging

2026-10-22; SRC-0361; TEST-0074. Five unchanged original framebuffer captures
from one isolated headless VirtualBox boot of the exact native x86-64 candidate.
Synthetic device input; no human/GUI/historical or release qualification.

- [Native PC window boot before input, counter0000.](pc-window-initial.png)

- [Body press toggles stripes; held movement leaves counter0001.](pc-window-body-held.png)

- [Window dragged60 pixels right and30 down, counter0002.](pc-window-drag-release.png)

- [Content click changes to checkerboard, counter0003.](pc-window-content-release.png)

- [After70 continuous host seconds and keys, a later press works, counter0004.](pc-window-final-release.png)

[Trial and limits](../../../development/pc-window-boot-trial.md).
[Identity/rights manifest](../manifest.json). GPL-3.0-or-later.
