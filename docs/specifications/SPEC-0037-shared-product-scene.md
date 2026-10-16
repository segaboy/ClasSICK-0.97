# SPEC-0037 - Shared monochrome product scene

Finalized 2026-10-16 under owner implementation instruction SRC-0209 and
continuing PC canvas requirement SRC-0210. Branding reference SRC-0211;
ADR-0042 / IMPL-0041 / TEST-0045. This is an original engineering scene.

## Canvas and presentation

All PC work targets a logical 512x342 image until the owner changes this rule.
Macintosh uses that native size. PC output centers that same unscaled canvas
in the framebuffer reported by the platform; 640x480 therefore has origin
(64,69). No assumption that the hardware supports a 512x342 video mode and no
640x480 mode requirement. Hosted development windows may use the existing
integer presentation zoom; their logical image remains512x342. Native-PC
margins are white. Undersized conformance
surfaces clip a top-left-anchored canvas; no fractional scaling or stretching.

One freestanding architecture-neutral C renderer supports MONO1_MSB and RGBA8
bands. Logical black is mono1 / RGBA 000000FF; white is mono0 / FFFFFFFF.
PC framebuffer reserved byte remains zero through its existing RGB/BGR adapter.
Logical geometry, typography and pixels are identical at equal scene state.
Only logical pixels may change: row padding, unused mono bits and tail survive.
Validate descriptors/dimensions/format/state before writes; no allocation,
native device operation, guest address conversion or mutable global in renderer.
Caller supplies live disjoint state, image and output. Image is borrowed for
the synchronous render only, 128x64 MSB mono / 1024 bytes when nonnull.

## Content

White canvas; four-pixel border at x[8,504), y[8,334). Original stepped monitor
and C mark at x[36,84), y[28,84), manually approximating the supplied artwork in
black/white. Original project lettering spells ClasSICK 0.97 at (112,34), scale4,
24-pixel character stride. APP at y65; three low decimal result digits at
(214,86), (246,86), (278,86), scale6. These describe the actual shared probe.

Black result panel x[48,464), y[128,208), white inset four pixels. Actual image
copied at (192,136). Six phase marks at y214, x64+64*i, 40x8. Sixteen progress
boxes at y232, x48+26*i, 20x28, two-pixel borders; progress clamped to16.
Four decimal digits at (80,286) for result and (352,286) for input count,
scale4, stride20, low digits only. Failure retains distinct side bars. Input
update changes only logical x[352,432), y[286,306). Center translation applies
equally to complete render and counter update.

## Portable probe and platform bindings

The drawing ROM and native-PC boot scene both use the existing original
40-byte 68k resource program through SPEC-0036's portable core. No native x86
application is loaded or run: this is ClasSICK's own native bootstrap code
executing the existing original 68k test fixture. Gateway/workspace initialization
and exclusive image lifetime stay unchanged; no loader/CPU/trap expansion.
PC preparation reserves aligned workspace plus at most64KiB RGBA band storage
inside its existing128KiB owned arena. No stack-sized workspace or hidden heap.
Scene uses actual result/phase/port data, never replaces a failed probe with a
synthetic success. The legacy pure scene helper uses a declared original
geometric demo when it has no probe image; it makes no execution claim.

PC Space/Escape input and qualification traces retain their existing meanings;
the visible input count is actual accepted nonrepeat Space presses. This work
does not add a PC mouse driver. Macintosh retains its existing mouse polling
and counter-only update. Hosted viewer uses the same logical canvas.

## Verification and limits

Compare complete Mono512x342 and both PC640x480 framebuffer orders at matching
states; test white margins, odd frame dimensions, bands, clipping, padding,
canaries and exact counter-only preservation. Use original independent pixel
expectations and real shared probe results, compiler widths/endians/sanitizers,
freestanding/import checks and unloaded native-PC/68000 builds.

No emulator/VM/reference run is authorized by this contract. Fresh explicit
owner trial approval still precedes each run. Preserve prior observations and
released alpha bytes/tag,24 research originals/copies and old HQ registry rows.
Native-app, source/UART, historical, physical/full128K/parity/edition/B2/human
and both System1/ROM boot gates remain. No OneNote/new chats/agents/automation.
