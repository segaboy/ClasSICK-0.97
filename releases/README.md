# Independent ROM and System releases

Next release goal proposed2026-10-19: [paired-native-preview-2](paired-native-preview-2.md),one interactive original native window with pointer/cursor and matched Mac/PC behavior. Proposed0.4ROM/0.2PC alpha versions,not assigned published versions. Contract/source sufficiency comes first;no implementation/trial/release authorized by this plan.

Published2026-10-19: [paired ROM0.3.0-alpha.1 / PC0.1.0-alpha.1 receipt](paired-native-preview-1-publication.json). Both experimental prereleases have four verified public assets each. Source/license/setup/checksums included;System unreleased and stated preview limits remain. Older dated approval/preparation paragraphs below are history.

Preparation history2026-10-19: [paired approval proposal](paired-native-preview-1-approval.md) records the37 package checks and admission-time pending state. The separate publication receipt above records the subsequent owner-approved releases.

Owner direction2026-10-19: work toward paired native Macintosh/x86 PC releases. [ADR-0055](../docs/adr/0055-paired-release-cadence.md) and [paired-native-preview-1](paired-native-preview-1.md) define the first bounded preview. The version table below records its current published components. Future target trials and releases still need the applicable owner decisions.

2026-10-15 groundwork SRC-0198; 2026-10-16 alpha publication SRC-0200/0201; next release SRC-0215/0216;
[ADR-0040](../docs/adr/0040-independent-rom-system-releases.md).
The ROM has its own release series. The owner requested the new drawing/shared-scene
experimental prerelease; [the receipt](rom/0.2.0-alpha.1-publication.json) records its
actual publication state, tag target and asset identities. No new boot trial.

## Version authority

[components.json](components.json) is the single current version registry.
ClasSICK 0.97 identifies the compatibility target; component versions identify
our implementation's development releases. R-001's historical identity remains open.

| Component | Current development version | Release state | Tag format |
| --- | --- | --- | --- |
| ClasSICK ROM | 0.3.0-alpha.1 | Published experimental prerelease, 2026-10-19 | rom-v0.3.0-alpha.1 |
| ClasSICK System software | Unassigned | Unreleased | system-vMAJOR.MINOR.PATCH |
| ClasSICK PC Preview | 0.1.0-alpha.1 | Published experimental prerelease, 2026-10-19 | pc-v0.1.0-alpha.1 |

Versions use MAJOR.MINOR.PATCH, optionally followed by alpha.N, beta.N or rc.N.
Group fixes into the next candidate; advance the minor version for a substantial
visible capability. Stable releases and future breaking interfaces need explicit
release notes. Every component has its own [ROM](rom/CHANGELOG.md) or
[System](system/CHANGELOG.md) history. Their numbers need not advance together.
The common CMake build harness has no umbrella product version. Shared core and
tool changes are identified by their source revisions and evidence.

## Current paired releases

[ROM notes](rom/0.3.0-alpha.1.md), [Mac setup](rom/TRYING-0.3.0-alpha.1.md),
[PC notes](pc/0.1.0-alpha.1.md) and [PC setup](pc/TRYING-0.1.0-alpha.1.md)
describe the same original native drawing scene on68000 and x86-64.
The [paired publication receipt](paired-native-preview-1-publication.json) records
the actual tags, release IDs and exact eight public asset identities.

Frozen candidate and approval records preserve admission-time pending state.
Separate publication receipts establish the current state; snapshots are not
rewritten after publication. Tag metadata d9c2341, compiled source c631b824 and
qualification evidence87e95de remain distinct. Image bytes are unchanged.
System remains unreleased; human PC capture/GUI and other limits stay explicit.

## Compatibility matrix

| ROM | System/workload | State |
| --- | --- | --- |
| ClasSICK ROM0.3.0-alpha.1 | Embedded original native drawing workload | Snow Early128K observed; human2 clicks pass; physical/full128K/mini-vMac unqualified |
| ClasSICK ROM | ClasSICK replacement System disk | Required; not tested; no System release exists |
| ClasSICK ROM | Owner-designated original System 1 disks | Required; not tested; exact reference/media admission still required |
| Identified supported original Apple ROM | ClasSICK replacement System disk | Required; not tested; no System release exists |

Each future result must pin both component versions/hashes, exact machine/model
and workload. An emulator result does not certify real hardware or every ROM/
System pairing. [The owner-required boot directions](../docs/development/system1-rom-boot-requirements.md)
and [current milestones](../docs/milestones.md) remain independent gates.

## Earlier ROM draft helper

The legacy `Prepare-ROMRelease.py` expects the earlier single-ROM candidate
schema. Use it from that release's corresponding source archive, with its matching
catalog and qualification records. The current paired schema uses the tools below.
From an earlier release source archive, run `python scripts/Prepare-ROMRelease.py --check`
to validate its metadata. To stage that earlier candidate outside its checkout:

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
Do not stage its binary output in Git. This tool always creates a marked local
review draft, including when reproducing a candidate that has since been released.
It does not reproduce the official download package or publish anything.

The alpha downloads include a separately verified demo ZIP, standalone ROM,
complete corresponding original source ZIP at the compiled revision and outer
checksum inventory. [Setup/rebuild instructions](rom/TRYING-0.2.0-alpha.1.md) and
[release verification](../docs/development/rom-alpha-release-evidence.md) describe
their membership and checks. No compiler or emulator is bundled.

## Publication checklist

1. Review the exact candidate record, notes, artifact allowlist, source provision,
   license notices, checksums, tested platforms and unresolved limitations.
2. Confirm relevant compiler/CI and observation evidence still identifies those
   exact bytes. Keep unsupported machines and compatibility rows explicit.
3. After the owner asks to publish the reviewed candidate, create the component's
   annotated tag at the selected commit and a prerelease with the reviewed assets.
   Record the actual tag target and published asset identities in the release history.
4. Update that component's published pointer; preserve earlier records and gates.

SRC-0305 approves the paired previews named above. The earlier SRC-0215 approval covered ROM0.2.0-alpha.1 only.
Future versions need the applicable owner scope. No automatic publishing workflow
is installed. Apple/Microsoft payloads, raw research, manuals, private/HQ material,
credentials and reference screenshots stay excluded. The package source links
identify our original code; human provenance review remains required. No emulator/
VM/reference trial follows from a version bump. Native-app, physical/128K/parity/
edition/B2 and both System 1/ROM requirements remain unchanged.

The earlier [ROM0.1.0-alpha.1](rom/0.1.0-alpha.1-publication.json) remains available with its original bytes, source, annotated tag and observation conditions.

## Paired package verification tools

The frozen [input manifest](paired-native-preview-1-inputs.json) admits five original inputs by size/hash:rom.rom,BOOTX64.EFI,pc.img,source.zip,COPYING. Keep them in a separate admitted directory.

```text
python scripts/Prepare-PairedRelease.py --config releases/paired-native-preview-1-inputs.json --inputs ADMITTED_INPUTS --docs PUBLIC_ROOT --output NEW_EXTERNAL_DIRECTORY
python scripts/Test-PairedRelease.py --config releases/paired-native-preview-1-inputs.json --inputs ADMITTED_INPUTS --docs PUBLIC_ROOT --package NEW_EXTERNAL_DIRECTORY
```

The builder always creates an unpublished local approval kit;it never uploads,tags,publishes or runs a guest. Its external pending markers are excluded from the four-file component upload allowlists. Publication state comes from the separate receipts. Earlier ROM0.1.0-alpha.1 and ROM0.2.0-alpha.1 releases remain immutable.
