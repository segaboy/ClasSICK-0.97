# ADR-0062 - Hide,repaint,advance,show

Accepted2026-10-20;SPEC-0054;SRC-0326..0328.
Use one existing cursor save slot and conservative full-width dirty row bands.
Reconstruct the unchanged branded backdrop,overlay original fixed window pixels,
advance scene revision,then save/draw cursor on top. No second framebuffer.
An unchanged scene uses cursor_move alone. Counter updates behind the window
redraw the covering window;moving it restores the original hidden backdrop.
Preserve pre-effect refusal and publish reached stages after effects;accepted
logical input cannot roll back. An external owner retains rendered snapshots,
stops on faults and explicitly repaints/rotates binding when necessary.
[Contract](../specifications/SPEC-0054-native-window-composition.md).
