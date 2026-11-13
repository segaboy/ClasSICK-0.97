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

## Correlated witness and edge-rate sweep — later on 2026-11-13

The owner supplied the correlated witness guide, SRC-0603. Its SHA-256 is
`05308bb4d5008d388b494b084734414f5c4825c51f42a38e2ca85ac2189ce81b`,
matching the local BootHush documentation; a byte-identical intake copy is at
`C:/ClasSICK/boothush-mouse-witness-20261113-b/mac-mouse-witness-intake.md`.
BootHush checkout `b6c2367` documented the new interface; the independently
identified executable was SHA-256
`c71e27c6ff3b7e11cd00dbee7d64240be26fb0e0ec4c1f3bfe0ec1aca9130522`.
Source revision alone does not prove executable build provenance.

The exact original ROM was run headlessly with and without witness capture for
each of 2,048 and 4,096 edge cycles, with guard 4,096 and `(+32,0)` input.
Within each pair, the complete result JSON and guest framebuffer were equal.
At 2,048 the pointer ended `(255,171)`, with 29 guest-reported omissions;
at 4,096 it ended `(288,171)`, with none. Each delivered 64 modeled transitions
and had no pending transitions. The detailed witness range reached its end and
reported zero dropped rows in both runs (594 and 649 rows, respectively).

One concrete sequence at 2,048 is: X phase 2 and SCC pending at cycle
60,006,236; X phase 0 at 60,008,286 while SCC remained pending; level 2 accepted
at 60,008,554; the first subsequent relevant VIA phase read at 60,009,664.
The precise-timing CPU mode is disabled; witness instruction context and shared
cycle stamps are not measurements of physical bus timing. This demonstrates a
modeled phase change while the interrupt was pending, not the cause of any
specific guest omission. A prior own-project counterexample rejects inferring
unobserved edge direction from ambiguous latch samples; see
[the split-recovery result](mac-fast-timing-20261103.md#rejected-split-recovery-proposal).
No guest mouse behavior or hardware-spacing policy was changed.

For BootHush developers: the detailed witness reports the device and interrupt
events but does not report writes to selected guest RAM, so it cannot directly
associate an individual guest-owned omission or timeline update with those
events. An optional, bounded, address-filtered RAM-write witness in the same
ordered stream would close that diagnostic gap. Include write width, address,
written value, instruction context and cycle, the existing drop/coverage flags,
and an explicit statement of timing granularity. It must not modify the guest;
compare traced and untraced stop state and frame. A timing-validated model would
be separately useful; do not silently present the current model as physical.

The existing timing probe now accepts an optional bounded `--sweep-edges` list.
Its four-direction sweep at guard 4,096 produced the following **observed
passes out of four**, under this ROM and emulator configuration:

| Edge cycles | 2048 | 2304 | 2560 | 2816 | 3072 | 3328 | 3584 | 3840 | 4096 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Passes | 0 | 0 | 0 | 0 | 2 | 4 | 4 | 4 | 4 |

At 2,304 and 2,560, some final displacements were opposite to the requested
direction. The sweep narrows a model-dependent boundary; it does not establish
a safe physical rate, behavior between sampled intervals or a fix. The script
correctly returned exit 1 for failing cases; a duplicate-interval input was
rejected with exit 2. Detailed witness, uninstrumented comparisons and sweep
outputs remain outside Git at `C:/ClasSICK/boothush-mouse-witness-20261113-b`.

The new `--x64-capability` check returned exit 1, `0x80070005`,
`implemented:false` and `guest_executed:false` in this execution environment.
This does not establish host-wide WHPX availability. Current BootHush docs
still identify x64 CPU and UEFI execution as unimplemented; the PC release
cannot be boot-tested using BootHush yet. No other emulator was used.
