# TEST-0020 — Retained artifact integrity and native boot readiness

2026-10-08. Owner-requested audit SRC-0055, reusing TEST-0019, SPEC-0007–0013
and the existing native-PC/firmware records. Documentation/evidence only.

Audit public revision `653c442a9fa83e4bb2bfea727f43acb0a727f027`; compiled/test/
script source `3b44cb9510a400397a33cd1cc0802f45c36c8110`. Hash all 117 recorded
local inputs, compare to their Git blobs with explicit text line-ending handling,
compare all 59 published/local/CI fingerprint labels, find all 53 distinct hashes
in retained twin owned artifacts, and parse the original nine CTest summaries.
Freshly invoke the unchanged owned Test-UEFIImage.ps1 auditor on four retained
images with their linker maps and on the O2 payload with its identical-image map.

All integrity/image checks pass. The retained full log SHA-256 remains
`71b7ef11904493f4cd828daa270bafa04ef64aeb29c5235d5a98f31e2f9e34c4`.
117 local hashes match; 116 committed blobs are byte-identical. The prior PS/2
fingerprint JSON has local SHA-256
`5c86b47932e90320fc1ae7d82303510588c393fe42031928b7c47d3aca6491d3`
and LF Git-blob SHA-256
`9bb34491ad821c8b924398d12322347a79d765021f2bf477bd01d9e98f26313e`;
CRLF-to-LF normalization under text/eol attributes is the only difference.
No source file was altered to make the comparison pass.

Final collection/image evidence root: boot-readiness-audit-20261008-c.
Initial -a collection stopped on the mistaken all-blobs-byte-identical assumption.
The -b collector stopped because it expected `100% tests passed, 0 tests failed
out of N`, while the pinned log says `100% tests passed out of N`.
Both failed collectors and corrected final results are retained. These are audit
collector corrections, not newly observed product failures. No image/test program
or firmware was executed by either failed collector.

The full CTest matrix and prior mutation/omitted-object controls are prior
TEST-0019 evidence, not rerun counts. New execution consists of metadata checks
and five unloaded EFI inspections. No firmware callbacks or privileged guest
instructions ran. The public staged-index guard passes 266 text/source files,
and the staged diff whitespace check passes. The first direct Windows PowerShell
file launch was refused by its session execution policy; invoking the owned
guard with a process-local ExecutionPolicy Bypass passes. No persistent host
policy was changed. HQ registration and its documentation guard are a separate,
later step; they had not been performed when this record was published.

Readiness decision is NO-GO: selected output is a one-file directory tree,
formatted media is unverified, exact firmware closure remains needs-review,
machine qualification is unverified and B2 is not passed. Media inventory scope
is the selected retained EFI root and tracked platform/scripts/specification
paths. It does not claim there are no other disks anywhere on the workstation.

[Full findings](../../docs/development/boot-readiness-audit.md) and
[machine-readable evidence](../../docs/development/boot-readiness-audit.json).
Aggregate audit/publication by the coordinating lead only; UART spec/code/test
work remains the fresh implementer's recorded work. Human provenance review
pending, no certified clean-room staffing claim, no new dependencies or assets.
All edition boots/parity and independent real Macintosh/mini vMac goals remain
unchanged. Git wiki planning is handled in HQ; no live OneNote publication.

The owner paused Codex after staging this audit. Claude (Anthropic; session
configured as `claude-opus-5-5`) then completed publication from the staged
documents. It reviewed their text and links, clarified the pending-HQ sentence
above and this attribution, and performed no new collection, execution or
UART-related review.
