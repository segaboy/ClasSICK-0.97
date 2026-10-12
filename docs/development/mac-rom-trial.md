# Original Macintosh 128K ROM trial - 2026-10-11

SPEC-0018 / ADR-0021 / IMPL-0022 / TEST-0026 / SRC-0084.
**First own-scene startup observed; corrected-image qualification pending.**
Local date October 11; timestamps below are UTC on October 12.

Snow v1.5.0-317c188 Windows x86-64 is the exact executable cleared by SRC-0082:
SHA-256 `32cf5285582c0f509e4f1ac38d86bfb90440bae9da43b7823bfb0841dea588e6`.
Only our own image is loaded. Macintosh 128K / 128 KB, Relative hardware mouse,
normal speed, custom-ROM validation override; persistent PRAM, extension ROM,
800K drive override and fast-forward off. No disks, shared directory or serial
bridges. The 128K model has no Ethernet device; a saved default NAT setting is
configuration metadata, not an attached network adapter.

The first short launcher lifetime ended before an operational window; the next
two empty-window attempts showed no response to primary-button automation or
an owner click. The alternate button opened the loader. No system mouse
settings changed; this was an input-mapping issue, not a ROM execution failure.
The successful run uses a new isolated application profile and explicit desktop
execution context. No owner emulator configuration or ROM library was read.

Initial Os image from `7c673f79ba4353a75cc08d38904bc2750cc385dd`, SHA-256
`563b98400615342f841fb11f0a4820127c605b6536ca2ad713c5511fce4d75f4`:
Load/run at 01:44:45.169; original scene visible by 01:44:59.547. At
01:46:33.198 the displayed count was 600, with zero presses. Later it reached
1,860/zero before the run was deliberately ended for a performance correction.
The normal scene is gated on our image CRC and TRAP return checks, but full
redraws were slow. No completion, operator input or calibrated timing acceptance
is inferred from this provisional run. Local screenshots/configuration retained;
no reference pixels or binaries committed.

The independently authored multiplier now stops when its remaining bits are zero
rather than always processing 32 positions. Fresh arithmetic/hosted checks and
68000 image twins/audits pass. Corrected Os image:
SHA-256 `920fbcfcd22c8dbf7a7214a5a1b93e8c43689f87cfc610dea218a6097023618e`,
3,160 used ROM bytes, seal `7a4c3e07`, 28 data/BSS bytes, 8 KiB reserved stack.
Corrected-image cold starts, elapsed timing and owner input remain pending.

Publication CI: run 38192057023 at initial source passed the Linux image audits
and Windows suites, but the Windows wrapper propagated an expected fault test's
nonzero exit status. Reporting fix at `0c33d7e2089ec13aa3287347a8f6038eb41edc26`
passes both jobs in run 38192805732. CI never runs Snow or an operator session.

This is an original startup/display/input demonstrator, not System 0.97, Toolbox,
desktop, disk or application compatibility. Full OS editions, real Macintosh,
Mini vMac qualification, historical parity and complete 128K budgets remain open.
No Apple ROM/code or emulator implementation used; human provenance review pending.
