# Resource-fork reader checkpoint - 2026-10-13

SPEC-0024 / ADR-0028 / IMPL-0031 / TEST-0032 / SRC-0106/0126-0128.
Compiled source `12208ed5591c17fbd4c031c22e138eecc69f2671`; [CI](https://github.com/segaboy/ClasSICK-0.97/actions/runs/38444454769) passes both jobs and the
complete original 68000 manifest matches local. Lead self-review; human
provenance review pending.

The reader validates an immutable resource fork and returns exact type/signed-ID
metadata. It checks all structural lists before names/payload spans, refuses
ambiguous requested keys while leaving unrelated keys usable, and reports bit-7
references without following their ignored local-data offset. No allocation,
mutable globals, native struct casts, payload scan or application execution.
Refusal preserves input/view/entry; argument refusal preserves the report too.

The complete nine-profile matrix passes 1,479 CTests, including Clang/GCC,
32/64-bit, debug/release and validated sanitizers. Each configuration compares
113,664 single-byte mutations against the delivered independent integer model:
1,022,976 comparisons locally. Python/C encoders independently agree on the
444-byte original fixture; generated model hashes match across all nine builds.
Fourteen small variants have 4,232 truncated-length checks per configuration;
tolerated trailing-byte removal can remain valid. Twenty-seven explicit malformed
cases test arithmetic, bounds, ordering and error precedence. Additional cases
check duplicates, warning combinations, shared spans, output retention, unaligned
input, independent views and diagnostic prefix spans overlapping tolerated regions.

Five large variants cover unsigned 0x8000 positions, the maximum 24-bit length-prefix
offset with a data region beyond 16 MiB, and a 32,769-byte resource. A 5,000-entry dense
list checks bounded work. Large fixtures use semantic boundary truncations; they
are not stored in Git or claimed to fit 128K RAM. Ten extra width/endian object
audits and both compiler debug executable twins pass. GNU68000 O0/Os passes two
module audits and two complete relocatable links with no runtime imports/global
state. Descriptors cost 60/40/24 bytes (124 total); reader-closure frame maxima
are 216/192. These are partial costs, not stack high-water or final budgets.

CI executes the entire final-source 1,479 matrix plus 354 bootstrap checks. All
six prior EFI profiles and both own ROM profiles/input fingerprints are unchanged.
Six original research drafts are unchanged/untracked; concurrent Claude launch
research is separate. Preparation corrections are retained in the JSON record;
earlier manifests are superseded, not relabeled as final evidence.

This is a format service, not Resource Manager loading, MFS decoding or an
application loader. Optional strict audit/reference resolution remain deferred.
No new boot binding, native application/SDK, VM/emulator/reference trial, UART
body work, delegation, chats, automation or OneNote. Historical, physical,
parity, three editions, full budgets and B2 criterion 6 remain open; the owner
native-application gate stays closed. Further CODE-only launch contracts must
use separately reviewed launch/segment research and explicit prerequisites.

[Machine-readable evidence](resource-reader-evidence.json).
