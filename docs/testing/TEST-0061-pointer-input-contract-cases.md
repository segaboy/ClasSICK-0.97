# TEST-0061: Pointer/input contract cases (UNEXECUTED)

All cases target SPEC-0051. None has been executed, and this list is not
evidence. Notation:

- S = state (x, y, down, suppress), with valid implied.
- Fresh = begin(0), giving S(256,171,0,0).
- Flags: M=MOVED, P=PRESS, R=RELEASE, D=DOWN, Su=SUPPRESSED,
  C=CANCELLED, Cx/Cy=CLAMP.
- Refusal sentinel:any invalid argument/state setup retains every pre-call p/out byte;valid-state tests keep p admitted and prefill out only. Alias/range checks precede dereference;unreadable native addresses are outside caller guarantees.

## Core

| ID | Setup / input | Expected |
|---|---|---|
| C01 | begin(p,0) | OK; S(256,171,0,0) |
| C02 | begin(p,1) held | OK; S(256,171,0,1); no press |
| C03 | begin(p,6) | OK; suppress 0 (right and middle not actionable) |
| C04 | begin(p,8) on sentinel | ARGUMENT; p unchanged |
| C05 | begin(NULL,0) | ARGUMENT |
| C06 | Fresh; (0,0,0) on sentinel out | OK; out (256,171,flags0,b0) fully written |
| C07 | Fresh; (10,5,0) | CHANGED; M; (266,176) |
| C08 | Fresh; (INT32_MAX,INT32_MAX,0) | CHANGED; M,Cx,Cy; (511,341) |
| C09 | Fresh; (INT32_MIN,INT32_MIN,0) | CHANGED; M,Cx,Cy; (0,0) |
| C10 | Fresh; (INT32_MAX,INT32_MIN,0) | M,Cx,Cy; (511,0) |
| C11 | S(511,341,0,0); (1,1,0) | OK; Cx,Cy only; unchanged |
| C12 | S(0,0,0,0); (-1,0,0) | OK; Cx only |
| C13 | S(0,0,0,0); (511,341,0) | M; (511,341); no clamp |
| C14 | S(0,0,0,0); (512,342,0) | M,Cx,Cy; (511,341) |
| C15 | Fresh; (4,-3,1) | CHANGED; M,P,D; (260,168); press at moved position |
| C16 | After C15; (0,0,1) | OK; D only |
| C17 | After C16; (0,0,0) | CHANGED; R; down 0 |
| C18 | begin(1); (0,0,1) | OK; Su; down 0, suppress 1 |
| C19 | After C18; (0,0,0) then (0,0,1) | CHANGED, no edge; then CHANGED P,D |
| C20 | Fresh; (0,0,2), then (0,0,4), then (0,0,6) | OK each; out.buttons echoes the value; no P |
| C21 | Fresh; (0,0,3) | P,D; out.buttons 3 |
| C22 | Fresh; (5,5,8) on sentinel | ARGUMENT; p and out unchanged; no motion |
| C23 | Fresh; buttons 0x80000000 | ARGUMENT; atomic |
| C24 | valid S(x,y,1,0);cancel | CHANGED; R,C,Su;buttons0; S(x,y,0,1);counter count unchanged |
| C25 | valid down0;cancel | CHANGED; C,Su;buttons0;suppress1 |
| C26 | After C24; (0,0,1) | Su; no P; down 0 |
| C27 | After C26; (0,0,0) then (0,0,1) | then P,D (re-press at current position) |
| C28 | valid down1,suppress0;cancel x3 | CHANGED each;first R,C,Su;then C,Su twice;one owner release |
| C29 | S(100,100,1,0); cancel | position kept at (100,100) |
| C30 | Counter presses UINT32_MAX-1; press edge | counter UINT32_MAX; P published |
| C31 | Counter at MAX; release then press | P still published; window toggles; counter stays MAX |

## Refusal, state and aliasing

