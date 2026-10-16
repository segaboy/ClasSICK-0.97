# SPEC-0038 - Paired native drawing workload

2026-10-16; owner continuation SRC-0219 after SRC-0217 / ADR-0043.
Original engineering profile `native-rectangles-v1`. Uses SPEC-0036's existing
native fixed-port rectangle service, not a new historical QuickDraw claim.

## Workload and result

`cs_native_draw(port,result)` is a synchronous original C workload compiled
natively for each target. Both live aligned objects are caller-owned, truthful,
mutually disjoint and synchronized. No guest addresses, callbacks, allocation,
mutable globals, libc, loader, trap gateway or interpreter dependency.
Null argument returns0 before changing either object. Other calls first zero
the port and result, including diagnostic counters, regardless of previous state.
Objects are borrowed only during the call; no pointer is retained.

Call the existing `cs_qd_rect` directly with an original big-endian Rect
(top8,left8,bottom56,right120), canonical PaintRect; then
(top16,left16,bottom48,right112), canonical EraseRect. The existing128x64,
stride16, MSB-first black/white profile and error behavior remain unchanged.
The result is a hollow rectangle with2304 black pixels, one paint and one erase.

`cs_native_draw_result` has uint32_t fields success,phase,value,pattern,black_pixels.
The pattern is the actual FNV-1a32 scan of all1024 resulting pixel bytes,
starting2166136261 and multiplying by16777619 after each XOR, unsigned wrap.
Phases are native-workload diagnostics:1 initialized,2 paint completed,
3 erase completed,4 pixels scanned,5 metrics validated,6 completed. These are
not CODE loading/retirement phases. Value97 and success1 appear only after both
real service calls and metrics validation. Verify actual one-paint/one-erase
counters and2304 black pixels. Service refusal or metric failure returns0 with
success/value0 and the completed phase/effects retained; never synthesize success.

## Bindings and compatibility

PC boot and hosted Windows scenes use the native result and native port pixels.
The older cs_scene_render entry also calls the real native workload in a bounded
local1032-byte port; it returns CS_SCENE_WORKLOAD before surface changes if the
workload fails. It no longer manufactures a successful geometric demonstration.
A distinct `--native-drawing` Macintosh ROM profile uses the same C workload.
Its kernel preflight uses native reset/TRAP and service checks; no legacy Line-A
or interpreter application probe is required. Unexpected Line-A faults in this
native-only profile; the separate legacy drawing profile retains its boundary.
The existing `--drawing` profile, original40-byte68k fixture and its resource/
interpreter/retirement tests remain independent compatibility coverage.
No external native executable format, variant selection, SDK or application ABI
is introduced. A direct native C call is not evidence of a general native loader.

Keep the shared512x342 scene/branding and each input backend. Native phase marks
describe the new phase definitions; APP/097 is the original workload's result,
not a claim of resource loading. Actual result/pixels drive display. New native
ROM/EFI bytes are candidates; existing releases and observations are immutable.
PC mouse/event parity remains open.

## Verification and limits

Independently check every pixel from the stated coordinate predicates, complete
native/legacy image equivalence, counts/hash/result, poisoned/repeated calls,
null no-effect refusal, object canaries and complete scene parity. Fault-control
links must show real helper refusal/incorrect metrics cannot display success.
Audit the workload object's sole allowed external import `cs_qd_rect`; a complete
native workload/service link has no undefined import or interpreter symbol.
Run compiler widths/endians/sanitizers, relevant regressions, native68000/x86
construction and ROM twins. Compiler/hosted evidence and actual target execution
are separate. No emulator/VM/reference run is authorized by this contract.
Fresh owner go-ahead, all source/UART/physical/full128K/edition/B2/human and both
System1/ROM requirements remain. No full QuickDraw/native application completeness.
