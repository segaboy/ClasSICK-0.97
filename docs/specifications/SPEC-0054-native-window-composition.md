# SPEC-0054 - Original native window/cursor composition

Finalized engineering design2026-10-20;SRC-0326..0328;ADR-0062.
Profile:isolated single-window512x342 native scene over ordinary-RAM Mono/RGBA.
Evidence strength:independent platform contract,not historical Macintosh evidence.
Implements the renderer only;SPEC-0051/0053 logical transitions remain external.

Stateless compositor over one already-bound cursor/full-frame ordinary-RAM surface. Owner passes truthful live/aligned initialized native objects,exclusive synchronous access,coherent token,unchanged borrowed descriptors/art/base-image for their live binding. No async/interrupt/reentry,arbitrary-pointer readability recovery,raw input or framebuffer allocation. Pointer/window/counter transitions already accepted are final even if rendering refuses;owner must fault/repair rather than rollback/re-begin/replay. This helper does not implement the input owner or hardware presentation.
Header includes ../../core/graphics/cursor.h, ../../core/ui/window.h, ../product-scene/scene.h.
typedef struct {cs_cursor_token token;cs_rect dirty;uint32_t stage;int32_t cause;} cs_window_scene_effect; //32B
statuses CS_WINDOW_SCENE_OK0,ERR_ARGUMENT1,ERR_STATE2,ERR_CURSOR3,ERR_PAINT4,ERR_EXHAUSTED5.
stages CS_WINDOW_SCENE_HIDDEN1,PAINTING2,PAINTED3,ADVANCED4,VISIBLE5.
int cs_window_scene_compose(cs_cursor_state *cursor,cs_cursor_token token,const cs_cursor_art *art,const cs_product_state *product,const cs_window *old_window,const cs_window *window,uint32_t old_inputs,int32_t x,int32_t y,cs_window_scene_effect *out);

Old_window NULL explicitly requests a full initial/repair repaint,only from HIDDEN. Otherwise require VISIBLE and old_window is truthful last-rendered snapshot;product fields other than inputs/pixels contents remain unchanged from last image. Later owner must retain snapshots and commit them only after success,while retaining accepted logical transitions on failure. Explicit full repaint on a valid HIDDEN token is recovery,never automatic rollback. Repair/exhausted binding rotation remains existing cursor protocol. Return full effect on success; pre-effect refusals preserve all caller bytes/pixels/output. On post-hide failure publish reached stage/token/dirty/cause,never restore stale pixels or fake rollback;caller stops and explicitly repairs. Stage PAINTING means pixels may be partial;PAINTED means complete base/window writes;ADVANCED means new token committed. A failed hide is pre-effect and leaves output untouched. Success always VISIBLE,cause0.

Before writes:validate fixed arg spans NULL/wrap/nonoverlap cursor vs out;query cursor canonical state without dereferencing retained buffers;check fault0,phase and exact token;validate surface using cs_surface_init and dimensions1..8192;frame geometry/format must match cursor binding fields. Check all pointer spans before respective deref. All writable spans(cursor,out,surface storage_size,payload_capacity) must be disjoint each other and every readonly descriptor/record/span(surface,art,art.bits64B,product,optional product.pixels1024B,old_window ifnonNULL,window). Reject wrap/alias before effect;readonly inputs may share each other if truthful. Native objects/buffers live for whole call,span checks not security. Product phases<=6,success<=1;value,inputs/progress uint32 allowed. Windows canonical per SPEC0053:valid marker,2<=w<=512,2<=h<=342,1<=title<h,0<=x<=512-width,0<=y<=342-height,bool down/drag/pattern;idlegrabs0,drag requiresdown and valid title grab. On update require old/new width,height,title equal(fixedsize). Art validate dimensions1..16/hotspot,nonnull bits64B,MSB2bytes/row,all unused bits zero,mask0/image1 forbidden. x0..511,y0..341. Revision UINT32_MAX with any repaint refuses ERR_EXHAUSTED before hiding/writing. Truthful bound identities/lifetimes not reconstructible from arbitrary native memory;don't silently edit cursor internals.

