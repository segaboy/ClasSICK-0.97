# SPEC-0051: Native pointer/input semantic core

Status: reviewed engineering contract for future ROM0.4.0-alpha.1 / PC0.2.0-alpha.1,2026-10-19;ADR-0059. Helper implemented2026-10-20 (IMPL-0055/TEST-0063);platform bindings deferred. Original contract basis:
PUBLIC HEAD 14a2c31. Published ROM0.3.0-alpha.1 and PC0.1.0-alpha.1 stay
immutable. System is unreleased. Every behavior here is an original engineering
choice. It makes no claim about January 1984 Macintosh behavior, and later
manuals never establish such behavior.

## 1. Scope

This contract covers one portable C semantic core, built natively for 68000
and x86-64. The core turns normalized pointer samples into exactly one
transition per call. A single input owner then gives that transition to the
press counter, the one fixed-size draggable window and the cursor compositor.
The core does not include a guest trap ABI, an EventRecord queue, timestamps,
acceleration, globals/A5, a window manager or rendering code. The cursor renderer has its separate SPEC-0052 contract; section 9 records its
original composition requirements. No hardware-register algorithm is selected here (section 8).

## 2. Coordinates

- Logical canvas: 512x342 (`CS_POINTER_WIDTH`, `CS_POINTER_HEIGHT`).
  Origin is the top-left. +x moves right and +y moves down. On PC the canvas
  stays centered as described in [the display target](../development/display-target.md).
- Valid stored position: 0<=x<=511 and 0<=y<=341, as int32.
- Initial position at begin and at every re-begin: (256,171), the integer
  center. Cancel never moves the pointer.

## 3. Native API (core/input/pointer.h)

```c
typedef struct { int32_t x, y; uint32_t down, suppress, valid; } cs_pointer;
typedef struct { int32_t dx, dy; uint32_t buttons; } cs_pointer_sample;
typedef struct { int32_t x, y; uint32_t flags, buttons; } cs_pointer_event;
enum { CS_POINTER_OK = 0, CS_POINTER_CHANGED = 1,
       CS_POINTER_ARGUMENT = 2, CS_POINTER_STATE = 3 };
enum { CS_POINTER_LEFT = 1u, CS_POINTER_RIGHT = 2u, CS_POINTER_MIDDLE = 4u,
       CS_POINTER_BUTTON_MASK = 7u };
enum { CS_POINTER_MOVED = 1u, CS_POINTER_PRESS = 2u,
       CS_POINTER_RELEASE = 4u, CS_POINTER_DOWN = 8u,
       CS_POINTER_SUPPRESSED = 16u, CS_POINTER_CANCELLED = 32u,
       CS_POINTER_CLAMP_X = 64u, CS_POINTER_CLAMP_Y = 128u };
#define CS_POINTER_VALID 0x50544E31u /* begun marker, not a security token */
int cs_pointer_begin(cs_pointer *p, uint32_t buttons);
int cs_pointer_sample_apply(cs_pointer *p, const cs_pointer_sample *s,
    cs_pointer_event *out);
int cs_pointer_cancel(cs_pointer *p, cs_pointer_event *out);
```

These values contain no native pointers, so they are relocatable. The state is
owned by the caller and there is no global or singleton. The core uses no
allocation and no libc beyond `<stdint.h>`/`<stddef.h>` types.
cs_mouse_button and its ABI are reused unchanged (section 6).

## 4. Admission and validation (all before any write)

Every check is completed before the first store. On any refusal, no byte of
`*p` or `*out` changes.

Validation precedence is the numbered order below,restricted to arguments actually present in each routine.
1. A NULL required `p`, `s` or `out` returns ARGUMENT.
2. Aliasing returns ARGUMENT: the byte ranges of `p`, `s` and `out` must not
   overlap pairwise. Supported native targets use flat uintptr_t ranges;check each end without wrapping before comparing ranges. A wrapping range is ARGUMENT before a dereference. This catches ordinary overlap misuse;it does not prove mapping,lifetime,alignment,absence of concurrent aliases or safe access to an arbitrary unreadable address.
3. Buttons with bits outside `CS_POINTER_BUTTON_MASK` return ARGUMENT. This
   applies to begin and to sample.
4. For sample and cancel, the stored state must pass these checks, or the
   call returns STATE:
   - `valid==CS_POINTER_VALID`
   - x and y are in range
   - `down` and `suppress` are each 0 or 1
   - they are not both 1
5. Begin never reads the old `*p`. It validates its arguments, then writes
   every defined state field. Success does not specify padding bytes or a cross-CPU memory representation.

