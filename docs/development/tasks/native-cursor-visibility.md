# Shared visible native cursor:bounded original artifact assignment

Windows11/PowerShell;ClasSICK0.97. Codex alone integrates/writes/verifies.
Owner directs finishing the current review and then moving to paired release.
No new hardware contract,emulator/VM/hardware/reference/release permission here.
Return one original portable C helper plus useful independent visual/restoration
test source. Reviewed public/original inputs below;no HQ,manuals,untracked
research,credentials,held UART,CLEANROM,ROM/media/Apple/Microsoft/emulator code.
Only one view of this frozen prompt through EOF;no other tools/paths/commands,
edits,web,MCP,delegation or tests. No schema/output commentary outside JSON.

Confirmed visibility issue:all three current frontends use the same7x7 plus
with image==mask (black only). On a black checker tile it draws no changed
pixel;own framebuffer export shows no distinguishable center cursor. The new
art must retain width/height7,hotspot3,3,original13pixel black plus,and add a
white one-pixel halo. No borrowed assets or historical cursor reconstruction.
This is a visibility correction,NOT a claim to fix host packet delivery.

Return JSON {helper_c:string,prototype:string,test_c:string,handoff:{result,
actionable_findings,tests_executed:[],blockers,decisions_needed}}.
helper_c is a complete function to append to apps/window-scene/input.c;
prototype goes in input.h. Name void cs_window_cursor_bits_init(cs_cursor_bits*).
One truthful caller-owned bits object;NULL is a harmless no-op. Initialize ALL
64bytes;unused rows/columns transparent. No allocation,globals,libc,hardware or
pointer retention. Original black plus:row3 bitsx0..6;other rows0..6 onlyx3.
White halo is its clipped one-pixel Chebyshev dilation inside7x7:mask rows2..4
bitsx0..6;other rows0..6 bitsx2..4. MSB-first16-bit row,second byte always0.
Do not alter cursor-core behavior,position/input/filter/geometry/payload rules.
Lead replaces duplicated bits loops in PC,Mac and Windows preview with helper;
dimensions/hotspot stay unchanged. No frontend patch needed from you.

test_c is complete tests/window-cursor-visibility.c,strict C11,no assert-only
tests,always-on checks and nonzero failure exit. <=230total source lines across
helper+test;handoff<=120words. GPL3-or-later/original copyright DeanHowell header.
Use actual cursor show/move/hide over a512x342 MONO1_MSB surface with guard bytes
and stride padding. Verify visible difference on uniform black AND white at
center and all four corners,plus the original8pixel checker background at
center;verify exact restore on hide and old-position restoration after move,
and no outside7x7 writes. Freeze bit expectations from the geometry,not by
calling helper twice. Include a concrete old-black-only counterexample that
draws zero difference on uniform black;new art must draw both bit values.
The test fixture must use truthful live extents and disjoint metadata/art/
payload/surface;do not forge native pointers or compare uninitialized padding.
Use provided cursor API;test creates no product/app/ROM/VM execution.

EOF after inputs. Return artifacts,not a plan. Worker executes no tests.
