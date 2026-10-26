# Native image verification and actual input checks

2026-10-26;SRC-0473..0475. Neither paired release candidate is qualified.

## Verified native builds

Public6777f9d full CI40315483335 succeeds. All three64KiB Mac images match
their manifests,43 committed Git-LF inputs each and original ROM seals.
The normal ROM is byte-identical to63bbccf:
673a2cca57d1ec9ffd842f4c858fa4aafbcb042ec6f86445ae5717d4816c4965.

|Mode|ROM used|BSS|Workspace|Conservative stack bound|
|---|---:|---:|---:|---:|
|Normal|23096|304|964|4996|
|Phase-only experiment|23244|304|964|5008|
|Phase plus finite-defer experiment|23668|312|964|5008|

The phase-only cost is148 ROM bytes and12 stack-bound bytes;no extra BSS.
The combined experiment costs572 ROM/+8BSS/+12 stack-bound bytes relative to
normal. Stack numbers are compiler frame-sum bounds,not measured use. The
normal image remains unchanged;no experimental policy is adopted as default.

## Actual operator checks

The owner restored an unlocked desktop. A zero-event inactive helper then
read foreground/cursor metadata successfully. Two following debug-panel
Snow attempts still yielded black captures and ineffective menu/edit actions;
no guest input pass. Simplifying the display profile and having the owner
bring Snow forward yielded a visible scene. Those variables changed together;
the debug panels alone are not established as the cause.

The owner physically tested the normal Mac ROM and reported that the scene
was visible,but no usable cursor appeared when entering it and the title bar
could not be grabbed. A clean earlier observation showed counter0005. Thus
buttons reached the guest,but motion/dragging did not pass. That test closed
after136.41s with the ROM unchanged.

The owner physically tested the verified normal PC disk and reported that
VirtualBox captured the mouse,but there was no usable cursor or interaction.
The observer injected no events. Of63 framebuffer captures,there are two
unique pixel images:the initial image and the first changed frame at114.95
host seconds. Changes fit[512,213,768,388),including restoration of the old
7x7 cross and a clipped cross at the upper-right logical corner. The window
and counter do not move. This demonstrates a guest display change,not useful
motion,dragging or a decoded packet chronology. It rules out injected input
as the sole explanation of the reported usability failure;the root cause
remains undetermined. The clipped pointer must not be described as absent
from every framebuffer pixel. PC cleanup completes after172.45s;disk
roundtrip,registered/running VM inventories and prior preferences are preserved.

An earlier fresh PC attempt requested its capture after five seconds and
received a0x0 framebuffer. Its PowerShell5 cleanup child then lacked
Get-FileHash. Lead recovered cleanup in PowerShell7 and verified the media
and inventories. The replacement used the proven30-second startup wait,
PowerShell7 and an independent300-second cap. The failed attempt is not a
product boot failure or a performed physical test.

The combined phase/defer Mac experiment boots to the scene. Its512x342 crop
and the normal PC baseline have identical175104 black/white logical pixels
at threshold128;full RGB colors differ. Exact crops only,no resampling or
retouching. This is a single baseline comparison,not dynamic parity proof.
The experiment closes at its300-second cap (300.64s observed cleanup);the
operator's result is still pending at this checkpoint. No motion pass is
inferred. All owned trial processes are closed;no guest shutdown is claimed.

## Direct diagnostic-read route

Lead read only Oracle's published readPhysicalMemory/readVirtualMemory
interface declarations and prose,not implementation code. The reference's
old4.0 note says these methods were unimplemented then;it does not establish
the installed7.2 result. In a new isolated normal PC run,lead separately
verifies an instruction pointer inside our own native image,then requests
only its16-byte original program header through readPhysicalMemory. The
installed API returns E_NOTIMPL80004001. No memory data is returned,no firmware
or unknown memory is inspected,and no input is injected. The owned trial
closes in36.48s with unchanged disk/inventory. Stop this route;do not retry
it or dump broad guest memory. [Published interface](https://www.virtualbox.org/sdkref/interface_i_machine_debugger.html).

The next diagnostic should expose the existing original Mac checkpoint
through registers,without changing the motion policy or patching guest state.
This is development work,not a new release exception. The paired E1-E3,
human-use,freeze and final publication requirements remain.286 progress is
retained;no new286 claim follows from these tests. New screenshots are
[indexed](../evidence/screenshots/2026-10-26/README.md);no successful-drag GIF
was made. Black/occluded captures and verbose private logs stay outside Git.
[Machine receipt](native-input-qualification-20261026.json).
