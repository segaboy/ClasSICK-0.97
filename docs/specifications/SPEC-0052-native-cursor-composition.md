# SPEC-0052: Native cursor save/restore/composition contract

Status: Reviewed original engineering contract2026-10-19;ADR-0060 / SRC-0316..0318 / TEST-0062. Shared helper implemented2026-10-20 (IMPL-0055/TEST-0063);platform binding/art deferred. Baseline: public 3bdc896aafb4ce7e449a229a31407a1bf0c73d89.
Targets: proposed ROM0.4.0-alpha.1 / PC0.2.0-alpha.1, using the same native portable C on 68000 and x86.
SRC-0319 accepts the shared helper and hosted/compiler/CI verification;no platform binding,target trial or release.
It does not change any of these: runtime/surface/port headers, released ROM0.3.0-alpha.1/PC0.1.0-alpha.1, the 24 originals, the held brief, UART, the wiki, or SPEC-0051.
All historical, source, role, edition, physical, B2, native-app and System1 gates still apply. The Mac motion adapter is still blocked at M1-M7. That block does not block this design.
This spec does not cover: a window manager, guest ABI, loader, regions, SDK, band renderer, MMIO/volatile binding, artwork, fixture bytes or code.

## 1. Model

- The helper composes one cursor over one borrowed, full-frame `cs_surface` in ordinary RAM. Frame limits: 1<=width,height<=8192.
- Logical canvas is 512x342. Origin: `ox = W>512 ? (W-512)/2 : 0` and `oy = H>342 ? (H-342)/2 : 0`, both floored. A smaller frame is anchored top-left and clipped.
- `cs_qd_view` (128x64) is fixture-only. It is not the canvas. It is not used here.
- The helper never calls back, never redraws the scene, never allocates, and uses no globals or retained stack objects. It needs no libc copy (byte loops are allowed).
- All mutating calls are serialized by one owner. They are never called from an IRQ and never reentrant. Breaking this is a caller lifetime breach (section 8).

## 2. Native types (`core/graphics/cursor.h`)

```c
#include "surface.h"
typedef enum {
    CS_CURSOR_OK = 0, CS_CURSOR_ERR_ARGUMENT = 1, CS_CURSOR_ERR_STATE = 2,
    CS_CURSOR_ERR_STALE = 3, CS_CURSOR_ERR_SURFACE = 4, CS_CURSOR_ERR_CAPACITY = 5,
    CS_CURSOR_ERR_ALIAS = 6, CS_CURSOR_ERR_ART = 7, CS_CURSOR_ERR_COORD = 8,
    CS_CURSOR_ERR_EXHAUSTED = 9, CS_CURSOR_ERR_COVERAGE = 10, CS_CURSOR_ERR_BUSY = 11
} cs_cursor_result;
enum { CS_CURSOR_MONO_PAYLOAD = 48, CS_CURSOR_RGBA_PAYLOAD = 1024,
       CS_CURSOR_MAGIC = 0x43535243u /* arbitrary tag */ };
typedef enum { CS_CURSOR_UNBOUND = 1, CS_CURSOR_HIDDEN = 2,
    CS_CURSOR_VISIBLE = 3, CS_CURSOR_REPAIR = 4 } cs_cursor_phase;
typedef struct { uint8_t mask[32]; uint8_t image[32]; } cs_cursor_bits;
typedef struct { const cs_cursor_bits *bits;
    uint8_t width, height, hot_x, hot_y; } cs_cursor_art;
typedef struct { uint32_t binding; uint32_t revision; } cs_cursor_token;
typedef struct {
    const cs_surface *surface; uint8_t *payload;
    uint32_t magic, binding, next_binding, revision, payload_capacity;
    int16_t origin_x, origin_y, frame_w, frame_h;
    int16_t save_left, save_top, save_right, save_bottom;
    uint8_t phase, format, fault, reserved;
} cs_cursor_state;
typedef struct { cs_cursor_token token; cs_rect saved;
    uint8_t phase, fault, reserved[2]; } cs_cursor_info;

cs_cursor_result cs_cursor_init(cs_cursor_state *st);
cs_cursor_result cs_cursor_bind(cs_cursor_state *st, const cs_surface *surface,
    uint8_t *payload, size_t capacity, cs_cursor_token *out);
cs_cursor_result cs_cursor_show(cs_cursor_state *st, cs_cursor_token tok,
    const cs_cursor_art *art, int32_t x, int32_t y);
cs_cursor_result cs_cursor_move(cs_cursor_state *st, cs_cursor_token tok,
    const cs_cursor_art *art, int32_t x, int32_t y);
cs_cursor_result cs_cursor_hide(cs_cursor_state *st, cs_cursor_token tok);
cs_cursor_result cs_cursor_advance(cs_cursor_state *st, cs_cursor_token tok,
    cs_cursor_token *out);
cs_cursor_result cs_cursor_discard(cs_cursor_state *st, cs_cursor_token tok,
    cs_rect *out_old);
cs_cursor_result cs_cursor_repaired(cs_cursor_state *st, cs_cursor_token tok,
    cs_rect covered, cs_cursor_token *out);
cs_cursor_result cs_cursor_unbind(cs_cursor_state *st, cs_cursor_token tok);
cs_cursor_result cs_cursor_abandon(cs_cursor_state *st, cs_rect *out_old);
cs_cursor_result cs_cursor_query(const cs_cursor_state *st, cs_cursor_info *out);
```

