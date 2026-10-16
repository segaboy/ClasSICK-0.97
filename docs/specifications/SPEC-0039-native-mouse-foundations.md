# SPEC-0039 - Shared native mouse button and pure PS/2 report decoder

Accepted2026-10-16; SRC-0227..0232 / ADR-0045 / IMPL-0043 / TEST-0047.
Original engineering contract; no historical Macintosh behavior is asserted.

## Scope
- `core/input/mouse.h/.c`: shared button-edge semantics.
- `platform/pc/ps2-mouse.h/.c`: pure three-byte report decoder.
- Windows viewer: left-button clicks count through the shared API.
- Native Mac drawing entry (`native-drawing-entry.c` only) counts presses through the shared API.

Out of scope: 8042 port I/O, controller startup, AUX routing (PC keyboard driver unchanged and still discards AUX), cursor motion UI, Event Manager, global QuickDraw, guest/native ABI changes, emulator/hardware runs, release. Freestanding PC mouse remains incomplete.

## Button API
- `cs_mouse_button_begin(b, down)`: ARGUMENT for null or down>1 with no mutation; otherwise down=baseline, presses=0, OK.
- `cs_mouse_button_sample(b, down)`: ARGUMENT for null/down>1; STATE when stored down>1; both before effects.
- A press is a 0->1 edge only; presses saturate at UINT32_MAX. CHANGED for either edge, OK for unchanged.
- Held at begin is not a press. Cancellation = sample(0), never counted.
- No allocation, libc, globals or guest pointers. Pointers must be live, truthful, disjoint.

## PS/2 decoder
- `begin`: ARGUMENT for null; otherwise zero all fields, MORE.
- `feed` refusals before effects: ARGUMENT (null, byte>255, corrupt>1); STATE (used>2, header>255, x>255, used>0 with header bit 3 clear).
- corrupt=1: reset all fields, DROPPED, whatever the byte is.
- used 0: bit 3 clear -> DROPPED; else save header, x=0, used=1, MORE.
- used 1: save x, used=2, MORE.
- used 2: reset; header bit 6 or 7 -> DROPPED; else dx=x-256 if header bit 4, dy=y-256 if bit 5, buttons=header&7, PACKET.
- Output is written only on PACKET; on every other path it stays byte-identical.
- No axis inversion or clamping. 0xFA/0xFE/0xAA/0x08 payload bytes are data.
- Strict profile (bit 3 required); arbitrary byte-loss resync is not guaranteed.
- Overflow drop and sign arithmetic are explicit original policies. Bit positions come from public Microsoft PS/2 interface table prose; no driver or sample code was used.

## Windows viewer
- `demo_state.mouse` (outside the arena budget). Counter shown = saturating accepted_spaces + mouse.presses, used for scenes and keyboard updates.
- WM_LBUTTONDOWN: sample 1, then SetCapture if not already owned.
- WM_LBUTTONUP: sample 0, then ReleaseCapture only if GetCapture()==window.
- WM_CAPTURECHANGED (new owner not this window): sample 0; never releases another window.
- WM_CANCELMODE / WM_KILLFOCUS: sample 0, release owned capture.
- State is updated before capture calls, so a synchronous WM_CAPTURECHANGED re-entry sees down=0 and is a no-op.
- Surfaces update and the window invalidates only when the shown count changes. No format switch, no flash.
- `--verify-mouse`: hidden window, real SendMessage/ReleaseCapture dispatch; checks repeat down, up, outside up, real capture loss, stale up, cancel, focus loss, Space sum, saturation. Surfaces must equal the prior bytes plus exactly the counter overlay. No platform or human input is claimed.

## Native Mac
After `cs_mac_loop_begin`, begin a `cs_mouse_button` with `loop.mouse_down` and sample after each poll; diagnostics and the counter use `mouse.presses`. Faults: 9 = begin refused, 10 = sample refused. The legacy loop counter stays as the compatibility implementation.

## Verification
`tests/mouse.c` covers primary behavior. Independent adversarial decoder/button
cases, authored Mac register transcripts and actual hidden Windows dispatch are
recorded in [TEST-0047](../testing/TEST-0047-native-mouse-foundations.md).
Compiler/hosted results and open target gaps are in the
[evidence receipt](../development/mouse-input-evidence.md).
