# ClasSICK product screenshots

Machine-readable discovery: [manifest.json](manifest.json). All PNGs below are unchanged captures of the project's own product framebuffer. Captions, source pins, dimensions, hashes, license and observation limits are in the manifest. No Apple or reference-system content, emulator chrome, desktop or credentials are included.

| Capture | Target | Evidence |
| --- | --- | --- |
| [Native68000 selected-port boot: two synthetic mouse presses, counter0002.](2026-10-18/paired-selection-mac-two-presses.png) | native-mac68000 | [Boot trial](../../development/paired-selection-boot-trial.md) |
| [Native x86-64 selected-port boot before input, counter0000.](2026-10-18/paired-selection-pc-initial.png) | native-x86_64 | [Boot trial](../../development/paired-selection-boot-trial.md) |
| [Native x86-64 first synthetic mouse press, counter0001.](2026-10-18/paired-selection-pc-first-press.png) | native-x86_64 | [Boot trial](../../development/paired-selection-boot-trial.md) |
| [Native x86-64 repeated held-button state leaves counter0001.](2026-10-18/paired-selection-pc-held-repeat.png) | native-x86_64 | [Boot trial](../../development/paired-selection-boot-trial.md) |
| [Native x86-64 second synthetic mouse press, counter0002.](2026-10-18/paired-selection-pc-two-presses.png) | native-x86_64 | [Boot trial](../../development/paired-selection-boot-trial.md) |
| [Native x86-64 two synthetic mouse presses plus Space, counter0003.](2026-10-18/paired-selection-pc-after-space.png) | native-x86_64 | [Boot trial](../../development/paired-selection-boot-trial.md) |
| [Native x86-64 normal completion at guest second60, combined counter0003.](2026-10-18/paired-selection-pc-complete.png) | native-x86_64 | [Boot trial](../../development/paired-selection-boot-trial.md) |

Representative matched-counter captures:

![Native68000 selected-port boot after two synthetic presses](2026-10-18/paired-selection-mac-two-presses.png)

![Native x86-64 selected-port boot after two synthetic presses](2026-10-18/paired-selection-pc-two-presses.png)

See [storage and publication policy](../../development/screenshot-evidence.md). The original private captures and verbose logs remain preserved; this index adds reviewed publication copies.

- [Mac native outline scene before input,counter0000.](2026-10-19/paired-preview-mac-initial.png) — [trial](../../development/paired-preview-boot-trial.md).

- [Mac human-confirmed two-click result,counter0002,drawing intact.](2026-10-19/paired-preview-mac-two-presses.png) — [trial](../../development/paired-preview-boot-trial.md).

- [PC native outline scene before input,counter0000.](2026-10-19/paired-preview-pc-initial.png) — [trial](../../development/paired-preview-boot-trial.md).

- [PC first synthetic mouse press,counter0001.](2026-10-19/paired-preview-pc-first-press.png) — [trial](../../development/paired-preview-boot-trial.md).

- [PC repeated held state leaves counter0001.](2026-10-19/paired-preview-pc-held-repeat.png) — [trial](../../development/paired-preview-boot-trial.md).

- [PC second synthetic press,counter0002.](2026-10-19/paired-preview-pc-two-presses.png) — [trial](../../development/paired-preview-boot-trial.md).

- [PC synthetic two presses plus Space,counter0003.](2026-10-19/paired-preview-pc-after-space.png) — [trial](../../development/paired-preview-boot-trial.md).

- [PC diagnostic completion,counter0006 after mixed synthetic and unreliable owner input; human qualification failed.](2026-10-19/paired-preview-pc-complete-human-attempt.png) — [trial](../../development/paired-preview-boot-trial.md).

- [PC automated follow-up:21 accepted mouse presses plus Space,counter0022; normal diagnostic completion.](2026-10-19/paired-preview-pc-automated-complete.png) — [trial](../../development/paired-preview-boot-trial.md).

- [Continuous native PC preview before input,count0000.](2026-10-19/continuous-preview-pc-initial.png) — [evidence](../../development/preview-session-evidence.md).

- [Continuous preview held repeat counts one press,count0001.](2026-10-19/continuous-preview-first-held.png) — [evidence](../../development/preview-session-evidence.md).

- [Continuous preview second press,count0002;full scene matches Mac.](2026-10-19/continuous-preview-two-presses.png) — [evidence](../../development/preview-session-evidence.md).

- [Continuous preview automated phase at guest second4,count0012.](2026-10-19/continuous-preview-phase-0.png) — [evidence](../../development/preview-session-evidence.md).

- [Continuous preview remains active at guest second68,count0022.](2026-10-19/continuous-preview-phase-65.png) — [evidence](../../development/preview-session-evidence.md).

- [Continuous preview remains active at guest second98,count0032.](2026-10-19/continuous-preview-phase-95.png) — [evidence](../../development/preview-session-evidence.md).

- [Continuous preview remains active at guest second128,count0042.](2026-10-19/continuous-preview-phase-125.png) — [evidence](../../development/preview-session-evidence.md).

- [Continuous preview accepts final Space at guest second185,count0043.](2026-10-19/continuous-preview-pc-final.png) — [evidence](../../development/preview-session-evidence.md).


- [PC window boot,dragging and later input:five reviewed captures.](2026-10-22/README.md) — [TEST-0074](../../development/pc-window-boot-trial.md).

- [Mac timer-path held click,count0001;no motion/drag qualification.](2026-10-26/mac-timer-click.png) — [evidence](../../development/timer-and-pc-input.md).

- [PC raw device control,+1X;no GUI dragging pass.](2026-10-26/pc-raw-device-control.png) — [evidence](../../development/timer-and-pc-input.md).

[2026-10-27 original refresh/PC negative input observations](2026-10-27/README.md):74PNG/twoGIF total;no successful-drag claim.

[2026-10-28 PC physical input:before](2026-10-28/pc-physical-before.png) and [clipped pointer](2026-10-28/pc-physical-clipped-pointer.png). Failed usability;[canonical diagnostic evidence](../../development/release-resumption-evidence.json).

2026-10-29:[Live PC host/device input comparison](2026-10-29/README.md);two own framebuffer PNGs,diagnostic only.

- [2026-10-30 native phase-witness observation](2026-10-30/README.md):new boot capture reuses byte-identical registered pixels;new trace provenance is recorded separately.
