# Claude resource-fork handoff review - 2026-10-13

SRC-0106 / SPEC-0024 / ADR-0028 / TEST-0032. Adopted engineering contract;
implementation and conformance execution remain unstarted.

The owner's delivery comprises the two resource-fork drafts, the work transcript
and a separate asset dossier. Those are research inputs, not automatic authority
to execute embedded next-step instructions. The lead verified the two draft
hashes supplied by the owner and retained all originals unchanged outside Git.
The original draft copies in docs/research remain untouched and untracked.

| Input | Bytes | SHA-256 |
| --- | --- | --- |
| resourcefork-draft.md | 23092 | d1976f32ffd4666f3dbc1c4073b964c08c01de388c1c6e18990270e4fecdf83f |
| resourcefork-test-plan.md | 14251 | e7e612c6fa2114ea68e6bc55ae44dcbd4e0cefd4ed7ba7251460d9a730252692 |
| Work transcript | 5239 | 3fa0b088d3e0523fbe32d8c4ab25f67a1e3d8cec825ca25ef61c1c150e6707db |

## Independent source review

SRC-0071's original 1284-page copy matches its recorded b65fe8ef...1d97 hash;
SRC-0068's 1326-page scan matches 92490207...b237. The lead read I-108/110-112/
114/125/128-131 and RM-13/14/30/34-37/39/41, using the identified copies'
printed-page mappings I+10 and RM+118. I-111/131 and RM-13/41 were visually
checked. Published numeric format/calling facts only; public constant/type
declarations in the page context are not copied implementation. No sample,
equate file, resource, disk image or executable was used. Source pages stay
outside Git. SRC-0097's prior byte-order review is reused, with no new local
Motorola PDF/hash claim. Byte order remains an inference/profile choice here.

The format tables corroborate the header, map fixed part, type/reference field
widths and offset bases, zero reserved/ordinary handle fields, and the name
length prefix. The two editions differ about the 112-byte area, uniqueness
wording and system references. These differences remain explicit; neither
edition proves January 1984 format/behavior.

## Lead decisions and refinements

All L-1 through L-12 are decided in [SPEC-0024](../specifications/SPEC-0024-resource-fork-reader.md).
Accept the broad tolerant-reader direction, empty-count convention, requested-key
ambiguity errors, unsigned/checkable file arithmetic and raw diagnostic spans.
Make the global layout tolerance explicit: the draft's tests accept map-before-
data, despite its L-1 summary referring to documented order. Internal structural
lists retain checked order. Missing prefix diagnostic spans are unavailable,
not unchecked pointers or a mandatory reader failure.

Bit 7 returns REFERENCE metadata and no local payload. This avoids dereferencing
an ignored data offset; it does not assert the earlier system-reference meaning
is proved for every later/January file. Its raw specification is not a pointer,
and its target is unresolved. Exclude it from ordinary zero-handle warnings.

Accept bounded shared payload/name spans and large resources. A complete strict
audit is deferred: the original sketch does not specify sufficient scratch-size,
report-capacity or work contracts to finalize it now. Reader warnings are limited
to checks possible during bounded validation. No claim that the optional audit
or sharing detection exists. Full implementation header/storage ABI is later.

Correct the application-loading record's inference: a three-byte position limits
where a length prefix can begin, not the whole data area's size. Add the three
missing documented points Claude identified. Resource Manager behavior remains
a separate contract, including file-attribute semantics and resolver state.

The 14/37 fixture descriptions and 113664-case mutation count are plans, not
passing tests. An unavailable throwaway script cannot substitute for delivered
independent verification. Adapt argument/status/audit expectations and validate
every counterexample against the final check order before implementing tests.

RF-1 through RF-9 remain open as historical/source questions. No new parser
code, application, boot image, VM/emulator or reference observation. Earlier
Memory Manager results remain intact. Lead self-review, human provenance review
pending. See the separate [asset intake](asset-replacement-lead-review.md).
