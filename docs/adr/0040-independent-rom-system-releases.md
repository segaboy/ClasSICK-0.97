# ADR-0040 - Independent ROM and System releases

2026-10-15; accepted owner-directed release groundwork, SRC-0198/0199.

The owner states "I think the rom should be its own release..." and then
"ok, do the groundwork for this now." The ROM and replacement System software
have different artifacts, compatibility requirements and delivery readiness.
An umbrella version would obscure those boundaries and imply progress in a
component that has no release yet.

Keep shared development in this repository, with independent ROM/System versions,
tag prefixes, release histories and an explicit compatibility matrix. The current
ROM is a draft 0.1.0-alpha.1 prototype. The System version stays unassigned until
there is a release candidate. Preserve ClasSICK 0.97 as the compatibility-target
name; remove the unused CMake 0.0.1 umbrella placeholder.

Use releases/components.json as current version authority and freeze a candidate
record around the already qualified image, source, manifest and observations.
Local preparation copies only that admitted original image and approved text
records into a fresh external directory, marked DRAFT, with a checksum inventory.
Do not patch version bytes into the observed ROM or manufacture a new boot result.

This decision creates no release/tag, publishing automation, repository split,
new boot/guest ABI, native application or emulator/reference trial. Existing
build/evidence/edition records remain exact. Actual publication is separately
requested against a concrete reviewed candidate. Verify tuple/hash/source and
failure behavior for the preparation tool, repository guards and the unaffected
native inputs; a complete runtime regression is unnecessary for metadata-only
groundwork. [Release policy](../../releases/README.md).