Canvas512x342,origin floor((W-512)/2),floor((H-342)/2) iflarger else0;smaller frames clip. No scaling. Full repaint:dirty physical rect entire surface,including white margins. Incremental repaint uses conservative full-physical-width row band covering old/new logical window Y extents if position or pattern changes,plus counter logical[286,306) if old_inputs!=product.inputs. Clip translated band to frame;empty clip is canonical{0,0,0,0}. Geometry/drag/down/grab-only changes with identical rendering projection need no scene repaint. No changes at all uses cs_cursor_move only (performs restore/save/redraw);same token,empty dirty. Logical dirty outside small frame may still advance revision conservatively.
Repaint ordering:hide existing VISIBLE cursor;full initial starts HIDDEN. Record HIDDEN,then PAINTING;reconstruct base using cs_product_scene for the truthful band (width=frame.width,height=frame.height,band_top=physical top). Band descriptor full frame width,stride unchanged,storage at top*stride,storage_size remaining truthful bytes,height band rows. Draw new window clipped to that band and logical canvas. Record PAINTED;cs_cursor_advance ->new token/stageADVANCED;cs_cursor_show at final x/y ->VISIBLE. Always restore old overlay even if outside dirty rows. No cs_cursor_discard/repaired needed for normal hide-before-paint;explicit owner repair on failure. No unbounded dirty list or second framebuffer. Padding/tail/unused Mono bits survive. Margins white after full init,remain white.

Original window pixels:filled rectangle [wx,wy,wx+w,wy+h). Outer1pixel border black. Interior title rows with localY<title are white,except localY==title-1 black separator. Content interior(localX1..w-2,localY>=title and <h-1):pattern0 black iff ((unsigned localX>>3) XOR (unsigned(localY-title)>>3))&1;pattern1 black iff (unsigned localX>>2)&1. Use efficient horizontal runs/rect fills,not full-frame per-pixel redraw. No title text,close/resize/highlight,guest Window Manager or historical art. Black means Mono1 bit1 or RGBA000000FF,white bit0/FFFFFFFF. Base original branding/counter may be occluded by window and must restore correctly when uncovered.

API summaries:cs_surface_init checks descriptor only,no pixel writes. cs_surface_fill clips halfopen rect,preserves padding/tail;native storage only. cs_product_scene full/band regenerates original branding+state,counter included;supports Mono/RGBA,logical canvas/framecentering as above,clears all pixels in requested band towhite before drawing base. No scratch allocations. cs_cursor_query validates state and returns info without retained-buffer deref. Bind checks payload/canonical surface/lifetime;tokenbinding/revision;VISIBLE hide validates then restores old save,clears savedrect,HIDDEN;advance HIDDEN increments revision,returnsnewtoken (refusesmax);show HIDDEN validatesart/payload/surface then saves/draws,VISIBLE;move VISIBLE validates before restore/save/draw,tokenunchanged. All cursor API refusals atomic before their own effects. Existing owner preconditions include cursor borrowing storage/art never mutated out of protocol. No new pointer/window source needed for draw closure;window state validation local and no mutation of it.


Clarifications:window bounds are exactly SPEC-0053's canvas bounds,not the
worker's proposed negative/32768 range. cs_product_scene returns1 on success.
Preflight must validate product's complete fixed span before reading pixels.
No memcpy/library import or extra framebuffer is required by this implementation.
The32-byte effect and actual compiler frames are separate costs;no fixed whole-
system stack,64KiB ROM or128K System budget is certified by the helper closure.

TEST-0065:the contract-first full-frame reference uses the unchanged product-scene
API for the previously verified backdrop,then directly sets expected window and
cursor pixels. It is independent of the new compositor,not of the old backdrop.
8 layouts/2,008 complete frames cover Mono/RGBA,small-frame clipping,centered PC
margins,padding,overlap,old/new dirty regions,counter occlusion and cursor restore.
Directed tests inject hide/paint/advance/show failures,check pre-effect refusals,
exhaustion and explicit repaint recovery. These are synthetic API failures,not
native hardware-exception recovery or a claim that every malformed case ran.
[Executed profiles,negative controls and exact inputs](../development/window-composition-evidence.json).