## 3. Layout derivation (<=64 B on both ABIs)

| Object | ILP32 (incl. m68k 2-byte int alignment) | LP64 |
|---|---|---|
| `cs_cursor_state` | surface 0, payload 4, magic 8, binding 12, next 16, revision 20, capacity 24, origin 28/30, frame 32/34, save 36..42, bytes 44..47 = **48** | ptrs 0/8, magic 16, binding 20, next 24, revision 28, capacity 32, origin 36/38, frame 40/42, save 44..50, bytes 52..55 = **56** (align 8) |
| `cs_cursor_bits` (art) | 64, align 1 | 64 |
| `cs_cursor_art` descriptor | 4+4 = 8 | 8+4+pad 4 = 16 |
| `cs_cursor_token` | 8 | 8 |
| `cs_cursor_info` | 8+16+4 = 28 | 28 |
| borrowed `cs_surface` (not copied) | 24 | 40 |

This table is a theoretical derivation, not an executed proof. No padding is expected inside the state on either ABI. Future builds must verify it (section 10). The state has no inline payload, so the 1024 B RGBA buffer is never paid by a Mac union or by default. The implementation must assert `sizeof(cs_cursor_state)<=64` and `sizeof(cs_cursor_bits)==64` at compile time.
Costs are kept separate:
- Persistent metadata: 48/56 B.
- Caller-owned payload: Mono 48 B, RGBA 1024 B. Only the format's required size is used.
- Borrowed art: 64 B + 8/16 B.
- Borrowed descriptor: 24/40 B.
- Mono restore masks: 0 B persistent. They are computed per row.
- Temporary stack: theoretical target <=96 B of locals per call. Not measured; to be verified in future builds.

## 4. Art format

- Row r (0..15) is bytes `[2r]` and `[2r+1]`. Column c is bit `7-(c%8)` of byte `2r+c/8`. Order is MSB first and does not depend on endian.
- mask 1 = opaque. Opaque pixels write `image`: bit 1 = Mono value 1 / RGBA bytes R,G,B,A = 00,00,00,FF (black); bit 0 = Mono 0 / RGBA bytes FF,FF,FF,FF (white), as SPEC-0001 supplies. The bytes written are exactly what `cs_surface_fill` writes for that value.
- mask 0 = transparent, and the pixel is unchanged. mask 0 with image 1 is refused (no XOR/invert mode).
- Valid art: bits non-NULL; 1<=width,height<=16; hot_x<width; hot_y<height. Every mask/image bit at a column >=width or row >=height must be 0. All64 bytes are checked for accepted art;invalid art may refuse early. Only mapped in-bounds art pixels participate in drawing; validation still reads the full 64-byte block.

## 5. Geometry

