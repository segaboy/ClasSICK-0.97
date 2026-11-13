# BootHush headless paired-release investigation — 2026-11-13

The owner requested BootHush only, headlessly, while moving toward the paired
Mac and PC release. This is an incremental result, not a release qualification.
The desktop was not used. No other emulator, reference system or emulator
implementation source was used. The [2026-11-12 receipt](boothush-headless-20261112.md)
remains historical; the new observations below supersede its statements about
single-range inspection and readiness scheduling for the tested build.

## Identities and material

- BootHush checkout `d48cd8e`; tested Windows executable SHA-256
  `5c5bef37bfdde1073bd2fe94b1e678ad3ec1dc78e531bbf72d0053e8cb099cd6`.
  The checkout identity alone does not establish how the executable was built.
- Original 65,536-byte Mac ROM SHA-256
  `faf37c6f96f8eb2ee6cf7a1e58a8b1b9538055d2fc89493c7419eeea816f6c8c`,
  previously built from `0f8d85df5b6fe21508c22594e0dc20923d4eebe6`.
- Original x64 disk SHA-256
  `80c279a69264cb07f2042bf14503c0687fa32e71057ee1196fc8174afbeeae4f`;
  EFI payload SHA-256
  `df284097539ed6b7c90700cb27589900b20d2e4c7574b7637ba5f387abba9fe9`.
- Detailed generated artifacts and logs remain outside Git at
  `C:/ClasSICK/boothush-release-20261113-a`.

## Mac: readiness, two drags and replay

[Test-BootHushMacWindow.py](../../scripts/Test-BootHushMacWindow.py) has an
optional `--after-ready` mode. It first observes the guest readiness flag at
cycle 56,842,028, then independently cold-boots each of the four existing
checkpoints with events scheduled relative to that flag. All scheduled events
were accepted. Both original absolute-cycle mode and the new mode passed all
four checkpoints. In the readiness-relative final checkpoint the guest pointer
was `(150,110)`, the dragged window `(112,46)`, with three presses, pattern 1,
button released, and no hard losses, omissions or guest fault. The final trace
replayed with the same cycle, both inspected memory ranges (1,024 and 512
bytes), and identical complete 512x342 framebuffer. The final frame matches the
existing guest evidence in the earlier receipt; no duplicate image was added.

These are BootHush-injected window messages. They do not establish ordinary host
input, human input, physical mouse behavior or a release. The four-direction
timing probe was also rerun: at a guarded 4,096-cycle edge interval all four
directions passed; at 2,048 cycles each moved one pixel opposite its intended
direction, with 29 omitted passes. At 8,192 cycles without the vertical-blank
guard it fell one pixel short with two omissions. This is a reproducible
guest/emulator-model observation, not a diagnosis of which implementation is at
fault or a physical timing claim. It remains a release blocker.

## 286: complete payload and both banks at one stop

[Test-BootHushPC286SpanAccess.py](../../scripts/Test-BootHushPC286SpanAccess.py)
now offers `--multi-inspect`. It checks the entire loaded diagnostic payload and
both six-byte banks in one stopped guest, instead of comparing three separate
cold boots and only the first 4,096 payload bytes. The normal case and three
negative-control cases all reached their expected POST values (`A6`, `E6`,
`E8`, `EC`); every inspected byte matched and every original floppy was
unchanged. The normal payload was 4,683 bytes and stopped at cycle 48,849.
The previous single-range mode also passed all four cases after the change.
This is an original 16-bit diagnostic, not a 286 OS boot, a production memory
provider or a real-hardware result.

## x64 PC: media preflight only

BootHush's `--verify-x64-media` accepted the exact disk and EFI payload above,
reported `media_verified` and `executed:false`, and matched the expected EFI
size of 207,360 bytes. Supplying the Mac ROM as an intentionally wrong expected
payload produced exit 2 and `disk EFI payload differs from expected file`.
The original media and payload hashes were unchanged. This confirms a package
relationship, not a boot, rendering, pointer movement or dragging test.

## BootHush gaps and requests

The current documented BootHush x64 support is a nonexecuting media verifier;
there is no implemented x86-64 guest CPU or UEFI execution path. Consequently
the PC half of the paired release cannot be tested in this emulator today.
For the current candidate, the emulator needs an x86-64 execution backend,
eligible UEFI firmware and boot services, raw-disk boot, GOP framebuffer,
timer and PS/2 keyboard/mouse support, and headless guest input, bounded
diagnostics, framebuffer capture and deterministic recording/replay. These are
requirements to scope and verify, not a claim that each individual feature is
independently absent from all builds. A more detailed developer handoff is at
`C:/ClasSICK/boothush-x64-enablement-2026-11-12.md`. The documented WHPX
capability query returned `0x80070005`; that does not establish that Windows
acceleration is universally unavailable.

The prior requests for readiness-relative input, repeated inspection ranges and
a bounded 286 witness are now documented capabilities. Readiness scheduling
and repeated ranges were exercised here; the 286 witness itself was not tested
in this slice. For the unresolved Mac fast-input issue, a useful next addition
would be an easily correlated, bounded event trace of generated quadrature
edges, interrupt assertion/acknowledgement, guest-visible VIA/SCC reads and
input-queue omissions, with cycle timestamps and dropped-event counts. This
would aid attribution without presuming an emulator bug. Writable Macintosh
floppy support and EGA text rendering remain later requests from the prior
receipt; neither is required for the tests run here.

No release was published. Mac timing, host/human input and other standing gates
remain open, and x64 execution cannot be assessed with this BootHush build.
