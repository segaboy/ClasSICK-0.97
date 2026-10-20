# SPEC-0053 - Original native window interaction

Finalized engineering contract2026-10-20;SRC-0323..0325;ADR-0061.
Profile:original single fixed-size native window on the512x342 canvas.
Evidence strength:independent platform design,not historical Macintosh evidence.
Pointer publication/order follows SPEC-0051. Rendering follows a later contract.
Implemented under IMPL-0056/TEST-0064 as an isolated shared C helper only.

typedef struct {int32_t x,y,width,height,title;} cs_window_config;
typedef struct {int32_t x,y,width,height,title,grab_x,grab_y;uint32_t down,dragging,pattern,valid;} cs_window;
typedef struct {int32_t old_x,old_y,x,y;uint32_t flags;} cs_window_effect;
CS_WINDOW_VALID 0x57494E31u
statuses OK0,CHANGED1,ARGUMENT2,STATE3; effects MOVED1,DRAG_BEGAN2,DRAG_ENDED4,TOGGLED8.
int cs_window_begin(cs_window*,const cs_window_config*);
int cs_window_apply(cs_window*,const cs_pointer_event*,cs_window_effect*);

Caller passes truthful live aligned native objects, single quiescent nonreentrant owner. Before dereferencing, reject NULL, wrapping uintptr_t complete ranges and every pairwise overlapping argument object as ARGUMENT. Never promise detection of unreadable native pointers. Begin does not read prior state: fresh/quiescent coordinated pointer+counter session only. Config:2<=width<=512,2<=height<=342,1<=title<height,0<=x<=512-width,0<=y<=342-height. Invalid config ARGUMENT before any writes. Success stores config plus grab0/down0/dragging0/pattern0/valid;returnsOK. No reset of live owner/pointer session.
Apply precedence: fixed argument ranges/overlap ARGUMENT; canonical state STATE; event ARGUMENT; edge continuity STATE. Refusal preserves all state/input/output bytes. Valid state marker/config bounds, bool fields0..1, idle grabs0; dragging requires down1 and 0<=grab_x<width,0<=grab_y<title. Event x0..511,y0..341,buttons0..7,flags only pointer.h bits. Ordinary event: PRESS and RELEASE never together; PRESS implies DOWN1/SUP0, RELEASE implies DOWN0/SUP0. SUP implies DOWN0/noedges and rawLEFT1. Without SUP, DOWN equals rawLEFT. CANCELLED canonical: flags exactly CANCELLED|SUPPRESSED optionally RELEASE;buttons0;no MOVED,CLAMP,DOWN,PRESS. Continuity: PRESS requires old down0; RELEASE requires old down1; otherwise event DOWN must equal old down except CANCELLED forces down0 and includes RELEASE iff old down1. Repeated cancel admitted. No heuristic event deduplication: owner must deliver authentic accepted pointer outputs exactly once (including unchanged/clamp-only outputs); no replay/resample. Pointer transitions already accepted remain final if consumer refuses; later pipeline must stop/fault/repair, not rollback or re-begin.
Window rectangle is [x,x+width)*[y,y+height); title top title rows/full width; content remainder. Border drawing deferred. On CANCELLED end drag, clear grabs and down, keep position/pattern. On an already-active drag, MOVED first sets window position to clamp(event.x-grab_x,0,512-width),clamp(event.y-grab_y,0,342-height), even on final RELEASE; then RELEASE ends/clears grabs. Preserve original grab offsets at canvas clamp, no reanchoring. New PRESS hit-tests final event coords against current window: title starts drag with grab offsets; content toggles pattern once; outside does neither. A new PRESS|MOVED must not move window using previous anchor. Track logical down even when outside/content. No toggles from holds/right/middle/release/cancel. Consume flags, never resample physical buttons to generate edges. Record old/new coordinates and effects on every success (includingOK); MOVED only actual window displacement; DRAG_BEGAN/ENDED and TOGGLED only actual transitions. ReturnCHANGED iff any state field changed (down alone counts), elseOK. Complete output initialization. State44B/config20B/effect20B checked via static_assert in tests. Normal math bounded by canvas, so no signed overflow/unchecked coordinate arithmetic.


Fixed geometry is configured once;no resize,close control,activation,overlapping
windows,guest Window Manager or historical Finder claim. Half-open hit areas are
engineering behavior. Raw button bits validate coherence only;edges come solely
from the accepted pointer flags. Authenticity/exactly-once delivery cannot be
proved by this API:the later owner must enforce it and coordinate fresh sessions.
No arbitrary native-pointer readability or async/interrupt safety is promised.

TEST-0064 executes directed boundary/drag/cancel/hold/config/refusal cases plus
a separate valid-event reference model authored before the implementation.
The10,000-transition trace uses accepted outputs of the unchanged pointer core.
Actual profiles,inputs,negative controls and compiler checks are in the
[receipt](../development/window-interaction-evidence.json).
This does not execute every earlier release-case inventory or qualify a boot
window,dirty compositor,hardware motion,whole-ROM/128K budget or target input.
