# Paired native selected-port demonstration -2026-10-18

SPEC-0047 / ADR-0053 / IMPL-0049 / TEST-0054; SRC-0274..0277.
An original square outline and split block now come from two separate prepared
ports. One shared native C workload sets/gets each current port,draws through the
resolved view with its own offset/clip,checks both operation counts and actual
pixels,and composes their union for the existing result panel. Each port remains
independently observable; the composite has2816 black pixels. APP/097,six phases,
branding,input counter and512x342 logical scene persist; PC output is centered.

The OS path executes natively on68000/x86. No interpreter,guest trap,image loader
or callback supplies this workload. The separate SPEC-0046 legacy binding remains
tested. Explicit --selected-drawing Mac and -SelectedDrawing PC modes preserve
the old demos. cs_scene_render stays the legacy utility; actual selected PC boot
entry uses cs_boot_scene_prepare. Native controller/UART and product renderer
bodies are unchanged. New candidates are unbooted; no VM/emulator/hardware/
reference trial or release. Existing release/boot results retain their identities.

## Worker and integration record
One Claude primary,one distinct-risk Copilot oracle; prior qualifications reused.
Frozen interface/contract precede both. Inputs22,691 and19,158 bytes; no private
HQ,untracked originals,manuals,credentials or prohibited implementation inputs.
Claude returned75 original C lines and5 exact replacements in63.67s,zero tools.
Copilot returned84 independent test lines in185.8s with10 view calls:9 packet
reads and one directory listing beyond instructions. That input-only directory
held only prompt.txt; no other file content was read. Deviation retained. Neither
worker edited repositories or executed tests; no retry or redundant review call.
Provider-reported consumption is retained in JSON; all four remaining allowances
are unknown and no cross-provider ChatGPT-quota savings are claimed.

Lead corrected the oracle's two product-renderer success-code assumptions,
GCC misleading indentation,fail-fast assertion handling and copyright; tightened
monochrome/reserved-byte,margins and exact counter-region checks. New implementation/
author logs were withheld from Copilot. Lead added a separately labelled fault
suite; invalid presentation is cleared on post-compute refusal while real native
port effects and diagnostics survive. No port rollback or fabricated success.
New build wiring preserves the literal default import contracts and replaces a
last-source positional dependency with the actual old-workload source index;
selected modules have independent missing-object controls.

## Executed verification
Final825 CTests across hostedClang,ASan/UBSan andGCC32 pass. The visual suite runs
482430 assertions,including every per-port/composite pixel and Mac-mono versus
centered PC output before/after input. Lead fault cases cover four refused drawing
operations and invalid final pixels.30 width/endian objects,18 existing import-
contract mutation tests and5 assertion-rejected broken implementations pass.
An earlier GCC build rejected oracle indentation; corrected before final runs.
Local PC: four O0/O2 twin EFI images,27 corruption rejections,21 omitted-object
controls; two original media writers agree and the independent filesystem/payload
checker passes. All target images remained unloaded. Actual native hosted render
previews are outside Git; their hashes are in the receipt,not boot screenshots.
CI/paired68000 candidate verification pending.

## Build/use and remaining gates
Build Mac with Build-LoadedAppROM.py --selected-drawing and the pinned tool root.
Build PC with Verify-UEFIImage.ps1 -SelectedDrawing,then
Package-SelectedDrawingPC.ps1 with two verified original media writers. CI retains
the candidate ROM,EFI/disk and sanitized manifests. A fresh explicit owner go-ahead
is required before either boot trial. This is not a release or full QuickDraw,
native SDK,external loader,physical/full128K/B2 or System1 boot qualification.
24 research originals/intake copies,UART,old rows/releases/tags and frozen wiki
are preserved; OneNote/wiki remain retired. Next propose paired visual boot trials
against these exact candidate hashes,with separate owner authorization.
