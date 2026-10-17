# Guest drawing-port contract lead review

2026-10-17. SPEC-0042 / ADR-0048; SRC-0244..0248. Contract only, no code,
fixtures, tests, registry entries, builds, target run or release. Public baseline
`ac201f160a90333e9f91a69f9ba31aac6d13ee7e`. Existing native implementation remains0563837; latest candidates unbooted.

## Source checks
Both eligible local publications retain recorded identities: SRC-0071,19245372
bytes/1284 pages/SHA-256 b65fe8efe782799a333a0b59b225e2bbaf9e2e150dc949dd555571971b331d97;
SRC-0068,53888388 bytes/1326 pages/SHA-256
924902074a9798457949c46f83ac016c98b10f50258ef36c277dcd0f8278b237.
Independent image checks: SRC-0071 I-148 public GrafPort fields (PDF158), I-165
Get/Set signatures and selection semantics (PDF175), I-209 numeric grafSize206
(PDF219), III-217 GetPort A874 (PDF1220), III-220 SetPort A873 (PDF1223).
Printed footers checked. SRC-0068 QD-36/PDF290 image agrees on Get/Set and warns
against selecting an unopened port; chapter footer3/2/83, published1985, not proof
of January1984 behavior. Prose I-162 initial NIL, I-163 reverse globals and II-20
A5 first-parameter indirection agree with prior accepted lead checks. Pascal
pointer/VAR-pointer size4 and trap stack rules reuse SPEC-0029 evidence.

The proposed108-byte extent is an opaque engineering limit; no field-offset
table, padding rule, private QuickDraw bytes or region parser is adopted here.
The206 reservation does not reconstruct its76 nonpublic bytes. Full InitGraf,
OpenPort/InitPort, fonts/cursor/pattern assets and manager-global installation defer.

Section eligibility note: text extraction on I-163 displayed two short manual
call-site examples (four instruction lines). No function bodies/Apple routine
implementation was encountered. They were excluded from all worker, contract and
implementation inputs; no replacement code generated. Eligible prose/interface
facts only. The standalone equate/sample-code bans remain; the published I-209
numeric constant is an admitted interface fact. Raw excerpts/renders remain private.

## Dispositions

| ID | Disposition | Accepted boundary |
| --- | --- | --- |
| D-01 | adopt | Separate selected-port lane; old fixed-port/default lanes unchanged. |
| D-02 | adopt | Only canonical GetPort/SetPort; drawing refuses until an actual guest-record adapter exists. |
| D-03 | adopt with changes | Trap words independently image-verified; P4 follows the reviewed pointer/Pascal convention, no result. |
| D-04 | adopt with changes | 108-byte opaque engineering extent only. Reject unsupported104 choice;106/layout/padding remain open. |
| D-05 | adopt | At most2 unique nonzero even strict24 port IDs with full live disjoint records. |
| D-06 | adopt with changes | Disjoint native view/backing per binding; exact sharing deferred. SPEC-0041 unchanged. |
| D-07 | adopt | 206-byte [g-202,g+4) reservation placement is engineering, checked in declared below-A5 storage. |
| D-08 | adopt with changes | Prepared initial current is always0. This does not claim InitGraf/global initialization. |
| D-09 | adopt | NIL SetPort is a defined engineering refusal; not a historical claim. |
| D-10 | adopt | GetPort output4 must be even/disjoint/below-A5; general stack/heap destinations deferred. |
| D-11 | adopt | Re-read current cell; honor a direct guest store of a registered ID; never cache/fallback. |
| D-12 | adopt | Guest writes to A5 header remain refused; g is immutable during the root. |
| D-13 | adopt | Required future exact-g/generation header exception; no dirty-flag/integrity bypass. |
| D-14 | adopt with changes | Dedicated owner environment span/commit plan; not a pure SPEC-0029 handler amendment by implication. |
| D-15 | adopt | All listed logical and translated backing aliases refuse, even read/read overlaps. |
| D-16 | adopt | Malformed guest ID/output/frame is a pre-effect service refusal; inactive prepare errors stay inactive. |
| D-17 | adopt | Trusted config/map/identity change quarantines owner; prior effects/stopped frame remain. |
| D-18 | adopt | Preserve SR and all registers except normal PC/A7 advancement as an engineering superset. |
| D-19 | adopt | Directory entries stay unsupported to the generic dispatcher; explicit coordinator only, no fake direct veneers. |
| D-20 | adopt | Retirement scrubs cell and revokes capability; fault snapshot alone is not permission to resume. |
| D-21 | adopt | One native selection semantic core for Mac/PC; interpreter is guest boundary only. |
| D-22 | defer | Original selection-only workload after native core and owner header/environment/lifecycle prerequisites; none generated now. |

## Integration and execution status
Claude's substantial three-document draft is retained as the base, with the above
source, extent, disjointness, initial-state and commit clarifications.226 lines total;
spec155 exceeded its150-line file limit by5 (total230 budget respected). It is a
proposal, not executed verification. Codex alone integrates. Grok's separate narrow
A5/environment-write review timed out at180.11s without a finished report. No
finding adopted, no retry and no claim of independent review completion. The lead
checked all implicit output/backing spans, staged input and total commit locally.
No provider repeated prior qualification. No HQ/manuals/research originals/UART
bodies/credentials/prohibited implementation transferred. Captures outside Git.

Only documentation/index/preservation checks execute in this slice. C01-C52 are
planned cases, not passing tests; old CI2991/11-profile results remain dated prior
SPEC-0041 implementation evidence. No new CI/compiler/native/boot claim.

## Remaining questions and next step
January1984 record extent/padding, g placement in arbitrary applications, full
public/global initialization, stack/heap VAR outputs, NIL/unknown-port historical
behavior, registered record-to-view drawing semantics, regions/callbacks and patch/
direct-call equality remain open. No QK/QQ/FA/AL historical question is closed here.
First propose the bounded pure native selection core and independent tests for both
CPUs. It requires finalized value/lifetime/alias API, not a guest header exception.
Then specify/verify exact-g owner integrity, explicit environment spans/total commit
and exclusive lifecycle coverage together before any guest binding. Full drawing
through the selected port follows its own reviewed adapter. Existing source/role/
edition/physical/full128K/B2/human/System1/ROM gates and wiki retirement persist.
