# SPEC-0060 - Bounded PC monochrome presentation bridge

2026-10-22;SRC-0349..0352;ADR-0069;IMPL-0062;TEST-0071.
Original engineering implementation slice of SPEC-0059, not a guest ABI.

The frozen native interface is [mono-present.h](../../platform/pc/mono-present.h).
It borrows a logical512x342,64-byte-stride ordinary-RAM Mono1 MSB canvas and one
RGBA row of4*physical width bytes. It retains no pointer, allocates nothing and
introduces no UEFI, controller, scene or boot dependency. The caller controls
initial publication and keeps mappings/storage live throughout each call.

Preflight validates numeric fixed target/canvas/output object ranges and row
NULL, then initial0/1;invalid fixed arguments return ARGUMENT. Validate source
format/exact geometry/stride and full declared storage span (SOURCE), then target
through unchanged cs_fb_init, minimum512x342 and full declared device span
(TARGET). Validate row capacity >=4W and its full numeric span (CAPACITY). Finally
reject all writable-span aliases against each other and readonly descriptors/
full declared canvas storage (ALIAS). Readonly spans may share. Empty/wrapped
source or target storage has its corresponding SOURCE/TARGET result. Native
objects must be truthful/readable/aligned; numeric checks do not catch arbitrary
pointer faults. No caller writes or canvas pixel reads before complete preflight.
cs_surface_init and cs_fb_init may initialize local descriptors during validation.

Center at floor((W-512)/2),floor((H-342)/2),without scaling. Initial publication
constructs each full physical-width row white and overlays its logical canvas
intersection. Later publication converts only512 pixels for each342 logical
rows and calls the existing presenter at the centered origin. Mono bit1 means
black RGBA000000FF,bit0 white FFFFFFFF. cs_fb_present writes reserved byte0 for
both supported device formats. Pitch padding, device tail and unused workspace
bytes remain untouched. No device framebuffer reads or direct writes occur in
the bridge; all device writes go through unchanged cs_fb_present. One row only;
no full physical-frame RAM image. No library/runtime imports beyond the three
named project APIs are admitted for the bridge object.

On complete preflight set report VALIDATED,rows0,required=physical height for
initial or342 otherwise,cause0. Each successful dependency row increments rows;
only complete publication sets COMPLETE. A dependency refusal after preflight
returns PRESENT,sets FAILED and raw CS_FB cause,retains earlier row/workspace
effects,and stops issuing row calls. This software progress is not atomic
scanout, recovery from MMIO exceptions or proof of visible hardware refresh.
The16-byte report is native,not a guest or UART trace record. No report/caller
mutation is permitted on any preflight refusal; post-preflight failure may leave
partial device output. Caller owns fault/retirement policy under SPEC-0059.

TEST-0071 uses an independently authored whole-target byte oracle from this API,
with real existing framebuffer/surface dependencies; a separate renamed-presenter
object injects row refusal and verifies calls/progress. Padding, margins, both
formats, bit order, reused workspace, initial/update, refusal aliases/wrap and
unchanged source/report bytes are meaningful checks. Compiled negative controls
must be rejected. Planned binding cases WB-05/06/21/29/30/31 inform this leaf;
their complete session/boot scenarios and other TEST-0070 cases stay unexecuted.

This slice permits original bridge/header, independent hosted tests, compiler/
normal CI and ordinary records. No session/UEFI/ROM/boot build input, hardware
interface, target trial or release changes. Released images/tags and all existing
source/role/UART/Mac M1-M7/physical/128K/B2/native-app/compatibility gates persist.
