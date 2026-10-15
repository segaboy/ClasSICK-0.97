# Independent ROM and System releases

2026-10-15, owner direction SRC-0198; [ADR-0040](../docs/adr/0040-independent-rom-system-releases.md).
The ROM has its own release series. Preparing this groundwork creates no tag,
GitHub release, published download or new boot qualification.

## Version authority

[components.json](components.json) is the single current version registry.
ClasSICK 0.97 identifies the compatibility target; component versions identify
our implementation's development releases. R-001's historical identity remains open.

| Component | Current development version | Release state | Tag format |
| --- | --- | --- | --- |
| ClasSICK ROM | 0.1.0-alpha.1 | Draft, unpublished | rom-v0.1.0-alpha.1 |
| ClasSICK System software | Unassigned | Unreleased | system-vMAJOR.MINOR.PATCH |

Versions use MAJOR.MINOR.PATCH, optionally followed by alpha.N, beta.N or rc.N.
Group fixes into the next candidate; advance the minor version for a substantial
visible capability. Stable releases and future breaking interfaces need explicit
release notes. Every component has its own [ROM](rom/CHANGELOG.md) or
[System](system/CHANGELOG.md) history. Their numbers need not advance together.
The common CMake build harness has no umbrella product version. Shared core and
tool changes are identified by their source revisions and evidence.

## Current ROM candidate

[Draft notes](rom/0.1.0-alpha.1.md) and [frozen candidate record](rom/0.1.0-alpha.1.json)
name the tested 64 KiB original ROM, compiled source, exact SHA-256, compiler
manifest, evidence revision and limitations. The version is carried by the
filename and release metadata; no bytes are patched into the qualified image.
Its on-screen 0.97 remains the target label. This is an embedded-demo/application
prototype; a complete System disk boot has not been demonstrated.

The proposed tag has not been created and its target revision is unassigned.
Before publication, select the reviewed metadata/source commit, confirm that
every native build input equals the qualified manifest, and distinguish that
tag revision from the compiled-source and observation revisions. Changed ROM
bytes require new build evidence and the appropriate separately authorized trial.
Published artifact identities are immutable; later changes receive a new version.

## Compatibility matrix

| ROM | System/workload | State |
| --- | --- | --- |
| Current ClasSICK ROM draft | Embedded original scene and original 68k test application | Observed in isolated Snow Macintosh 128K; input delivery remains unqualified |
| ClasSICK ROM | ClasSICK replacement System disk | Required; not tested; no System release exists |
| ClasSICK ROM | Owner-designated original System 1 disks | Required; not tested; exact reference/media admission still required |
| Identified supported original Apple ROM | ClasSICK replacement System disk | Required; not tested; no System release exists |

Each future result must pin both component versions/hashes, exact machine/model
and workload. An emulator result does not certify real hardware or every ROM/
System pairing. [The owner-required boot directions](../docs/development/system1-rom-boot-requirements.md)
and [current milestones](../docs/milestones.md) remain independent gates.

## Prepare a local review bundle

Run `python scripts/Prepare-ROMRelease.py --check` to validate the draft metadata.
To stage the exact admitted candidate outside the checkout:

```text
python scripts/Prepare-ROMRelease.py --rom PATH_TO_QUALIFIED_ROM --build-manifest PATH_TO_QUALIFIED_MANIFEST --output NEW_EXTERNAL_DIRECTORY
```

The tool verifies frozen image/manifest/qualification identities and current
native source fingerprints before creating a fresh directory. If the current
checkout has moved on, `--source-root QUALIFIED_SOURCE_CHECKOUT` supplies the
matching source tree. Output inside either source checkout or an existing output
directory is refused. It copies only the approved original ROM and named text
records, notices, source links, checksum inventory and a prominent DRAFT marker.
It performs no network request, Git mutation, tag, publication or guest launch.
Do not stage its binary output in Git. This is a review bundle, not a release.

## Publication checklist

1. Review the exact candidate record, notes, artifact allowlist, source provision,
   license notices, checksums, tested platforms and unresolved limitations.
2. Confirm relevant compiler/CI and observation evidence still identifies those
   exact bytes. Keep unsupported machines and compatibility rows explicit.
3. After the owner asks to publish the reviewed candidate, create the component's
   annotated tag at the selected commit and a prerelease with the reviewed assets.
   Record the actual tag target and published asset identities in the release history.
4. Update that component's published pointer; preserve earlier records and gates.

This groundwork authorizes no publication. No automatic publishing workflow is
installed. Apple/Microsoft payloads, raw research, manuals, private/HQ material,
credentials and reference screenshots stay excluded. The package source links
identify our original code; human provenance review remains required. No emulator/
VM/reference trial follows from a version bump. Native-app, physical/128K/parity/
edition/B2 and both System 1/ROM requirements remain unchanged.