- Pointer (x,y) must satisfy 0<=x<512 and 0<=y<342. Otherwise ERR_COORD.
- Logical rect is half-open `[x-hx, x-hx+w) x [y-hy, y-hy+h)`. It is clipped to 512x342, translated by the origin, then clipped to `[0,W)x[0,H)`. Art pixel (i,j) maps to frame (ox+x-hx+i, oy+y-hy+j).
- A result may be empty only on undersized frames. Then the phase becomes VISIBLE with an empty saved rect, and no payload or pixel bytes are touched.
- Saved rect fields are int16 frame coordinates. An empty rect is stored as 0,0,0,0.
- PC margins and every pixel outside the clipped rect are never written.
- Mono row footprint: bytes `l/8 .. (r-1)/8`, at most 3 per row, at most 48 total. Payload rows are packed densely.
  - Save copies whole bytes.
  - Restore writes `dst=(dst&~m)|(saved&m)`, where m covers only columns [l,r).
  - Draw changes only opaque bits.
  - Bits beyond the frame width in the last byte, stride padding, and storage tail are never written.
- RGBA: payload holds 4*(r-l) bytes per row. Restore is an exact byte copy, so it does not depend on endian.
- Save writes only the densely packed saved-rectangle prefix of payload; the rest of the admitted capacity stays unchanged. Hide/restore does not zero payload bytes.

## 6. Validation, precedence and atomicity

- Native inputs must be initialized values: token, art fields, x/y, covered, capacity, and the state (for every call except init). Outputs (`out`, `out_old`, query `out`) need no prior value and are never read.
- On OK every defined output field is written, including `reserved` (written as 0). Padding is unspecified. On any refusal, no output, state, payload or pixel byte is written.

One precedence for every call. The first failing step returns, and later steps read nothing:
1. ARGUMENT: a required pointer is NULL: st always; out/out_old where the call has one; art for show/move; surface and payload for bind.
2. ALIAS (fixed objects): a fixed range wraps, or a writable fixed range overlaps another fixed range. Fixed ranges: [st,+sizeof state), each output object, [surface,+sizeof cs_surface) for bind, [art,+sizeof art) for show/move. No field is read before this step.
3. STATE: the stored state is not canonical (6.1), or its phase is in the call's STATE column (section 7).
4. STALE: token binding or revision differs from the state's (token calls only).
5. BUSY: the phase is in the call's BUSY column.
6. EXHAUSTED: bind with next_binding==0; advance at revision 0xFFFFFFFF; show with fault==1.
7. For show/move/hide, first check the retained descriptor fixed range for wrap and overlap with state/current fixed inputs, returning ALIAS before dereferencing it; bind already did this at step2. Then SURFACE (bind/show/move/hide): re-read the live descriptor's checked fields:
   - storage is non-NULL; format is 1 or 2; 1<=W,H<=8192;
   - stride>=ceil(W/8) for Mono, or stride>=4W for RGBA;
   - stride*H<=storage_size, using checked size_t arithmetic;
   - after bind, width/height/format equal the bound snapshot.
8. CAPACITY (bind): capacity < required (48 Mono, 1024 RGBA) or capacity > 0xFFFFFFFF. The admitted value is stored exactly in payload_capacity. Only the required prefix is used.
9. For show/move, null art->bits returns ART. Then ALIAS (derived ranges, bind/show/move/hide only): any wrap, or any overlap among [storage,+storage_size) (the whole truthful size, including stride padding and tail), [payload,+payload_capacity), [art->bits,+64) (show/move) and fixed state/descriptor/art/output ranges (including the retained surface descriptor). Read-only inputs may overlap each other. No payload, art-bit or pixel byte is read before this step.
10. ART (section 4), then 11. COORD (section 5), then 12. COVERAGE (repaired).

- Bind preflight uses the supplied descriptor, payload and admitted capacity; later pixel calls use the stored references/capacity. Retained descriptor range checks at step7 precede its field reads.
- Once every check has passed, a pixel call cannot fail, provided the ordinary-live-RAM preconditions hold. All writes happen after all checks.
- Query, discard, advance, unbind and abandon never dereference surface, payload or art. They do not validate all borrowed-array/output overlap: disjointness from live descriptor, storage, payload and art is a truthful owner precondition on these calls. The state and every writable output remain mechanically checked for fixed-range overlap.
- Ranges are compared as `uintptr_t`, under the established flat-address target assumption for 68000 and x86 (implementation-defined in C). The checks prove arithmetic and range consistency only. They do not prove mapping, lifetime, identity or security. There is no recovery from arbitrary unmapped pointers.

### 6.1 Init and canonical state

