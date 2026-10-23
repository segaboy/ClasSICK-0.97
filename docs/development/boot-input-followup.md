# Native boot and input follow-up

2026-10-23; SRC-0416..0418; SPEC-0072/0073, IMPL-0074/0075, TEST-0087/0088.

The accessible-desktop trials find two distinct issues. The Mac ROM draws the
shared scene, then refuses SCC setup. The PC accepts a GUI click when the test
uses the host's primary button; the previous automated drag did not use it.
Release2 remains unfrozen. These observations supersede the earlier UNBOOTED/
desktop-inaccessible checkpoint for this one default Mac and diagnostic PC image.

## Mac observation and bounded correction

The exact ae4ae47 default ROM (SHA256 d1ef824abab8deaacdc1ab10676acb483551559ec724f652dfc9918f478b9f2f)
draws the original512x342 scene in Snow Early128K. Snow's unchanged PNG uses
its own two-colour display palette. Its thresholded pixel mask matches the
PC's centered initial scene exactly; RGB byte parity is not claimed.

Own registers show PC0040072C/SR2700/D000000006. Black-box runtime warnings
report unimplemented reads/writes at the selected lowest SCC aliases. No
counter response or functional input follows. This is a failed input candidate,
not a successful session. No emulator source, reference ROM or disk was read.
Absolute workspace paths correct a separate launch-harness error; failed
relative-path/process attempts are retained outside Git. The owned Snow closed.

The next original engineering profile selects the upper eight-byte cell of
the same published SCC read/write windows, retaining the inferred A/B bit and
read-even/write-odd rule. Only four platform constants and matching manifest
values change; no address scan, fallback, IRQ or loss-policy change. It remains
an unverified board hypothesis pending native compiler and own-ROM trials.
The old rejected image remains negative evidence; no physical mapping or
historical implementation is inferred. Held source material remains excluded.

## PC GUI finding

One fresh isolated boot uses the verified optional probe EFI, SHA256
2ce9b2c1ec39d3e1d0fa1f58aed6ba04cad8639df714d2304098f11ea8734531.
After capture, the old default-button drag leaves counter0000/window unchanged.
RIP10008F65 is our INB return, outside the own probe spin1002D052..1002D055;
no register mailbox is decoded and the loss hypothesis is not demonstrated.

Read-only host preference is SwapMouseButtons=1. An explicit SKY right-button
command activates Snow's menus and, after VirtualBox capture, the native PC
left-button counter advances to0001. RIP after that check is10007447, again
outside the probe spin. The host setting is unchanged in this trial. This
explains the failed click test. The supported drag API has no button selector;
GUI drag with the primary button still needs qualification. No guest driver
change is justified by the previous automated-button mismatch.

The VM powers off and unregisters; exact raw-media roundtrip and existing VM/
running inventory are preserved. An initial unregister hits a transient GUI
session lock; retry follows powered-off verification. No broad memory read,
core dump, firmware inspection or emulator-source inspection occurred.

## Verification and next gate

The receipt records actual image identities, raw observations and closure.
Two original framebuffer PNGs are indexed for the research updater. No new GIF:
no new successful drag sequence has been captured. E1-E3 on both final native
candidates, final package review and separate publication approval remain.
No E4/hardware/System1/full128K claim.286 progress is unchanged, not restarted.

No new external worker is warranted for these bounded observations and four
constant edits. Prior qualified implementations/reviews are reused; no provider
retry or duplicate review. Remaining Claude/Grok/Copilot/Antigravity balances
are unavailable. Native compilation and subsequent trials are reported separately.
