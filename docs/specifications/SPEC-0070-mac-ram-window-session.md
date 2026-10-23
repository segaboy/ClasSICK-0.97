# SPEC-0070 - Mac RAM window-session bootstrap

2026-10-23; SRC-0402/0403 / ADR-0081 / IMPL-0072 / TEST-0085.

This composes the existing surface, cursor, window-input owner, capture helper, inbox and foreground coordinator APIs into one owned session over a borrowed canvas. It contains no hardware binding: no SCC/VIA/MMIO, no interrupt or CPU primitive, no ROM entry, no boot script, no driver and no physical mapping assumption. It does not clear the held hardware-review lane.

## API

`int cs_mac_window_begin(cs_mac_window_session*, uint8_t *canvas, size_t capacity, const cs_foreground_hooks*)` returns `CS_MAC_WINDOW_OK`, `CS_MAC_WINDOW_ARGUMENT` or `CS_MAC_WINDOW_FAULT`.

The session owns its surface, cursor state, cursor bits and art, a 48-byte cursor payload, window input, capture, inbox and foreground descriptors, plus diagnostic `magic`, `phase`, `stage` and `cause` fields. There is no global state, no allocation, no libc call and no extra full-screen buffer.

## Caller obligations

- The session is a fresh, quiescent, live, aligned object. Begin never reads old session bytes.
- The canvas is ordinary stable RAM. The hook object holds a stable, valid, infallible enter/leave pair.
- The session, canvas and hook spans do not overlap, are owned by the caller and stay stable.
- No producer runs during begin. The producer stays off until a later fresh external baseline. Begin does not fake a release or baseline and does not enable a producer.

## Preflight

These checks run before any write or hook call:

- null session, canvas, hooks or either callback;
- capacity below 21888;
- span overflow;
- session or hook alignment;
- pairwise overlap of the three spans. The canvas span is the declared capacity.

A refusal returns ARGUMENT and changes nothing in the session, canvas or hooks. After preflight, the hooks are copied by value before any other effect.

## Sequence

1. Set `magic` to the original tag, phase BUILDING, stage START, cause 0.
2. Surface: `cs_surface_init` with 512x342 Mono1MSB, stride 64 and exactly 21888 bytes. Canvas storage past 21888 bytes is kept and never touched.
3. Cursor art: the original PC 7x7 plus with hotspot (3,3). For i in 0..31, mask[i] = image[i] = (i even and i < 14) ? (i == 6 ? 0xFE : 0x10) : 0.
4. `cs_cursor_init`, then `cs_cursor_bind` into the owned 48-byte payload.
5. `cs_window_input_begin` with window 80,70,224,144, title 18, product {97,0,16,6,1,NULL} and initial buttons 0. OK or CHANGED is accepted.
6. `cs_foreground_begin` with the copied hooks. This performs the one owner cancellation; begin adds no other cancellation.

Begin never calls the hooks directly. The only excluded section is the existing inbox mutation inside the foreground API.

## Outcomes

**Success:** magic is the tag, phase READY, stage READY, cause 0. Cursor and owner are ready; foreground is ACTIVE at epoch 1; inbox and capture are WAIT at epoch 1, as the called contracts promise. The session phase records only the bootstrap result; it is not a copy of the foreground phase. Afterwards the caller uses the existing foreground and inbox lifecycle APIs directly.

**Failure after preflight:** returns FAULT with phase FAILED. It keeps the reached stage, the exact raw result of the failing call as cause, and the graph and pixels as far as they got. There is no rollback, retry, restart or recovery. To try again, the caller must first make all borrowed objects safe and then start a fresh quiescent lifetime.

## Portability

The module is portable C11 and is intended to compile warning-free under the existing -Wconversion and -Wsign-conversion flags. Overlap and overflow are checked as numbers through `uintptr_t`. That conversion is implementation-defined, and these checks do not protect against hostile objects.
## Lead admission and limits

Lead moves callback-field reads after numeric alignment/span/overlap checks;
otherwise a misaligned hook pointer can fault before the promised refusal.
Only after all three spans pass is the hook value copied and checked, before
any mutation. The hook cookie is an external caller-owned live/stable object
and is not a fourth admitted span. Hooks obey SPEC-0066 exclusion/order and
non-reentry obligations; a bootstrap outcome never enables a producer.

Session cause is the exact return of the failing child call; that child's own
descriptor retains any deeper cause/stage. Fresh means no old borrowed graph
remains live, not permission to overwrite an active session. Local config and
product are copied by the existing input API; art/surface/payload/owner/capture/
inbox/foreground are retained in the caller's stable session.

No native ROM input changes, physical framebuffer claim, hardware admission,
ISR/MMIO/CPU binding or target trial. Tests and compiler measurements belong to
[the evidence](../development/mac-ram-session-evidence.md). The session owns no
extra canvas: a future admitted platform binding may supply framebuffer RAM.
This is software preparation for the paired release; M1-M7 remain open.
