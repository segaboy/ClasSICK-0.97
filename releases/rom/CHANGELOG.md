# ClasSICK ROM release history

## 0.2.0-alpha.1 - 2026-10-16

- Original loaded68k application now paints/erases its fixed port to produce
  a hollow rectangle; shared monochrome512x342 scene and original branding.
- Exact current ROM observed in Snow128K; no input qualification in that run.
- [Notes](0.2.0-alpha.1.md), [candidate](0.2.0-alpha.1.json),
  [publication state](0.2.0-alpha.1-publication.json). System remains unreleased.
- Published experimental prerelease; all four public downloads and GitHub
  digests match the reviewed local assets.
- Minor version advances for the new visible drawing capability. The first
  alpha's bytes, annotated tag and records remain immutable.


## Dated development between 0.1.0-alpha.1 and 0.2.0-alpha.1

- Shared monochrome512x342 product scene and original monitor/C branding now
  compile for68k and native-PC; [verification](../../docs/development/shared-product-scene-evidence.md). No new release or trial.

- [Selected loaded rectangle calls](../../docs/development/drawing-app-evidence.md)
  add a new original drawing-ROM candidate, now observed in one isolated
  [Snow trial](../../docs/development/drawing-app-boot-trial.md). No new version/tag/release has been
  published; the existing alpha image and source pins remain immutable.

[First experimental prerelease](https://github.com/segaboy/ClasSICK-0.97/releases/tag/rom-v0.1.0-alpha.1)
published 2026-10-16. The System track remains unreleased.

## 0.1.0-alpha.1 - 2026-10-16

- Original 64 KiB Macintosh 128K boot/application prototype.
- Exact tested candidate, source/evidence pins, capabilities and open limits:
  [release notes](0.1.0-alpha.1.md), [frozen candidate record](0.1.0-alpha.1.json).
- Annotated `rom-v0.1.0-alpha.1` tag at preparation revision
  `06dd25ef7e139f947d50710db2acc8b841cb68ad`; compiled source remains
  `2ea01129377d248ef48dd81d6f8873e948456ce6`. No new binary build or trial.
- Four downloaded assets match local bytes and publisher digests:
  [publication receipt](0.1.0-alpha.1-publication.json).
- No System disk boot, original-System interoperability or physical qualification.

Artifact bytes and the annotated tag are immutable; later changes use a new version.
[Independent component policy](../README.md).