- `cs_cursor_init`: NULL gives ARGUMENT, and a wrapping [st,+sizeof state) gives ALIAS, both before any write. On OK it writes every field and reads none: magic, phase UNBOUND, next_binding 1, everything else 0/NULL.
- Init is valid only on fresh memory, or on a quiescent UNBOUND state whose earlier tokens, state copies and aliases are all retired. Init over a HIDDEN, VISIBLE or REPAIR state is an undetected caller breach. It is never an identity reset for a live cursor.
- Every other call requires initialized native state fields. Never-initialized memory violates that precondition; the helper cannot identify or safely diagnose indeterminate values. The canonical check reads only state fields:
  - all phases: magic==CS_CURSOR_MAGIC; reserved==0; phase is 1..4.
  - UNBOUND: surface and payload are NULL. binding, revision, payload_capacity, format, fault, origin, frame and save are all 0. next_binding may be any value (0 = exhausted).
  - bound (HIDDEN/VISIBLE/REPAIR):
    - surface and payload are non-NULL; format is 1 or 2; binding!=0;
    - next_binding == (binding==0xFFFFFFFF ? 0 : binding+1); revision>=1;
    - payload_capacity>=required, and [payload,+payload_capacity) does not wrap;
    - 1<=frame_w,frame_h<=8192, and origin equals the section 1 formula;
    - fault is 0 or 1. fault==1 only with revision 0xFFFFFFFF in HIDDEN or REPAIR.
  - save rect: all 0 in HIDDEN. In VISIBLE or REPAIR it is either all 0, or a nonempty half-open rect inside both the frame and translated logical canvas, with each extent <=16. Explicitly, origin_x<=left<right<=min(frame_w,origin_x+512), origin_y<=top<bottom<=min(frame_h,origin_y+342).

## 7. Lifecycle

Section 6 gives the precedence. Every phase falls in exactly one of the Runs, BUSY or STATE columns.

| Call | Runs | BUSY | STATE | Effect on OK |
|---|---|---|---|---|
| bind | UNBOUND | - | bound phases | Stores surface, payload, payload_capacity (admitted), format, frame_w/h and origin. binding=next_binding; next_binding = binding==0xFFFFFFFF ? 0 : binding+1. revision 1, fault 0, save rect 0. HIDDEN. out={binding,1}. |
| show | HIDDEN | REPAIR | UNBOUND, VISIBLE | Saves the clipped background, then draws. VISIBLE. |
| move | VISIBLE | REPAIR | UNBOUND, HIDDEN | Restores old, saves new, draws new. Handles overlapping rects and art changes. The scene is not redrawn. |
| hide | VISIBLE | REPAIR | UNBOUND, HIDDEN | Restores the saved rect. Save rect set to 0: the payload is logically retired, its bytes are not zeroed. HIDDEN. |
| advance | HIDDEN | VISIBLE, REPAIR | UNBOUND | revision+1. out token. |
| discard | VISIBLE, HIDDEN | REPAIR | UNBOUND | No surface, payload or pixel dereference. out_old = saved rect (empty from HIDDEN); the rect is kept. REPAIR. The payload is dead. |
| repaired | REPAIR | - | UNBOUND, HIDDEN, VISIBLE | `covered` needs left<=right, top<=bottom, and must contain the saved rect (an empty rect is always contained); otherwise COVERAGE. Save rect 0. Below max: revision+1. At max: revision kept, fault=1. HIDDEN. out={binding,revision}. |
| unbind | HIDDEN | VISIBLE, REPAIR | UNBOUND | No dereference. Writes canonical UNBOUND (6.1), keeping magic and next_binding. The binding id is consumed. |
| abandon | bound phases | - | UNBOUND | No token, no dereference. out_old = saved rect (0 if none), then canonical UNBOUND keeping magic and next_binding. |
| query | all | - | - | Refused only if non-canonical. No dereference. out: token {binding,revision} ({0,0} when UNBOUND), saved rect (0 if none), phase, fault, reserved 0. |

Borrowing and identity:
- The descriptor fields and descriptor/storage/payload allocation identities remain live, stable and unchanged from bind until normal unbind. Pixel contents may change only under the owner scene/repair protocol; saved payload bytes are helper-owned while VISIBLE. Abandon is the exceptional expired/quarantined-storage path below. The width/height/format snapshots catch changes to those fields only. They do not prove storage pointer, stride or storage identity.
- Art descriptors and bits are borrowed only during show/move. They are never retained.
- Tokens are per-state consistency labels, not security or global identity. A numerically equal token from another state is an invalid caller association and may go undetected. A bound state must never be copied or moved.

