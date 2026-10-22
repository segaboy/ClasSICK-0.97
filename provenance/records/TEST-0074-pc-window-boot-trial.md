# TEST-0074 - Exact PC window candidate black-box trial

2026-10-22; SRC-0359..0361; SPEC-0059/0061/0062; IMPL-0064.
One authorized cold start, now closed. [Trial and exact identities](../../docs/development/pc-window-boot-trial.md).

|Actually executed check|Result and limit|
|---|---|
|Image/VDI identity before and after|Exact EFI/raw hashes and both RAW roundtrips match|
|Startup|Own window/branding visible; initial 0x0 capture failed, same-boot continuation succeeded|
|16 decoded framebuffer/counter checks|Black/white, white margins, unscaled centered scene; exact 0/1/2/3/4 glyph pixels|
|Body, held movement and release|One press changes pattern/count; held/release add no click|
|Title and three drag moves|Count2; three20x10 translations; four224x144 crops match|
|Preserved region|127688 pixels outside old/new window bounds and counter unchanged|
|Content and later press|Checkerboard/count3, then stripes/count4|
|70s interval, keys, later mouse|Identical frame after keys, later mouse works; actual key drain/guest time unknown|
|CWP1 attempt|Blocked: public read method not implemented, no returned memory bytes|
|External close, inventory, media|Powered off/unregistered; previous VM states and raw bytes preserved|

Original-glyph pixel readback, not OCR or guest self-report. Crop comparisons
test captured raster translation, not an independent full renderer or every
restored-background pixel. E2 synthetic input bypasses host capture/focus;
E3 GUI/E4 human and the earlier intermittence report remain open. No target
fault/loss/saturation/semantic-stop testing. Earlier348 CTests/CI reused, not
rerun. No runtime change, new worker, Mac trial, release or further permission.
