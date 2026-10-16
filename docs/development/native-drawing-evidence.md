# Paired native drawing workload - 2026-10-16

SPEC-0038 / ADR-0044 / IMPL-0042 / TEST-0046; SRC-0219..0222.

The original native C drawing workload now calls the rectangle service directly on native68000 and x86 paths. PC boot/Windows viewer and a distinct native Mac ROM use real native results/pixels; the separate40-byte68k compatibility fixture remains. Independent pixel/failure/legacy tests, nine-profile2280 checks, dependency audits and native candidates pass. No new boot observation or release; PC mouse/event parity remains open.

## Native and compatibility paths

`apps/native-drawing/workload.c` calls the existing native `cs_qd_rect` directly,
with the selected paint/erase rectangles. It resets its caller-owned port/report,
computes actual pixels/hash/counts and reports97/success only after successful
calls and metric validation. Native phases describe initialization, painting,
erasing, scanning, validation and completion; no CODE loading/retirement is claimed.
The native workload object's only project import is `cs_qd_rect`. The standalone
hosted workload/service test links without the interpreter or resource loader.

PC boot and the Windows viewer now use this native result/pixel buffer. The older
`cs_scene_render` helper also executes the real workload and refuses without
surface writes on failure. A distinct Mac `--native-drawing` ROM entry uses the
same native C function, native reset/TRAP/service preflight and deterministic
unexpected Line-A fault; it has no interpreter/legacy application dependency.
Its native closure links only workload, rectangle service and original arithmetic
helpers, with no undefined imports or interpreter symbols. The linked native ROM
also rejects interpreter/application-run symbols. The existing `--drawing` ROM
and original40-byte interpreted fixture remain independent compatibility paths.
No external native executable format, loader or SDK is introduced.

The shared scene/branding remains512x342. PC input is still the keyboard counter,
Mac input still its mouse backend; native drawing does not close pointer/event
parity. Windows arena budget is1428520 bytes (existing display/input storage plus
1032-byte port); its LLDB assertion matches the new reservation.

## Verification

- All2280 CTests pass in nine Clang/GCC debug/twin/optimized32/64/sanitizer profiles.
  Seven added suites cover the independently calculated complete pixel image,
  poisoning/repeat/null/canaries, separate legacy equality, native-only link,
  failure-control helper link and freestanding imports. Failure controls include
  retained effects/phases and no fabricated scene or surface writes.
- Sixty final Clang/GCC width/optimization and ARM little/big-endian import audits
  pass. Four additional audits check the exact new native EFI object twins.
- Native-only workload/service/helper68000 relocatable twins have zero undefined
  imports or interpreter symbols. The two complete ROMs are identical. Used ROM
  7560 bytes; data/BSS2160, port workspace
  1032, conservative summed frames/margin
  10316 within24320 bytes; largest frame604.
  These are partial profile bounds, not complete128K OS feasibility.
- Native x86 EFI O0/O2 twins pass, along with27 corruption refusals and14 existing
  omitted-object controls. A supplementary missing-native-workload link refusal
  adds the fifteenth, using the already compiled objects without another build.
  CI's updated script runs all fifteen together and audits the native object.
- Six valid x64 LLDB cases and three invalid-argument refusals pass. Compiler
  twins match for native conformance/native-only executables and the viewer.
- Both [CI jobs](https://github.com/segaboy/ClasSICK-0.97/actions/runs/38972919659) pass all2812 CTests across11 profiles
  at compiled revision `112780684253f3169df591fced5367de1ef21cd7`. The downloaded
  native ROM, complete ROM manifest and complete native-closure manifest
  match local bytes exactly, without normalization. Both EFI hashes appear
  in CI;60 object audits and all15 omitted-object controls pass.
  Compiled-only identities/results are in the [JSON record](native-drawing-evidence.json).

The new ROM is65536 bytes, SHA-256 `b02c765567787161b07c673c3edb8b84ca158dba45dfdef2e36d25d91c5a2fb1`.
Optimized PC payload is149504 bytes, SHA-256
`2df45e6116b78fe4e62ad134f83cb3f86671efb9056c27053c956e6260dfeef2`. Neither candidate has been booted. Ordinary build
commands are `Build-LoadedAppROM.py --native-drawing` with the existing pinned
tool root/new output, and `Verify-UEFIImage.ps1` with a fresh BuildRoot. Both
perform construction/checks only. Published ROM0.2.0-alpha.1 remains unchanged.

## Bounded Claude/Grok work and lead dispositions

The owner expressly requested both tools under standing reviewed-public/original
transfer permission. Claude received one9504-byte contract/API packet and authored
an original288-line independent test artifact,48 lines over the requested limit;
the lead reviewed/adopted it without another request. One CLI request/two model
turns/one Read, no edits or test runs. Grok received one25431-byte frozen boundary
packet, one request/three turns; its second Read at offset459 returned the final
blank line. No extra files, commands or network research were read by either.
Prompts: [Claude](tasks/native-drawing-tests.md), [Grok](tasks/native-drawing-boundary-check.md).
Exact packet/output identities and provider accounting are recorded in the JSON;
CLI cost estimates are not quota/invoice measurements.

Grok's synthetic helper finding led to the real workload path and added failure
controls. Its mutable-platform-entry omission had independently failed the lead
compiler build already; explicit entry admission fixes it without relaxing core
global rules. Its Mac legacy-preflight coupling finding led to native-only
preflight; an initial zero/failure frame remains intentional, not successful
execution. Its full native closure suggestion is implemented. Native object
disjointness remains an explicit truthful-caller precondition; arbitrary alias
validation is not added. No extra worker call was needed. Future small reviews
should declare packet line counts/read limits and use low reasoning effort when
appropriate to avoid redundant reads and excessive review reasoning.

The initial sanitizer pass caught instrumentation in an import-audited object;
the audit object now stays uninstrumented while actual native code/tests remain
sanitized. Initial compiler/check logs are retained. A repeated object-check
attempt refused an existing output folder; final checks use a fresh directory.
The private supplementary linker command first refused PowerShell syntax before
execution; the quoted argument then ran and rejected the omitted object correctly.
The first CI run's earlier framebuffer wrapper retained a stale dependency
allowlist and rejected the new native call. Its scene entry now explicitly
allows `cs_native_draw` instead of the old interpreted probe, matching the
actual dependency and the other object audits; no runtime code changes.
The corrected two endian scene audits are checked locally before rerunning CI.
No failing attempt is counted as a pass. No new manual, reference payload,
emulator implementation, UART body or historical claim was used. All24 originals/
copies, prior evidence/edition/target records and released assets/tags are retained.
Human/source/UART/physical/full128K/edition/B2/System1-ROM gates remain. Fresh
owner go-ahead precedes each new target trial; no automation/OneNote/new chats.