Owner protocol:
- **Normal scene, counter or window mutation:** hide, then mutate the scene (owner redraws), then advance, then show with the new token.
- **Pointer motion only:** move.
- **Scene write while VISIBLE:** a protocol violation that the owner must diagnose. It is not language-level undefined behavior. Recover with discard, then repair. Never call hide or move afterwards: that would restore stale pixels.
- **Recovery (VISIBLE, or HIDDEN after a partial redraw or pipeline fault):** call discard; from HIDDEN, out_old is empty. Fully repaint out_old plus the complete scene dirty area, even when out_old is empty. Then call repaired with a truthful `covered`. The helper cannot detect arbitrary writes or false acknowledgements. After a normal hide, check the revision before scene mutation: at max, rotate the binding while HIDDEN before writing. If mutation already happened when advance refuses, retain HIDDEN, use discard/repaired to lock out show, then unbind/rebind; never reuse the old token to display the changed scene.
- **Revision exhaustion:** advance gives EXHAUSTED. repaired at max sets fault=1 and HIDDEN, after which show gives EXHAUSTED. Recover with unbind, then bind; the new id comes from the counter. A full repaint alone never revives a token.
- **Binding exhaustion (next_binding==0):** bind gives EXHAUSTED. init is allowed only from a quiescent UNBOUND state with all tokens, state copies and aliases retired. REPAIR is not quiescent, and a full repaint alone is no proof.
- **Retiring storage:** hide (or discard+repaired) while storage is live, then unbind. abandon is only for storage that has expired or is permanently quarantined and no longer presented. It is never a shortcut that leaves a cursor on a reused live surface.
- **Stale or lost token:** query returns the current token and phase. The owner chooses explicitly between using it, discard/repaired, or abandon. There is no deferred recovery.

## 8. Fault classes

- **Validated refusal:** a result other than OK, with no effects. A pointer transition the owner already accepted under SPEC-0051 stays accepted. The owner records the cursor refusal, keeps the old data and effects, and repairs explicitly. It never rolls back or fabricates input.
- **Checked live-field corruption:** a still-mapped descriptor whose width/height/format changed, or a state whose initialized fields are non-canonical. These are refused (SURFACE or STATE) with no effects.
- **Caller precondition breach (not detected; no safety guarantee):**
  - expired, unmapped or replaced descriptor/storage/payload while bound;
  - a change to unchecked descriptor fields (storage, stride, storage_size);
  - art mutated during a call; concurrent, IRQ or reentrant calls;
  - never-initialized state; init over a live state; a bound state copied or moved;
  - a foreign token; a false `repaired` call;
  - outputs overlapping live storage/payload on calls that do not dereference them.
- **Owner-diagnosed protocol violation:** a scene write while VISIBLE. Recover with discard/repaired (section 7).
- **Post-acceptance pipeline fault:** the owner's scene redraw fails after hide or discard. The owner keeps the cursor HIDDEN or in REPAIR and must not call show until it has repaired.

## 9. Original proposed case inventory (TEST-0062;not an execution record)

Notation: M512 = Mono 512x342, stride 64, size 21888, payload cap 48. Canary 0xA5 fills tails and padding. Art A16 = 16x16, full mask, image checker, hot(0,0). Art A1 = 1x1, mask1, image1.

