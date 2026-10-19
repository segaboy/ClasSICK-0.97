# Paired native selected-port boot trials - 2026-10-18

SRC-0278/0279; SPEC-0047 / IMPL-0049 / TEST-0055.
The owner said "Continue" after the proposed pair of separately approved Mac and
PC boot trials. This authorized one fresh Snow Early128K boot and one fresh
isolated VirtualBox boot of the exact previously verified candidates, synthetic
input checks and screenshots. Both sessions are closed. No rebuild or release.

The compiled source is `ac4e7b31784dde1620938ca9a2855e61f14d7c1c` and the prior
compiler/CI receipt is `71047e966997d3935260f363326a824cec0a7ff7`. Its dated
no-boot meaning is preserved. The 65,536-byte ROM SHA-256 is
`1cd00f53acefff1be8c553924d3b5d27d2e9f964a551b6de2b38330878800167`;
the 159,232-byte native EFI is
`a19e85ff866586fc6eada703e1c9a2ec8d1c2a42e113d911260afb4f92c72e36`;
the 67,108,864-byte raw disk is
`8a2eaf896b7e51363182d7ee53741429dd14d943b87dd3e63298c8b944ee9fac`.
Copied inputs were rehashed before and after the trials. The independent disk
reader and VDI/raw roundtrip passed. [Sanitized receipt](paired-selection-boot-trial.json).

| Target | Actual observation | Input result |
| --- | --- | --- |
| Native68000, Snow Early128K | Branding, APP/097, both port drawings, six phases, result0097 | Two synthetic mouse clicks:0000 ->0001 ->0002 |
| Native x86-64, isolated EFI64 PC | Same drawing;512x342 centered at(256,213) in1024x768 | Mouse0000 ->0001, held repeat0001, release/press0002; Space0003 |

PC reports controller03/03, A20 probes0/0 and normal result0/keyboard-ready1 at
guest second60 with combined count3. Same-session pauses/resumes preclude an
uninterrupted wall-clock timing claim. Its public mouse device API used button
masks0,1,1,0,1,0 and zero relative movement; keyboard used make/break39/B9.
Snow used the observed alternate-button UI mapping. No human/physical input is
credited. Native OS workload C runs directly on both CPUs; the separate legacy
68k application compatibility path is not the PC OS implementation.

Six PC captures each preserve161,856 static logical pixels; all physical pixels
outside the logical scene remain white. At the same count0002, the Mac and PC
match all163,456 logical pixels outside the progress strip after binary palette
normalization. The original images are unchanged: Snow uses dark/light gray,
PC uses black/white. The Mac binding deliberately leaves progress empty, while
the PC carrier retains its timed strip; this trial does not establish timing or
full carrier-feature parity. Raw screenshots and logs remain outside Git.

Snow's command-line workspace path failed before any guest started; loading the
verified workspace through the same process's file dialog succeeded. A left
click did not dismiss the initial dialog; the alternate mapping worked. One
unsupported alternate double-click produced no input. The three attempted child
profile-directory overrides remained empty and the picker recalled a previous
project location, so persistent preference isolation is explicitly unqualified.
The fresh workspace itself was checked: own ROM only, no media/shared directory,
extension ROM, PRAM or serial bridge, and debug panels closed. Early128K has no
Ethernet device. No private implementation or reference payload was inspected.

VirtualBox management was inaccessible inside the sandbox before preparation;
approved host-context execution succeeded. Its fresh machine disabled all eight
network adapters, USB, audio, clipboard/file transfer, drag/drop and VRDE, with
no shared folders and only our disk. First unregister waited for the already
powered-off GUI lock to release. The machine is unregistered, artifacts retained,
and original inventory/running states restored. No extra cold start was used.

Previous3,068 CI tests/11 profiles are reused, not rerun or counted as this trial.
Only disk/configuration/input/capture/closure checks above were newly executed.
No external worker call was needed for target control/final verification; existing
Claude/Copilot artifacts and qualifications were reused. All four external account
remaining allowances are unavailable; no quota estimate or new usage is invented.

Runtime/UART bodies,24 research originals/copies, old evidence rows, releases/tags
and frozen wiki remain unchanged. OneNote/wiki maintenance is retired. Physical,
complete128K budget, historical/edition, B2 criterion6, human input, full manager,
native external-application gate and both System1 boot requirements remain open.

Next proposed slice: define the next paired drawing primitive and its independent
oracle before implementation, using the existing native selection/view foundation.
No further target trial, release or new runtime implementation is authorized here.

## Subsequent screenshot publication -2026-10-18

At the owner's direction,SRC-0280 publishes7 unchanged own-product capture copies in [the screenshot index](../evidence/screenshots/README.md) with [manifest identities/captions/limits](../evidence/screenshots/manifest.json). Private originals/logs remain preserved. This adds discovery/publication,no new run or observation.
