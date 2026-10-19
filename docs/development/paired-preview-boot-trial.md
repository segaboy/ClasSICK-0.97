# Paired preview boot and input observations -2026-10-19

SRC-0295..0297; SPEC-0049 / IMPL-0052 / TEST-0058.
The exact paired-native-preview-1 images booted successfully. The Mac human
two-click check passed. The PC human check failed because the owner reported
intermittent acceptance after many clicks. Synthetic passes do not override that
report. The pair remains **not release-ready**.

The owner authorized one isolated Snow Early128K and one VirtualBox trial with
"Do it", then explicitly cleared automated tests without further human input.
That permitted one additional bounded PC press-duration trial. Totals: one Mac
cold start and two PC cold starts, all now closed; preexisting VM inventory and
running states restored. No rebuild,release,reference payload or source inspection.

Compiled source: `207e557b83cbab6933e7628275879f0f38c11021`;
prior compiler receipt: `a484eaf887e722fb2983e1ed4876eeb46492d876`.
Images were rehashed before and after execution:

| Image | Bytes | SHA-256 |
|---|---:|---|
| Original Mac ROM |65536|`2b600319d334145e582adc3db0e0fcf495d5f44309488c19e652dfb378391780`|
| Native PC EFI |159232|`cc667453316e262de61af774c880e3607b034cded3d742957b220a589f36e042`|
| Original raw disk |67108864|`e9567ac8f91c48dd435ed7fc7c1021511da131ab8975a7ca7a3ad17aeaa8127c`|

Both VDIs passed independent media/EFI identity and raw roundtrip checks. Both
machines used the same isolated profile: EFI64,1CPU,512MiB,vboxvga,PS2 keyboard/
mouse,secure boot off,all8 NICs/USB/audio/clipboard/drag-drop/VRDE disabled,no shares.
Snow's explicit fresh Early128K workspace used only our ROM with no media,
extension ROM,PRAM or bridge; debug panels remained closed. Its attempted child
profile overrides still recalled an older own-project picker location,so persistent
preference isolation is unqualified. The new absolute workspace was loaded through
that picker. UI controls needed the previously observed alternate button mapping;
no automated Snow guest clicks were used. Product framebuffer exports exclude UI.

Mac displayed branding,APP097,ring/two rails,six completed phases,result0097 and
counter0000. The owner clicked twice and confirmed "Yes—0002, drawing stayed
intact". The exported counter0002 image agrees. This is a narrow human button
check in Snow,not physical hardware or general Macintosh input qualification.

The first PC boot accepted synthetic mouse0->1,held repeat1,release/press2 and
Space3. The owner subsequently made many clicks; the counter reached6,but the
owner reported intermittent acceptance. The expected human count is unknown.
VirtualBox reported unavailable/suspended pointer integration; capture/focus and
the finite diagnostic lifetime were not controlled well enough to assign a cause.
The PC human-input qualification is **failed/unreliable**,not a six-event pass.

The separately authorized automated follow-up delivered three presses at each
requested hold duration5,10,25,50,100,250 and700ms,with350ms release/settle gaps.
All21 were accepted exactly once; Space added one,ending at22. Host scheduling
means requested durations are not precise pulse widths; actual host API intervals
are recorded per press. Synthetic device injection bypasses host pointer capture
and does not establish ordinary human-click reliability. No missed click was
reproduced in this bounded matrix; its root cause remains unresolved.

Both PC boots reached guest second60,result0 and keyboard-ready1. The retained
scene remains visible after the diagnostic stops reading input. A post-completion
button press/release and Space left the second boot's framebuffer byte-identical.
Pauses/resumes were used; uninterrupted wall-clock timing is unqualified.

At counts0000 and0002 the native Mac and PC match **all175104 logical pixels**
after dark/light palette normalization,including the now-static empty progress
strip. Six first-boot PC captures and ten follow-up captures preserve173504
pixels outside the counter rectangle; first-boot physical pixels outside centered
512x342 are white. The held repeat leaves the counter unchanged. Reviewed original
captures are indexed in [screenshots](../evidence/screenshots/README.md),with
hashes,rights,pins and explicit human/synthetic limitations.

The previous3123 CI tests/11 profiles are reused,not rerun or added to the new
checks. No worker call was needed for device control and final verification;
completed Claude/Copilot artifacts were reused. All four remaining external
account allowances are unavailable. No provider-token/ChatGPT-quota equivalence
is claimed. [Sanitized receipt](paired-preview-boot-trial.json).

Next release blocker: reproduce and resolve PC human capture/input reliability
and make the active versus completed diagnostic state unambiguous. Define exact
automated acceptance criteria before changing runtime behavior; then verify the
same scene preservation on both native paths. The current slice makes no runtime
change. Package freeze/lifecycle acceptance/publication remain pending; published
versions/assets,tags,24 research originals,UART and prior evidence remain unchanged.
OneNote/wiki is retired. Historical/edition,physical,full128K,B2,native external
application,Linux/complete edition and both System1-ROM boot directions stay open.
