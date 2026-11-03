<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Paired preview 2: source archive preparation

2026-11-03. To reduce the remaining packaging work, two local Git source archives were generated from revision `e3cd4d867a34a72924cd7fe2693d450ca8758b64`, the source revision recorded for the current draft images. They are byte-identical, 4,012,886 bytes, SHA-256 `a1d3e2e263d2c201c35ee1b268e2267926380092e49bcde905ff30576117a031`. An independent local audit compared all 1,675 regular Git blobs and names with the archive, rejected duplicate/case-colliding and encrypted entries, checked ZIP integrity, and confirmed that the archived `LICENSE` is byte-identical to the draft's `COPYING`. Local untracked research is excluded.

The first archive metadata audit incorrectly rejected Windows Git's unspecified ZIP mode 0; the final audit accepts it only after verifying that the corresponding Git tree entry is a regular blob. An initial check looked for `COPYING` inside the source archive; the committed file is named `LICENSE`, and the corrected comparison passed. Both initial failures remain recorded in the local audit.

The files and audit are at `C:\ClasSICK\paired-preview-2-source-draft-20261103-a`. This is preparation, not a release package or a source freeze. The existing PC build manifest identifies only a subset of its inputs; the independent rebuild below supplies additional evidence. A changed candidate requires a new archive; final license/provenance, package, input and approval gates remain open. No tag, release, external reference or hardware test is implied.

## PC source archive reproduction

The verified archive was extracted into a fresh scratch directory, with the existing installed tool directory connected separately. The archived `Verify-UEFIImage.ps1 -WindowDrawing` built two matching O0 images and two matching O2 images and passed its 27 image-corruption and 26 omitted-object controls. The O2 EFI SHA-256 was `8e871da4e540da3f09498e3729dce55d92c9f6f95ecec464dcf6b778f0b55435`, identical to the EFI payload in the existing local review draft and the previously tested PC image. After the build, all 1,675 archived file contents were independently compared byte-for-byte with the extracted source files and remained unchanged. Build output and controls are outside the extracted tree at `C:\ClasSICK\pc-source-rebuild-20261103-a\build`.

This establishes a reproducible PC payload from the archived project source under the locally installed tools; it does not retroactively turn the earlier subset manifest into a complete build-time record, establish every external compiler/header byte, validate an altered candidate, or prove input behavior. No emulator or VM was run for this reproduction. The existing input, package, license and release gates remain in force.
