# Independent release groundwork - 2026-10-15

SRC-0198/0199; [ADR-0040](../adr/0040-independent-rom-system-releases.md).
This is release-management verification, not a new runtime build or boot result.

[The component registry](../../releases/components.json) assigns ROM draft
0.1.0-alpha.1 and leaves System unversioned/unreleased. Separate histories,
[milestones](../milestones.md), [compatibility matrix and publication process](../../releases/README.md)
keep the embedded ROM prototype distinct from a replacement System disk.
No release tag, GitHub release or published download is created.

The frozen candidate identifies the unchanged 65,536-byte original ROM:
SHA-256 `43c0296f808b85366fdbe55c9bcfd42e2daf9ef5fae77dfecfb85e5f80db0942`.
Compiled revision `2ea01129377d248ef48dd81d6f8873e948456ce6` and observation
revision `031212db9de4711d157b8e997aa91777f999a47b` retain their original scope.
[Existing qualification](loaded-app-redraw-evidence.md) remains byte-identical;
its 2,127 local / 2,625 CI checks were not rerun or counted as new checks here.

Verification performed for this groundwork:

- Metadata validation ties component/version/tag proposal, qualification hash,
  exact image, source revision, manifest and CI identity together.
- Two fresh local ten-file bundles have identical bytes and checksum inventories.
  Their allowlist contains only the original ROM, build/qualification records,
  notes, release record, source links, notices, checksums and DRAFT marker.
- Six refusal cases pass: altered own-ROM byte, wrong manifest, existing output,
  output inside checkout, source fingerprint mismatch and conflicting CLI mode.
  Refusals create no output; the existing directory stays unchanged.
- Fresh pinned Clang 23.1.1 / Ninja CMake configuration passes after removing
  the unused umbrella 0.0.1 version. No build-version consumers were found.
- All 197 runtime/test files and all native manifest inputs remain unchanged;
  all 24 untracked research originals and existing private copies match.
  UART comparison is mechanical only; no implementation/test bodies are read.
- Public indexed repository guard and HQ documentation/registry guard pass.

No new binary construction, fixture, runtime test, emulator/VM/reference run or
worker call was needed. The draft carries pinned original-source links; source
delivery must be reviewed before actual binary distribution. Human provenance,
native applications, historical identity, physical/full128K/parity/edition/B2,
input reliability and both required System1/ROM boot directions remain open.
