# ROM 0.2.0-alpha.1 release verification - 2026-10-16

SRC-0215/0216. The owner requested another ROM release after current-build boot
captures. Version0.2.0-alpha.1 advances the minor version for the newly visible
loaded-application drawing capability. The exact previously compiled/observed
ROM is packaged; there is no new native build or emulator/VM/reference trial.

Compiled source `a8b6aae8e583b26d16012d5af330b2a2092c6c59`; observation revision `eee4327f2cd70c287e56d10c38c8d09da17dd012`.
The release-preparation tag target will identify metadata/source review rather
than another compiled image. All native input fingerprints must match both
the complete source archive and current checkout. Existing compiler/CI checks
are reused with their original counts and conditions.

The four allowlisted assets are a deterministic ten-member demo ZIP, standalone
65536-byte ROM, complete tracked original source ZIP at the compiled revision,
and outer SHA256SUMS.txt. The demo contains ROM, setup/rebuild README, release
notes, release/build/qualification metadata, LICENSE, COPYRIGHT.md, source
identity and internal checksums. Matching source includes build/linker/setup
scripts and pinned tool definitions. Untracked originals, Git history, private
manuals, HQ, tools, third-party binaries/assets and emulator code are excluded.
Mechanical archive/hash checks include UART files without reading their bodies.
The existing GPL-3.0-or-later grant, author notices and corresponding original
source accompany the binary. This is engineering package/provenance review;
human clean-room/legal certification is not claimed.

Before publication: compare demo twins/member bytes/CRCs, exact source tree and
native fingerprints, all asset checksums and retained previous-release inputs;
run the repository guard and metadata/draft preparer checks. Create an annotated
tag at the reviewed commit, upload to an unpublished GitHub draft prerelease,
then download and verify all four assets. Publish only after this succeeds;
keep prerelease=true/latest=false. Re-download public assets and compare their
bytes, GitHub digests, tag object/target and receipt.

The [receipt](../../releases/rom/0.2.0-alpha.1-publication.json) records prepared
versus actual published state. The [exact candidate](../../releases/rom/0.2.0-alpha.1.json)
keeps its frozen draft snapshot. System remains unreleased; the original alpha
and prior compiler/boot conditions remain unchanged. Input/human, historical,
physical/full128K/native-app/parity/edition/B2/source/UART and both System1/ROM
gates persist. No workers, new chats/agents, automation or OneNote.

## Local preparation - passed

The657-file source archive equals the exact compiled Git tree. All60 native
input fingerprints match archive/current bytes. Ten demo members, their bytes
and CRCs, deterministic demo twins and all four outer asset identities pass.
The local draft preparer accepts the new candidate and rewrites its versioned
setup/qualification links. Both old and new prerelease metadata validate; an
incorrect published-version pointer is refused. All24 originals/existing copies,
211 runtime/test/old-candidate files and the four first-alpha assets are unchanged.
The live published pointer stays on0.1.0-alpha.1 until actual new publication.

## Publication - passed

The [release](https://github.com/segaboy/ClasSICK-0.97/releases/tag/rom-v0.2.0-alpha.1) is published: release ID415149789,
`2026-10-16T16:14:57Z`, draft=false/prerelease=true; stable latest not requested.
Annotated tag object `27ddb67bea88d2f2630253c5f6213444e0739037` points to preparation commit
`c7700ca9897258aee7907ce6e7e938ba7936215f`. All four draft and public downloads equal
the local allowlist; publisher SHA-256 digests, sizes and outer checksums match.
Total download size is1742793 bytes, including
1619157 bytes of matching source. Source/build/boot/tag identities are distinct
and retained in the receipt. This closes distribution verification only.