Callers must pass live,aligned,owned,truthful readable/writable objects. Forged addresses and hidden aliases are caller breaches,not recoverable STATE errors. Begin starts a fresh/quiescent owner session;it is not a live-drag reset. The owner must retire old cursor/window/counter state before re-begin. Readable state/input fields have initialized C values;STATE cases use explicit invalid values,not indeterminate automatic variables. Begin needs no prior state values,and out needs no prior field values. Pointers are
borrowed only for the duration of a call and are never retained. All calls
for one state run on one thread and are not reentered. Interrupt handlers
may capture/queue bounded raw records under a later adapter contract;they never call the semantic core. No byte-only interrupt representation is imposed on the unreviewed Mac motion path.

## 5. Sample semantics (one call = one transition)

Order: motion is applied first, then the button is evaluated, then the event
is published. A press or release therefore happens at the moved position.

- Motion: clamp without overflow, using no int64 and no negation. For dx>0:
  `x = (dx > 511-x) ? 511 : x+dx`. For dx<0: `x = (dx < -x) ? 0 : x+dx`.
  The same rule applies to y with bound 341. INT32_MIN and INT32_MAX are
  safe. MOVED is set if x or y changed. CLAMP_X/CLAMP_Y is set if the request
  was cut short, including a push against an edge the pointer is already on.
- Button: L = buttons & LEFT. RIGHT and MIDDLE are accepted, copied to
  `out.buttons`, and are not actionable.
  - suppress=1, L=1: no edge, `down` stays 0 and suppress stays1.
  - suppress=1, L=0: suppress is cleared. No edge.
  - suppress=0, down=0, L=1: down=1, PRESS.
  - suppress=0, down=1, L=0: down=0, RELEASE.
  - Otherwise there is no edge.
- Every successful sample writes all defined event fields:final x/y,flags and accepted buttons. DOWN and SUPPRESSED mirror final down/suppress on every event;all other flags start clear for this call. Raw buttons are telemetry,not an alternative edge source. Consumers act on PRESS/RELEASE/CANCELLED,never resample raw LEFT while suppressed. Padding has no cross-CPU meaning.
- Return CHANGED if MOVED, PRESS, RELEASE or a suppress change happened.
  Otherwise return OK. A zero-motion, unchanged sample is OK with a
  deterministic `out`.
- On both OK and CHANGED,all defined output fields are valid and the owner observes the event once. The return code is not a repaint command or a substitute for flags;repeated cancel is CHANGED even if state fields already match. A suppressed state given buttons2 has observed LEFT0:it clears suppression and returns CHANGED,not OK.

## 6. Begin, cancel and resynchronize

- `begin(p,b)`: the position is centered and down=0. suppress=(b&LEFT)?1:0,
  so a button held at begin is never a press. Returns OK.
- `cancel(p,out)`: an explicit uncertainty gate for a release that may have
  been lost.
  - Let olddown be the admitted logical down. Set down=0,suppress=1,keep x/y and publish buttons=0 (unknown physical state,not proof of a physical release).
  - Flags are CANCELLED|SUPPRESSED,plus RELEASE iff olddown=1. Return CHANGED even for a repeated cancel,because it is an explicit cancellation notification.
  - Repeated cancels are valid. Only the first reports RELEASE;neither fabricates a PRESS.
- Resume: the next press requires an observed L=0 sample followed by L=1.
- A cancel release is never a press. Cancel does not promise a full hardware
  resynchronization. It only guarantees that no edge is fabricated.
- Counter: the owner keeps a separate `cs_mouse_button` started with
  `begin(btn,0)`. On PRESS it calls `sample(btn,1)`. On RELEASE (including a
  cancel release) it calls `sample(btn,0)`. Existing saturation at UINT32_MAX
  holds the count,but PRESS still reaches the window. The owner keeps counter.down equal to pointer.down between calls and delivers each edge once. A newly begun pointer uses a freshly coordinated counter baseline0;old button-only profiles are unchanged. Replaying a transition or reentering consumers violates the owner contract;there is no hidden exactly-once deduplication.
- A RELEASE routes sample(btn,0):it sets mirror down0 and never increases presses. Every admitted press/release/cancel sequence must leave counter.down==pointer.down;presses is saturating telemetry,not an event identity. Core begin cannot inspect an external mirror;the later owner protocol must prevent live re-begin or coordinate retirement/counter reinitialization before any new sample.

## 7. Distribution and publication

The owner consumes each normalized sample once and calls the core once. It
publishes the single `out` to the counter, then the window, then the cursor.
Consumers never resample shared button bits. The window rules are:

- PRESS in the title bar starts a drag.
- MOVED while dragging shifts the fixed-size window. The window position is
  clamped by the window consumer, not by this core.
- RELEASE or CANCELLED ends the drag.
- PRESS in the content toggles the pattern.