| ID | Trace | Expected |
|---|---|---|
| C01 | init; query | UNBOUND, token{0,0}, empty rect |
| C02 | bind M512 | OK, {1,1}, origin(0,0), HIDDEN |
| C03 | bind RGBA 640x480, stride 2560 | OK, origin(64,69) |
| C04 | bind Mono 513x343, stride 65 | origin(0,0) by floor |
| C05 | bind Mono 300x200, stride 38 | origin(0,0); later clips at 300/200 |
| C06 | bind M512 cap47; LP64 cap0x100000000 | CAPACITY each; no truncation or writes. The inclusive UINT32_MAX admission bound needs arithmetic review, not a fake claimed4GiB allocation |
| C07 | stride 63; width 8193; size stride*H-1 | each SURFACE, no change |
| C08 | payload overlaps last storage byte; state inside storage | ALIAS, payload/pixels unchanged |
| C09 | NULL surface and cap 0 together | ARGUMENT, by precedence |
| C10 | show art: image bit with mask 0; w=17; hot_x=w; bit at column >=w | each ART, HIDDEN, pixels unchanged |
| C11 | show A1 at (0,0) on M512 bg 0x00 | byte0 row0 = 0x80, payload[0]=0x00, other bits unchanged |
| C12 | show A16 at x=8k+r, r=0..7, bg 0x5A | footprint 2 bytes (r=0) or 3 bytes; bits outside [x,x+16) keep 0x5A; hide restores exactly |
| C13 | show A16 separately at all four logical corners on M512 | rects [0,0,16,16),[511,0,512,16),[0,341,16,342),[511,341,512,342); mapped art starts (0,0) |
| C14 | art 16x16, hot(15,15), at (0,0) | 1x1 at (0,0) using art pixel (15,15) |
| C15 | art with transparent ring | ring pixels unchanged; restore correct |
| C16 | RGBA 640x480, distinct bytes per pixel; show, hide | exact bytes restored; x<64 margins untouched |
| C17 | Mono 509x342, stride 66; A16 at (500,10) | clip at 509; bits 509..511 and bytes 64..65 keep canary |
| C18 | C05 frame, pointer (400,300) | VISIBLE, empty rect, 0 bytes touched; hide OK |
| C19 | show (100,100); move (104,102) | new payload is background, not cursor; non-cursor pixels = original |
| C20 | move same position, A16 then 8x8 art | 8x8 drawn; rest of old area restored |
| C21 | show; show; hide; hide | OK; STATE; OK; STATE; no change on refusals |
| C22 | hide; product input counter repaint; advance; show {1,2} | OK, cursor over new counter |
| C23 | show with {1,1} after C22 | STALE |
| C24 | advance while VISIBLE | BUSY |
| C25 | show; discard; show; repaired missing 1 column; repaired full; show | OK; out_old=rect, REPAIR; BUSY; COVERAGE; OK HIDDEN rev+1 rect 0; OK with new token |
| C26 | seed HIDDEN {1,0xFFFFFFFF}; advance; show; discard; repaired full; show; unbind; bind | EXHAUSTED; OK; OK; OK fault1 HIDDEN rev kept; EXHAUSTED; OK; OK {2,1} |
| C27 | show; unbind; hide; unbind; hide old token | OK; BUSY; OK; OK; STATE |
| C28 | abandon after storage dead (by reasoning, no deref); bind | UNBOUND; next id 2 |
| C29 | VISIBLE; live mapped descriptor width changed; move; hide; width restored; hide | SURFACE; SURFACE; no writes, VISIBLE kept; OK |
| C30 | zeroed state; hide | STATE (magic) |
| C31 | seed UNBOUND next 0xFFFFFFFF; bind; unbind; bind | OK {0xFFFFFFFF,1}, next 0; OK; EXHAUSTED |
| C32 | query with out==st; discard with out_old overlapping st; bind with out inside storage | ALIAS each, no change |
| C33 | seed HIDDEN fault1 rev 5; reserved 1; VISIBLE rect 17 wide; then query | STATE each |
| C34 | HIDDEN; discard; repaired empty; show | empty out_old, REPAIR; OK HIDDEN rev+1; OK |

Pointer cancel has no implicit pixel operation: the later owner may explicitly hide or retain the cursor; accepted input is never rolled back. C12 uses k=0..61 so its unaligned spans remain in frame.

C26, C31 and C33 assume a PROPOSED future test-only method that writes an explicit, initialized state (canonical or deliberately corrupted). This contract generates no tests, fixtures or seeding code.

## 10. Future verification (not passed)

Record the following on actual 68000 and x86 builds before acceptance:
- ROM code and data size against the 64 KiB ROM;
- RAM per object (section 3);
- worst-case stack;
- compiler/ABI `sizeof`/`offsetof` per section 3;
- the <=96 B temporary-stack target.

The section 3 layout and the stack target are theoretical. Nothing here was built or executed.

This is not a full 128K System certification.


## Helper verification checkpoint -2026-10-20

[IMPL-0055/TEST-0063](../../provenance/records/IMPL-0055-pointer-cursor.md) records actual helper checks;the inventory above is not a blanket claim that every row ran. Compile-time48/56B layouts and per-function native compiler frames are separate from the96B theoretical local-workspace target. No whole-system stack/ROM/128K or target qualification is implied.
