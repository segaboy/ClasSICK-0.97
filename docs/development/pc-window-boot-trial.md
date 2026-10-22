# Native PC window boot and drag qualification

2026-10-22; SRC-0359..0361; SPEC-0062; TEST-0074.
The exact CI candidate booted, accepted four synthetic mouse presses and moved
its window through three positions with exact raster preservation. Input worked
after a continuous 70-second interval. This is E2 device/framebuffer evidence;
GUI capture, human reliability and Mac parity remain unqualified.

Compiled source: `0b23981a83f5c1e0fdb231707d83c3d25c6085c8`. Reviewed inputs: `a23e9f9f786cfb21b1fa2afb9f297bec7d29b185`.
[Existing Windows/68000 CI](https://github.com/segaboy/ClasSICK-0.97/actions/runs/39826971657) reused; no rebuild or runtime change.

|Image|Bytes|SHA-256|
|---|---:|---|
|Native x86-64 EFI|182272|`3bd7d5a286f3fa5b3f0c4765d23a23e2de5c78ed1b924c67aceeaf680f8839a9`|
|Original disk|67108864|`4529eb740639b868dce5cf57224574ebb78bc5f1518146a6389a26563b4bc748`|

Owner's "Do the next step" accepted the explicitly proposed exact-candidate
qualification. One fresh headless VirtualBox 7.2.16 boot: EFI64, 1 CPU, 512 MiB,
vboxvga, PS2 keyboard/mouse, only our disk. SecureBoot, all eight NICs, USB, audio,
UART, clipboard/file transfer, drag-drop and VRDE off; no shares. Afterward the
temporary VM was powered off and unregistered; previous inventory/running states
were restored. EFI/raw hashes and pre/post-trial VDI-to-RAW roundtrips agree.
External poweroff is not an observed guest STOPPED transition.

Actual checks: 16 decoded monochrome 1024x768 captures, unscaled 512x342 centered
at 256,213, white physical margins. Exact original-glyph pixel readback confirms
counter 0->1 for the first body press, held movement still 1, title press 2,
content press 3 and final press 4. A real released report preceded testing to
clear startup suppression. Body presses alternate checkerboard and stripes.
Title dragging moves the window 80,70->100,80->120,90->140,100. Four translated
224x144 crops, including the cursor, match exactly; 127688 pixels outside the
old/new window bounds and counter remain identical. Release frames are unchanged.
These are specific translation/preservation checks, not a full renderer oracle.

After one uninterrupted 70-second host interval, Space/Escape injection succeeds,
the frame remains identical at count 3, and a subsequent mouse press produces
count 4 and changes the pattern. Actual key draining/guest elapsed time are
unobserved. Total active-host upper bound 120.04 seconds, below the 240-second
budget; other captures used pauses. No precise pulse-width or whole-trial
uninterrupted timing claim. Private scripts/captures: C:/ClasSICK/pc-window-trial-20261022-a.

Target CWP1 remains **unobserved**. Installed public COM type information exposes
ReadPhysicalMemory, but its first attempted 432-byte own-header read returns
"not implemented". No memory bytes returned; no scan, core dump, firmware,
descriptor contents or executable instructions read. Original GDT/IDT/stack
register relationships were checked to locate only our bundle. Hosted trace
tests stay reused E1 evidence, not target trace proof.

Control failures retained: initial nine-second screenshot request returned 0x0;
25 additional seconds in the same boot produced the scene. One host helper alias
collision sent zero mouse events and was corrected. A suffix-less UTC timestamp
was initially treated as local; its four-hour overcount was corrected before
further execution. No extra cold start or omitted successful test.

Sanitized exposure record: a search for public debugger documentation exposed
unrelated emulator table entries in one excerpt. No function body or result page
was read, no content reused/transferred, no affected ClasSICK implementation.
Lead stopped that review and notified the owner under POLICY81-88. Owner:
"Do what you think is right. My only goal is to achieve an all original code
reproduction of system 1." Lead excludes the excerpt and continues only public
interface and own-data inspection; no search content admitted or copied.

[Five unchanged reviewed captures](../evidence/screenshots/2026-10-22/README.md)
are retained in Git for research/web discovery; no desktop, emulator chrome,
reference pixels, credentials or protected assets. [Machine receipt](pc-window-boot-trial.json).
[Executed test record](../../provenance/records/TEST-0074-pc-window-boot-trial.md).

No worker call: completed Claude/Copilot artifacts and existing controls reused.
Small qualification work was performed directly to avoid more coordination.
Current external allowances unavailable; earlier Claude five-hour snapshot
expired. No provider-token/ChatGPT-quota equivalence or measured saving claimed.
TEST-0073's 348 profile CTests and CI are reused, not rerun or newly counted.

No target fault/loss/saturation testing or GUI/human acceptance. Mac M1-M7, paired
parity, BIOS, physical/full128K, B2, historical/edition, native external-app,
Linux and both System1-ROM directions remain open. The 24 originals, held brief,
UART bodies, old records, releases/tags and retired wiki are preserved. No release
or further trial permission. Next: review Mac motion-source/adapter prerequisites
for the same shared window, retaining the PC trace/GUI gaps as qualification work.