These are requirements for the later window consumer,not an installed Window Manager. For a drag already active before this event,apply its MOVED final position before ending on RELEASE;final movement must not be lost merely because DOWN is now0. A new PRESS hit-tests the post-motion position and establishes its anchor there. CANCELLED never fabricates movement.

A transition that has been accepted is final. A later draw, transport or
presentation or consumer failure cannot undo it. Stop distribution/mark the owner faulted as the later pipeline contract specifies,retain completed state,effects and output,and require explicit coordinated repair;do not silently call begin and pretend rollback. This contract guarantees atomic refusal only inside each core call.

## 8. Adapter obligations (no register algorithm selected)

- A future PC binding may consume only PACKET results and must validate the decoder's declared dx/dy range[-256,255] and buttons0..7 before conversion;do not hide an invalid decoder result by masking. Never apply abs/negation to an unvalidated arbitrary INT32_MIN.
- The logical contract chooses +y downward. The proposed dy=-packet.dy mapping yields[-255,256] for admitted packets,but native directional source/qualification evidence must confirm the adapter convention before binding;SPEC-0039's bit table alone is not that confirmation. Dx is unchanged under that proposal.
- MORE emits nothing. DROPPED emits no motion/sample and conservatively cancels the logical gesture;an observed released sample is needed before a new press. This is an original loss policy,not complete hardware resynchronization. Existing coordinator ACK/report routing and old profiles are unchanged.
- Mac motion binding remains gated on the lead source-sufficiency review. Button-only dx=dy=0 samples can describe old behavior but cannot qualify release2 motion or silently substitute for its native parity requirement.

## 9. Cursor composition requirements (next contract,not an installed API)

- Original art only:1<=w,h<=16,one1bpp mask and image;0<=hx<w,0<=hy<h. Art is at most64 bytes for two16x16 bitmaps. No artwork is generated here.
- Hotspot follows logical pointer. Draw rect(x-hx,y-hy,w,h) is half-open and clipped through the truthful native view/surface and512x342 logical bounds;an empty clip saves/draws nothing. PC margins remain white.
- The owning compositor must restore a valid previous save before any underlying scene write,then repaint the dirty scene,save fresh background,and draw the cursor on top of window/counter. It must include old and new cursor extents in repair;only the window's new dirty rect is insufficient.
- Exactly one borrowed save-under slot. Pixel payload caps:MONO1_MSB <=16*3=48 bytes for an unaligned16-pixel span;RGBA8 <=16*16*4=1024 bytes. Metadata has a provisional64-byte design cap,to be proven for the actual layout/ABI in the next contract. No extra full-screen buffer. Art,slot,metadata,stack and persistent storage costs are recorded separately,including any Mono restoration bit masks.
- Binding identity,geometry/storage/view lifetime and scene revision must be coherent. Stale background is never restored after scene mutation/rebind. Before retiring old storage,remove its cursor while storage is still owned/live;otherwise refuse/quarantine rather than touching dead memory.
- If a stale slot must be discarded on a still-live surface,repair the old overlay area plus the underlying dirty area before saving/drawing again. Unknown binding or insufficient repair coverage is a refusal/fault,not permission to continue with trails.
- Revision identity cannot silently wrap/reuse. At exhaustion,refuse further cursor save until a later specified quiescent retirement/rebind protocol establishes a fresh identity;full repaint alone does not prove stale aliases safe. Actual binding/revision types and APIs are deferred,not imposed on existing cs_surface/port headers here.
- Invalid binding/stride/size/overlap is refused before pixel/slot writes. A pointer transition already accepted is not rolled back by cursor refusal. Actual clipped restore,partial-byte preservation,dirty repair and error atomicity need the next rendering contract.
- Keep64KiBROM target and record actual native code/data/stack/workspace before acceptance;this is not full128K System certification.

## 10. Evidence separation and gates

The following evidence classes are never merged:

- E1 hosted semantic tests/dispatch and original traces.
- E2 device/API injection:the prior38 mouse+5Space/185guest-second headless pass belongs here;it did not qualify GUI capture/focus.
- E3 automated host-delivered GUI input through the visible VM/viewer,with capture/focus/transform/launch configuration recorded;not established by prior E2.
- E4 actual human GUI input:prior PC clicks failed/intermittent,not a pass. Unavailable human input is recorded as unavailable,never synthesized into E4.

The cause of the GUI preboot CPUID failure remains unknown. Later source admissions,code,exact-image trials and publication retain separate gates. Published-source/role/edition/physical/full128K/B2/native-app/legacy/System1-ROM requirements remain. [TEST-0061](../testing/TEST-0061-pointer-input-contract-cases.md) cases are all unexecuted. The shared pointer API/header is implemented;adapter/window/IRQ bindings remain deferred. Actual helper checks are in IMPL-0055/TEST-0063;no target trial is authorized.
