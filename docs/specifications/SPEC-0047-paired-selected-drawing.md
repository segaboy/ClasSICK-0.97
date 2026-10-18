# SPEC-0047 Paired native selected-port demonstration

Accepted engineering profile2026-10-18; owner "Do it." accepts the proposed
paired visual workload after SPEC-0046. Same original native C on68000/x86;
the PC OS drawing path does not execute68k instructions. No target/reference/
hardware trial,release,SDK/full manager/global dispatch or historical claim.

Frozen API: apps/selected-drawing/workload.h. Caller-owned workspace contains
two disjoint1032-byte ports and1024 presentation bytes; result holds original
draw diagnostics plus numeric current/selections. No retained native pointers.
Null either input returns0 byte-inert; otherwise reset workspace/results before
work. Truthful native liveness/alignment/disjointness/exclusivity are preconditions.
No private GrafPort layout or real guest RAM is constructed by the native demo.

Use the real SPEC-0043 selection core with two root-local prepared native views:
globals_begin=0x7000,A5=0x7200,g=0x71FC; IDs0x7000 and0x706C. These are original
engineering identifiers,not historical allocations. Current starts0. Set first
ID through cs_qd_select_set,publish plan.value as current,Get and verify it via
cs_qd_select_get,draw Paint then Erase through that resolved native view. Repeat
Set/Get/Paint/Erase for second ID; final Get must still report second ID. Count
two successful selections. Never choose a view by hardcoded port address after
selection; resolve the plan's view to its root-local index without const casts.

Port0 view offset(8,8),physical clip(top8,left8,bottom56,right56).
Port1 view offset(72,8),physical clip(top8,left72,bottom56,right120).
Both Paint Rects signed16[-32768,-32768,32767,32767].
Port0 Erase logical[8,8,40,40]; port1 Erase logical[0,16,48,32].
All Rects are eight-byte BE original constants. Per-port paints=1,erases=1.
Expected port0 is physical[8,8,56,56] excluding[16,16,48,48]:1280 black pixels.
Expected port1 is[8,72,56,120] excluding[8,88,56,104]:1536 black pixels.
Only after validating both counts/current/selections compose pixels[i] as the
bitwise union of the two ports; union has2816 black pixels. Compute actual FNV1a
and black count over composite; value97/success1 only if actual validations pass.
Phases:1 reset/views;2 first Set/Get/Paint/Erase complete;3 second complete;
4 final current/counts validated and composite/hash/count produced;5 all actual
invariants including2816 confirmed;6 success/value97. Failure preserves last
completed phase/current/selections/port effects,never fabricates success.

Explicit build mode CLASSICK_SELECTED_DRAWING selects this workload in Mac
native-drawing-entry.c and PC cs_boot_scene_prepare. Old modes/APIs/scenes/tests
retain behavior. cs_scene_render remains the legacy utility even in the selected
PC compilation; the actual native PC entry uses prepare/draw. The existing
product renderer receives the composite only: branding,APP/097,six phases,input
counter,logical512x342 and PC centering remain. Result panel shows left outline
and right split block; counter updates must preserve both. No product-scene code
change or native input/controller/UART change. New paired candidates are unbooted.

Mac compiler build adds --selected-drawing alongside existing mutually exclusive
modes and audits direct-native symbols/twins/stack/storage. PC builder accepts
-SelectedDrawing and retains mouse profile,old modes/controls. All four target
builds are compiler-only; normal Git/HQ/CI permitted. Tests must independently
check both pixel arrays/composite/diagnostics and mono-vs-PC scene/input parity.
The legacy SPEC-0046 owner binding remains separately tested,not implied to run
in this native workload. No full128K/physical/B2/System1 boot gate closes.