| ID | Setup / input | Expected |
|---|---|---|
| R01 | Stored x=512 | STATE; p and out unchanged |
| R02 | Stored y=-1, then y=342 | STATE; atomic |
| R03 | down=2 or suppress=2 | STATE |
| R04 | down=1 and suppress=1 | STATE |
| R05 | valid!=marker (never begun) | STATE; cancel also STATE |
| R06 | out==p, s overlaps out, s overlaps p | ARGUMENT; atomic |
| R07 | NULL s, NULL out, cancel(p,NULL) | ARGUMENT; p unchanged |
| R08 | Fresh p;separate out initially has P;(0,0,0) | OK;flags exactly0;no stale P |

## Adapter (PC decoder to core)

| ID | Setup / input | Expected |
|---|---|---|
| A01 | Proposed adapter maps admitted dy=+3 | Normalized dy=-3;logical pointer up;native mapping still requires binding evidence |
| A02 | Packet dx=-256, dy=255 | dx -256, dy -255; no UB |
| A03 | DROPPED while down | No sample;cancel -> R,C,Su;buttons0;no P/motion;next held sample suppressed |
| A04 | Only old Mac button samples available | Does not qualify motion/parity;new binding awaits reviewed sources |

## Cursor composition (future renderer)

| ID | Setup / input | Expected |
|---|---|---|
| K01 | MONO1_MSB,x-hx=3,16x16 | Save<=48B;metadata design cap64B to prove later;partial-byte neighbors preserved |
| K02 | RGBA8 16x16 | Save<=1024B;mask+image<=64B separately;no screen buffer duplicate |
| K03 | Pointer (0,0), hotspot (8,8) | Clipped 8x8 rect; no out-of-bounds writes |
| K04 | Pointer (511,341), hotspot (0,0) | Clipped 1x1 |
| K05 | Window mutation under cursor | Valid old save restored before mutation,or stale slot discarded and full old-overlay+dirty-area repaired;then save/draw |
| K06 | NULL or stride-mismatch binding | Refused; zero pixel or slot writes |
| K07 | Revision identity exhausted/ambiguous | No save or stale restore;quiescent retirement/rebind requires next contract;repaint alone insufficient |
| K08 | Cursor over window and counter | Cursor drawn last; restore before dirty repaint |

## Parity and evidence classes

| ID | Setup / input | Expected |
|---|---|---|
| E01 | Same admitted sample script on native68000/x86-64 | Equal decoded state/event fields;future normalized512x342 pixels;not native padding/endian bytes |
| E02 | Prior headless device API versus future automated host GUI | Prior is E2 only;future E3 must record real GUI capture/focus;neither is E4human |
| E03 | Human click trial | Separate class; requires an authorized new run |

Additional boundary cases,unexecuted: R09 null+bad-state=>ARGUMENT precedence;R10 invalid buttons+bad-state=>ARGUMENT before STATE;R11 wrapping native object range=>ARGUMENT before dereference under supported flat-address model. A05 invalid decoder range/buttons=>binding fault,never masked/negated blindly. K09 hotspot outside actual art=>refusal;K10 old surface retired with live save=>owner breach/refusal before old-storage access. C32 suppress-clear writes all defined output fields with no edge;owner failure after core acceptance retains pointer state,no rollback.

W01 future-window consumer:an active drag receives MOVED|RELEASE with DOWN0;apply final moved position before ending the drag. W02 PRESS|MOVED hit-tests/anchors at the final pointer position,without replaying the movement as an old drag. Both are unexecuted consumer requirements,not implemented Window Manager tests.

Owner boundary cases,unexecuted: O01 active old counter.down1/live drag cannot be reset by silently re-beginning only p;this violates owner quiescence,not a detectable core-only STATE error. O02 coordinated begin(buttons1),counter.begin(0),held1,release0,press1=>no initial press,then one PRESS;mirror.down matches pointer.down after each step. O03 at MAX,admitted PRESS/RELEASE/PRESS=>mirror.down1/0/1,counts remain MAX;two PRESS events survive. O04 suppress1/sample buttons2=>CHANGED,suppress0,flags0,buttons2;repeat=>OK,flags0;owner uses flags,not CHANGED alone,to choose redraw/action. R12 wrapping object range+bad buttons=>ARGUMENT before reading fields;begin(NULL,8)=>ARGUMENT. Every owner trace checks mirror.down and unchanged count on RELEASE/cancel.
