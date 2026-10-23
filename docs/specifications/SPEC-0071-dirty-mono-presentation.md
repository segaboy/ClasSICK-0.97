# SPEC-0071 - Bounded dirty Mono presentation

2026-10-23; SRC-0405..0407 / ADR-0082 / IMPL-0073 / TEST-0086.

Add cs_pc_mono_present_rects without changing the old full-frame API. Accept
zero through three half-open logical rectangles, clip to512x342, and publish
only nonempty clipped rows. Null rectangles are allowed only with count0.
Reject inverted bounds before any effect; clamp before subtraction, including
INT_MIN/MAX. Duplicates/overlaps retain list order. required is the sum of
nonempty rectangle heights, rows the number successfully presented.

Preflight ARGUMENT, SOURCE, TARGET, CAPACITY and ALIAS before changing report,
row or device bytes. Include the read-only rectangle span among alias checks.
Require one RGBA row of physical-width*4 capacity. Fill only its used prefix;
call the unchanged framebuffer presenter for each clipped row. Never read the
device framebuffer. A later presentation failure retains completed rows and
the exact child cause; it does not promise rollback.

The native PC window session publishes the compose dirty band, plus old/new
7x7 hotspot(3,3) cursor bounds when the pointer moves: at most three rectangles.
No visual change means no device publication; consumed receipt/RAM processing
counts still advance under their original contract. Initial full presentation,
pointer/window semantics, controller/polling and CWP1 layout are unchanged.
The retained report describes the last physical presentation. A cursor-only
update touches at most98 logical pixels, versus175104 for a full canvas;
this is an operation bound, not a measured latency or GUI-input repair claim.

This amends SPEC-0060/0061 presentation behavior. Existing full-frame clients
are unchanged. The nine-report UEFI fixture now has six publications:
startup plus five visual updates. Releases/no-op release and cancellation
receipts do not fabricate pixel changes. [Evidence](../development/release2-integration-evidence.md).
