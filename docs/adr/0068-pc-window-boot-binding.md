# ADR-0068 - Bind the native window through RAM and a bounded row bridge

Accepted2026-10-22;SPEC-0059;SRC-0346..0348;TEST-0070 planned only.

Keep shared cursor/window work in one512x342 Mono1 RAM canvas. Convert one row
at a time through unchanged PC framebuffer APIs;center unscaled,initialize white
margins once,and never read device framebuffer for cursor save/restore. Keep the
PC session firmware-neutral;current UEFI wrapper enforces its real full gates.
One initial begin+cancel batch precedes first publication and any controller
poll. Unknown LEFT is suppressed until observed release;no invented baseline.
One poll receipt,one owner and coherent accepted-frame publication;bounded
keyboard drain before mouse delivery. Keep owner-prefix and partial-publication
faults distinct,stop without replay or automatic repair,and use dedicated trace.
Preserve legacy timed and continuous profiles;future new window mode is opt-in.

One Grok primary supplied the contract/case drafts;lead corrects startup
publication,product-buffer arithmetic,queue ordering,trace and legacy-mode
assumptions. No second provider or code. Freeze native interfaces with later
implementation;first code proposal is the isolated presentation bridge. Mac
source closure,measured native budgets and fresh image trials remain prerequisites.
[Review](../development/pc-window-boot-contract-review.md).
