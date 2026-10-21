# Screenshot evidence storage and publication

Owner instruction2026-10-18 (SRC-0280): store continuing screenshots in the public
repository so the owner's morning research updater can discover evidence and use
it in web-facing content. This is ordinary Git evidence publication, not a new
target-trial, release, reference-system observation or OneNote/wiki permission.

Use [the screenshot index](../evidence/screenshots/README.md) and its
[machine-readable manifest](../evidence/screenshots/manifest.json). Save reviewed,
unchanged product-framebuffer PNGs under `docs/evidence/screenshots/YYYY-MM-DD/`.
Link the matching research/test record to the image collection. Future approved
captures follow this rule; verbose logs and transient UI-automation snapshots stay
outside Git. Inspect every image for private paths, account data, desktop content,
credentials, protected/reference pixels and unrelated third-party assets before
publication. Do not publish any such content under this exception. Capture a clean
own-product framebuffer when needed; another target launch still needs its own
authorization. If a safe capture is unavailable, record the gap rather than
publishing an unsafe image. Never silently alter evidence or present a preview as
an actual boot capture.

Each manifest entry records identity, relative path, exact SHA-256 and byte count,
dimensions, local capture date, target, caption, alt text, original compiled-source
and observation-record revisions, evidence document, GPL-3.0-or-later license,
rights note, input classification and limitations. The directory date equals the
recorded local date. Use honest descriptions: synthetic input is not human input,
and a native prototype screenshot does not establish historical compatibility.
Captions/alt text are metadata, not proof of the claimed behavior.

The publication guard accepts only individually registered indexed PNGs, max1MiB
each, with checked dimensions/hash/structure/provenance metadata. All other binary
and source/secret exclusions persist. It checks the Git index rather than a
different working copy; no arbitrary `.png` extension exemption exists. Structural
PNG checks do not prove originality, privacy, complete raster decodability or
historical truth. Human image/rights review and actual decoder verification remain
part of intake. Screenshots are a compact evidence exception, not build/media
storage. Current collection:7 original captures,66,476 PNG bytes.

The initial collection backfills the latest SPEC-0047 paired boot trial from
unchanged private originals. Their earlier records retain their dated status;
publication copies add discovery links and hashes, with no new boot or rebuild.
The private logs and earlier capture originals remain preserved. No retired wiki
queue or OneNote entry is created or maintained.

Format cross-check: SRC-0286,[PNG1.0 File Structure](https://www.w3.org/TR/PNG-Structure.html) and [critical chunks](https://www.w3.org/TR/PNG-Chunks.html),W3C Recommendation1996. This is a selected publication profile,not a universal decoder: the uppercase third chunk byte is an encoding restriction chosen here,while §3.3 advises decoders to tolerate future use of that bit. Version1 metadata has an exact field set; extensions require a reviewed schema change. No PNG sample implementation is an input.


## One reviewed hosted animation - 2026-10-21

SRC-0332 owner requests interesting test screenshots and a dragging GIF.
The PNG rules remain unchanged. One separate exact-path/exact-SHA GIF exception
is registered in docs/evidence/animations.json and checked from the Git index by
scripts/check_animations.py. That guard checks the pinned identity/header/metadata,
not general GIF validity. Lead independently decoded every one of49 frames with
existing Pillow12.3.0 and compared RGB bytes against actual original Win32 client
captures;the audit records each frame hash and chosen4.2-second presentation loop.
Any different GIF requires a new explicit source/rights/decode review and guard
change. It is never admitted by extension alone. Original client pixels only,
GPL-3.0-or-later;no desktop/window chrome/reference/emulator content.
[Stills and animation](../evidence/screenshots/2026-10-21/README.md).
