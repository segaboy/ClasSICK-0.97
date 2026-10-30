<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# PC286 follow-up packet intake

2026-10-29;SRC-0540. Baseline public5802dfec7c3afdb4b9113d3d2a9537c1056860f9.
Disposition:research/intake only. New implementation is NOT adopted. No new
SPEC,IMPL,TEST or build acceptance is assigned. No worker dispatch,compilation,
test execution,link,target run or production change occurred for this intake.
The previous [SPEC-0082 result](pc286-span-build-evidence.json) retains its exact
source/build/CI scope and does not validate these additions.

## Identity and review scope

All29declared checksum rows pass,including repeated declarations;this is not a
count of unique files. Eleven files are byte-identical to the prior submission;
six new source/header/test files and seven changed wrapper/script/build/prose
files are present. Byte-identical intake/member copies and the complete identity
manifest remain outside Git. Archive paths,case-insensitive uniqueness,symlink
and encryption checks pass. Private material is retained separately,not published
or forwarded. Only public-candidate identities are indexed below.

The lead reviews changed README/contract/receipt prose,the two new API headers
and the CMake insertion. New implementation/test bodies and compiler-script body
remain uninspected and unexecuted. Hashing bytes is not semantic code review.
Author/direct-cloud claims remain proposals,not independent project verification.

| Proposed file | Bytes | SHA-256 |
|---|---:|---|
| probes/pc286/span-access.c | 7034 | 07dd03d281db9014028eb61c81c94a3e9b52911de064bc1c3eaeb8b80be5a010 |
| probes/pc286/span-access.h | 1800 | 1144def4e4c43e86019ddb5107b6d1b7b18b08afb33bd2225c1a9bbe1b2d4787 |
| probes/pc286/span-cap.c | 4192 | 0e63c2f4e2c34694c01017371a5ae543f7d5d959345a221e22b448bb81980172 |
| probes/pc286/span-cap.h | 2412 | d41a02992c5ff3b46f3f4e8b98c43db34d7a0708c27a4ac8ff2bad39fa4071fc |
| tests/pc286-span-access.c | 9494 | 73ef302340ee74f3907e6c4f781b46427868248837bba2204561fa6ae49a1502 |
| tests/pc286-span-cap.c | 9180 | 35a23c159526e7f2a56a21e4f4f1a9fec5c96d3658bfa15d71b62ad7ea4796eb |

## Useful proposals and disposition

| Proposal | Disposition | Reason / next requirement |
|---|---|---|
| Checked BE read/write widths1/2/4 (span-access) | Defer implementation adoption;review first | Extends the existing SPEC-0069 numeric model toward bound-memory access;freeze environment/error precedence and caller obligations before source/test review |
| C lifetime/generation capability (span-cap) | Defer implementation adoption;review separately | Relevant to SPEC-0069 section5;numeric lifetime/cursor conformance does not yet establish a native borrow protocol |
| Replace original span-build files | Reject replacement | They are byte-identical to the PRE-INTEGRATION candidate and omit the adopted0:0 refusal/stronger obligations;retain SPEC-0082 and its independent oracle |
| Insert the supplied CMake block verbatim | Reject insertion | pc286.span-build and its target already exist;adding this duplicates names. Merge only separately reviewed new targets and retain the current oracle |
| Expand --span-build to all three slices | Defer / require separate profile design | The proposed contract couples the closure to both new components,despite calling the slices independently adoptable;retain the existing qualified flag/closure |
| Reuse reported cloud tests as project passes | Reject that interpretation | The new suites,200000 differential operations,20000 drains,128KiB fill and25defect controls are self-reported;no new result was executed here |

## Contract issues to settle

1. **Native pointer borrowing and rights.** The proposed cs_bx_locate accepts an
   environment,bank and offset,and returns a mutable native pointer/available
   count. It takes no capability/access argument. The proposed capability stores
   its own owner,lifetime,generation and rights. A bridge must bind these to the
   SAME admitted map/backing lifetime/epoch,apply stored rights,and cap each run
   to both the descriptor remainder and native span. A raw privileged locator is
   not automatically a permission-enforcing capability API. This is an interface
   requirement/gap,not a demonstrated defect in the unreviewed function bodies.
2. **Borrow scope and metadata.** State when returned pointers expire,who keeps
   owner/registry/map/environment/spans alive,and how exclusive stability lasts
   from validation through the final load/store. Do not retain a pointer across
   remap,owner advance or callout. Carry truthful extents,nonaliasing,disjoint
   inputs/outputs and C16 metadata-window obligations forward from SPEC-0082.
3. **Error order.** Freeze the deliberate insertion of BACKING/STALE ahead of
   WIDTH/alignment/window checks;do not silently call this identical to guest.c.
   Specify the rights/release/EOF helper refusal order and unchanged outputs,
   registry,cursor and backing effects. Existing SPEC-0069 remains authoritative.
4. **Independent expectations.** First review access using literal boundary,
   late-invalid-binding,alignment,permission,stale and preservation cases against
   the accepted contracts. Then review capability preflight,lifetime invalidation,
   advance-before-remap,release,exhaustion and descriptor/native-split coupling.
   Large author-generated random counts are neither unique feature counts nor
   evidence of an independent reviewer. No fixtures/cases are generated here.
5. **C16 qualification.** New pointer additions,frame costs,helpers and closure
   need their own pinned compiler checks. Existing span-build listings cannot
   qualify changed code. Link,ABI/alignment,stack high-water,physical ownership,
   target execution and measured performance remain open.

## Stale and overstated wrapper claims

The packet is based on8a197ff,preceding the integratedd78ad5e/5802dfe slice.
Source citation and four-object/two-refusal qualification are already recorded
for that older scoped experiment;the NEW expanded closure remains unqualified.
Keep the strict sub-1MiB envelope distinct from the published addressing relation;
retain the95reachable-chunk bound under this profile and192general capacity.

The author says every slice is below450artifact lines,but the supplied access
header/implementation/test declarations total37+229+204=470. This was an
owner-direct contribution,so supervised worker limits do not retroactively
qualify or disqualify it;the wrapper's blanket size claim is still inaccurate.

The previous narrow source/role disposition is not enlarged by instructions in
this attachment. CLEANROM/affectedMac/ROM/CPU exclusions persist. No private
context or implementation is sent to another worker. All research originals,
held brief,UART role separation,tags,old records and registered media persist.

Next:review the checked-access contract and implementation as one bounded slice,
then the capability layer and its native bridge separately. These are286 model
advances;they do not solve the paired-release Mac/PC GUI input blockers or supply
a linked/bootable286 system. This intake grants no new run or publication scope.

## Revision2 review -2026-10-29 (SRC-0541)

Baseline b68bef1a066d3cc0dd1a1fab031614c6351c1137. The new owner-supplied r2
packet is45455bytes,SHA256
e58db02042c9c9d4086817b358aec07173de2fbd6d39f396aa50cad44a5ddc7f.
All19declared checksum rows pass;archive/member copies and identity manifest
remain outside Git. Paths,case-insensitive uniqueness,symlink and encryption
checks pass. The new packet has no private-HQ subtree;the previous private
submissions and their dispositions are retained,not deleted or overwritten.

This is a STATIC CONTRIBUTION REVIEW,not integration or executed qualification.
Unlike the earlier packet's interface-only review,the lead now reads r2's
span-bind,span-access and span-cap C bodies/headers,contracts,response and CMake
deltas,and reviews the compiler-script diff. Submitted test bodies and the full
compiler script are not reviewed or executed. No candidate tests/builds,worker dispatch,
target runs or implementation changes occur. No new feature SPEC/IMPL/TEST/build
acceptance or affected-lane source/role permission is assigned. Statements below
about control flow are static observations,not passing or failing test results.

### Earlier findings addressed in r2

| Earlier issue | Static disposition |
|---|---|
| Stale builder overwrite | Addressed:the adopted builder/header/oracle are omitted from the packet;no replacement requested |
| Duplicate CMake target/test names | Addressed:new-only access3/cap2 targets;existing builder and independent oracle retained |
| Coupled compiler profile | Addressed in the proposed diff:separate --span-access/--span-cap and exclusivity checks;old --span-build control flow retained;not compiler-tested |
| Unscoped public locator | Addressed for the checked-access API:read/write return values only;capability adds a rights-checked borrow;shared backend helper remains privileged |
| Missing native-span cap | Addressed in borrow source:min(remaining,limit,bank room,native span available) |
| Borrow expiry and error order | Substantially addressed in separate contracts,including deliberate BACKING/STALE insertions and valid-binding-only guest.c parity |
| Misstated test independence/size | Corrected:literal oracle is author-written;all large counts remain self-reported;whole slices exceed worker output cap |

### R2-1 - Registry initialization needs a fresh-context rule

SliceB span-cap.c:5-14 resets issued/current to0 unconditionally. Owner-open
then issues lifetime1 and generation0 again. The contract states lifetimes never
recycle,but does not describe registry_init or forbid reinitializing a live
registry/owner. Static trace:initialize,open owner,open cap;initialize the same
registry again;open into the same owner. With the same epoch,the old cap's
lifetime/generation can match again. This is an unexecuted reasoning trace.

Before adopting B,explicitly restrict initialization to a fresh,unreferenced
context lifetime and prohibit registry/owner address reuse while old caps can
be used,OR provide a non-recycling reinitialization contract. Keep owner/registry
alive throughout capability use,not merely throughout one returned run. Under
an explicit fresh-only precondition,this is caller misuse rather than a runtime
revival defect;the present wording does not establish that precondition.

### R2-2 - A numeric epoch needs an identity domain

SliceB open stores a caller-supplied binding_epoch (span-cap.c:56-107). Borrow
compares map pointer and both numeric epochs (187-190),but records no binding
identity. Two distinct valid environments can share that map pointer and epoch
number while naming different backing spans. The checks alone cannot establish
that this is the same binding 'the capability was opened on'.

Choose one explicit contract before adoption:one authoritative active binding/
epoch domain per owner/map lifetime,with advance-before-any-backing-change and
no epoch reuse,OR an owner/provider identity captured at open and checked at
borrow. Test the cross-binding same-map/same-number case under that chosen
contract. This is an unresolved identity precondition,not proof of an observed
unauthorized access or a security boundary. Do not invent physical ownership
from pointer/number equality.

### R2-3 - Shared helper preconditions need to be explicit

span-bind.h exposes cs_sb_walk/find as backend primitives. Their bodies assume
a validated map,truthful span/count extents,64-byte scratch arrays,valid bank
index and the successful walk's matching scratch state. The inspected scalar
and borrow callers supply validation first;no defect in those paths is found.
Document those helper-only preconditions and stable/disjoint metadata duties;
do not describe the helpers themselves as safe arbitrary-input entry points.

### Disposition and next work

SliceA:ready for a bounded qualification pass after clarifying helper
preconditions;no scalar-path correctness defect identified in static review.
That is not test acceptance or source certification. Verify its literal cases,
independent expectations,host profiles and new pinned C16 closure before adopting.
The proposed46/200000/300000 counts and12controls remain author reports.

SliceB:defer adoption pending R2-1/R2-2 decisions and executed qualification.
The new borrow implements the proposed rights and run-length checks,but its
identity/lifetime context must be unambiguous. The proposed8000-window parity,
20000drains,byte totals and15controls remain author reports. No production
guest.c,ROM,CPU,device or existing286 probe is changed.

Next:qualify A first;then B after those two context rules are resolved. Preserve
the existing builder/oracle and old compiler modes. Link/ABI/alignment,stack
high-water,physical ownership,target execution and measured performance remain
open. The paired-release input blockers are unchanged. No other AI service was
used for this review;remaining provider balances are unreported.

## Revision3 review -2026-10-30 (SRC-0556)

Baseline e11711836ec4ea875e88a8f795d4d6a7f822ce65. Owner supplies r3 as
additional input,after the separate retained-phase diagnostic checkpoint.
Archive:56445bytes,SHA256
a50814bbf5e1be7d267b8347263ab13cd1628a0d70594bd82ffc2e1ee14ccd69.
All25declared checksum rows verify against the26files (the checksum file is
not self-listed). Safe paths,case-insensitive uniqueness,no symlinks/encryption
and byte-identical external copies are verified. The nine unchanged members
retain their r2 identities;they are not independently requalified by this review.
The earlier source/role disposition persists;source certification is not inferred
from the author's provenance statement or checksum equality.

This is STATIC PROPOSAL REVIEW. The lead reviews the changed286 implementation,
header and contract diffs;spot-checks the changed tests' registry/rebind cases;
reviews the compiler-script delta,control/matrix scripts and submitted logs.
The full compiler script matches its supplied diff against the baseline and
parses as Python;it is not executed. No submitted test,mutant,compiler,build or
target case is executed here. The response's held-lane section,root README and
Mac brief remain unread/excluded. The PC-only report is reviewed as a proposal
from public project observations;its API assumptions require eligible evidence.
No contribution source is integrated,no worker is dispatched,and no new
SPEC/IMPL/TEST/build acceptance or trial is assigned by this intake.

### Earlier findings and remaining clarifications

| Finding | R3 static disposition |
|---|---|
| R2-1 fresh registry | Addressed subject to caller lifetime rules:used counters cause STATE without mutation;caps now retain registry identity,including equal-number lifetimes in different registries |
| R2-2 binding identity | Addressed in mechanism:owner captures one binding;borrow accepts no substitute;validated same-map rebind advances generation before replacing it |
| R2-3 helper preconditions | Substantially documented:validated map,truthful bounded spans,matching64-byte index arrays and stable/disjoint metadata;one pointer wording correction remains |
| Compiler closure | Proposed --span-cap now includes owner_rebind;old builder input remains unchanged;new pinned C16 qualification still required |
| Submitted test evidence | Proposed same-map/same-epoch and equal-number/different-registry cases target the findings;author-written expectations/logs are not independent project results |

**R3-1 pointer wording.** span-bind.h says cs_sb_env accepts "any env pointer
value". It checks NULL then dereferences non-NULL. Require NULL or a valid
readable environment object,plus truthful reachable array extents. This does
not validate arbitrary native pointers or establish a security boundary. The
shared helper and scalar bodies are otherwise unchanged from r2.

**R3-2 advance versus rebind.** The contract permits backing changes after
owner_advance,but separately prohibits binding edits in place and requires
owner_rebind. Freeze one rule before qualification:advance invalidates current
caps;all binding metadata/placement changes use a fresh stable binding and
rebind. Ordinary data writes through a permitted writable run are separate.
No registry/owner/binding/map/span storage may be recycled while a referencing
capability can still be used. Counter checks do not establish physical ownership
or protect against forged context fields. These are contract clarifications,
not observed runtime failures.

**R3-3 author verification controls.** The supplied logs report42suite runs
across six Linux profiles and19detected mutants. None is credited as a project
pass. The mutation driver counts compilation failure as detection,prints SITE?
or MISSED without a failing final status,and does not itself require a clean
baseline. Before reuse,require a passing baseline,unique replacement sites,
successful mutant compilation and the intended failing assertion,with nonzero
exit on a missed/invalid control. Its log reports runtime FAIL rather than
build-fail for these19cases,but this does not independently verify that report.
Pinned WCC,Windows profiles,link,ABI/alignment,stack high-water,physical ownership,
target execution and measured performance remain open. No supplied shell or
mutation script is run on the shared checkout.

Disposition:A remains ready for bounded qualification after R3-1 wording;
B's earlier identity objections have concrete proposed solutions,but adoption
waits for R3-2 wording and executed qualification. Keep the author originals
unchanged. Start with A,then B;preserve the existing builder/oracle and old
compiler modes. This advances the review queue,not a bootable286 or release claim.

### PC host-delivery proposal

7575bytes,SHA256
b64c5b8eeb6e465eb083d77e147d3e6d4b5bf4f6a4d12d34f39ea5473f1872c3.
The report cites own-project observations and marks external API assumptions
for verification;no emulator source is used. Its causal conclusions are narrowed:
E2 proves the tested guest sequence,not every GUI-driven guest path. One guarded
cursor2,2 refusal does not prove that all captured injection is impossible.
Nominal PS/2 configuration does not exclude host absolute-to-relative conversion.
Older physical checks on different candidates do not settle today's injection
path. Ballistics,warp suppression and forwarding remain hypotheses.

Useful next measurements,after a reviewed contract and eligible published API
facts:read-only capture/DPI/physical-versus-logical cursor/clip context (D0),then
a bounded multi-packet guest history under separately labeled controlled and
physical stimuli (D2). First-packet-only halt cannot supply that history;new
diagnostic code would need its own review/tests. The current helper uses
GetCursorPos;the proposed physical-cursor/clip/DPI comparison is a new measurement,
not an unchanged retry. Do not weaken owned-client containment,change global
pointer settings or infer guest acceptance from capture metadata. No diagnostic
or desktop trial is implemented/executed by this intake. Mac/PC motion and
drag/release qualification remain open;the Mac brief is not a permitted input.

## Revision4 review -2026-10-30 (SRC-0558)

The owner supplies r4 as further input,not authority to adopt its instructions.
Archive115012bytes,SHA256
0a9d5881a443911bb1ff76ce4d0cc5ae89e29ad515acaf1b09d007c135b67d24.
All46declared checksums verify against47files;ten members match r3 exactly.
Safe paths,case-insensitive uniqueness,no symlinks/encryption and byte-identical
outside-Git copies are verified. The bundled Python bytecode cache is retained
privately by identity only;it is neither inspected,executed nor admitted to Git.
All earlier originals/dispositions persist;none is deleted as 'superseded'.

The author's disclosed exposure is unchanged (SRC-0534);the author states no
PC,8042,BIOS or hypervisor source was used. This intake reviews PC-only proposal
documents/patch identities and286 interfaces;it does not certify that claim or
admit an affected Mac/ROM/68000 lane. No HQ/manual/held material is forwarded,
no worker is called,and no new implementation,SPEC/IMPL/TEST/build or target
acceptance is assigned. No submitted suite,mutant or self-test is executed.

### R3 findings

| Finding | R4 disposition |
|---|---|
| R3-1 arbitrary-pointer wording | Addressed:NULL or valid readable environment and truthful reachable arrays;no security-boundary claim. Comment-only delta |
| R3-2 advance/rebind rule | Addressed in header/contract:advance invalidates only;metadata/placement changes require fresh stable binding and rebind;ordinary permitted data writes remain separate |
| R3-3 meaningful mutation controls | Substantially addressed in proposed driver:passing baseline,unique site,compile failure invalid,expected assertion,nonzero on missed controls. Windows byte restoration and precise failure association still need qualification |

The shared access/binding and capability C bodies remain byte-identical to r3.
Changed B tests add unbuffered output and distinguishing assertion comments;
this is reviewed as a diff,not credited as execution. A can proceed to bounded
qualification,then B. This resolves the wording prerequisites;it does not
constitute adoption or close pinned C16/link/ABI/stack/physical gates.

The author now reports that r3's 'skip revalidation' mutant was invalid because
it also removed initialization of a helper index. Record this correction to
the author report;the earlier19/19 was never project verification. The new
driver is statically reviewed,not run. It hashes decoded text and restores via
text writes,so universal-newline translation can mask changed source bytes on
Windows. Require original-byte preservation/hash checking. Also require the
expected assertion to come from the SAME failing build/run;the current driver
combines run statuses and output matching separately and keys output by test
path. Include multi-build and LF/CRLF cases before claiming these guarantees.
Its Linux /tmp executable convention needs Windows adaptation in a scratch tree.

### New286 slice C:placement/backing store

Header/contract review only;new span-store.c and its test body are not semantically
reviewed or executed. The proposal uses caller-declared holes,two stable binding
slots,a persistent fresh registry and rebind-before-copy. These fit the intended
review sequence but still require A/B qualification first. Before a C contract:
freeze exclusion across rebind/copy/active-slot/epoch commit,including reentry and
interrupt access;state which inactive scratch fields may change on refusal;
prove old/new physical disjointness,truthful extents and infallible copy duties.
Rebind makes old caps stale;it does not make partially copied data safe for a
new observer. Retire/reopen and slot reuse need independent stale-capability
cases. Budget both slots,embedded chunks,scratch and temporary double placement
with the pinned target ABI;no host-size estimate is a640KiB/128KiB fit proof.
No memory discovery,production provider or286 boot is supplied by this model.

The proposed compiler script's full copy matches its diff against current public
inputs and parses;it is not run. New exclusive --span-store closure and its
claimed30public symbols require actual pinned WCC checks. Existing compiler
modes/builder/oracle stay unchanged. Matrix/control counts remain author reports.

### New PC sequence diagnostic

Review covers the draft contract/trial plan and patch packaging,not new recorder,
reader or test implementation bodies. The five replacement paths are unchanged
since the claimed0511263baseline;full copies match the supplied patch,which passes
read-only application/whitespace checks against efcf33f. Nothing is applied.

The proposed CSPSEQ01 recorder is a useful D2 candidate:up to48auxiliary bytes,
per-byte error flags,gap/count metadata,and a3-byte variant. Keep it a new opt-in
diagnostic with existing modes/default output unchanged. Its decoder-replay
model needs frozen independent byte/error/gap/saturation oracles and actual
native image/import/seal/stack/omission checks. Author74/580/11assertion and
48suite/19+12+12mutant claims remain unverified;no boot occurs here.

Correct these trial-plan inferences before adoption:48bytes are not necessarily
16valid reports;unsynchronized/error bytes and gaps matter. Failure to reach
3bytes does not prove zero delivery or locate failure upstream. Separate cold
starts do not establish the other run's exact report count. Large steps alone
do not prove warp/recentring;small packets alone do not prove GUI usability.
Report bytes,gaps,READY/fault/stop state and duration separately,then hypotheses.
Read-only D0 needs eligible published API facts;observe current button mapping
instead of hard-coding the Windows primary button. Keep capture/owned-client
guards,windowed readiness,instructions,stimulus classes and bounded cleanup.

Priority:finish the separate Mac witness build/trial dependency,then a bounded
PC sequence CONTRACT/implementation review to enable a new measurement. Qualify
A-then-B in the separate286 lane;defer C adoption until its lifecycle/exclusion
contract and prerequisites pass. This packet closes no paired-release blocker.

## Host checked-access qualification - 2026-10-30

SRC-0564 / TEST-0113; baseline 40909e06b84992a077aac2547a7ffe21e3027a8a. In isolated scratch, the r4 checked-access and binding proposals and author tests were compiled with GCC 16.2.0 and Clang 23.1.1, C99, O2 and strict warnings. Each compiler passed the author's 46 literal cases, 200,000 differential operations against its flat reference (26,696 successful), and 300,000 operations against existing guest.c (39,961 successful). The literal oracle and differential generator are author-written; comparison with existing guest.c gives a separate parity check, not a completely independent test design. Exact input hashes and limitations are in TEST-0113. The scratch setup initially missed an existing header; after adding the unchanged header, all six tests passed. Seven proposal inputs match the preserved originals byte-for-byte after execution. No other AI was used.

This is host evidence only. The proposal remains unintegrated: pinned C16 compiler/ABI, independent fault sensitivity, target link/execution, physical ownership and lifecycle guarantees remain open. It does not establish performance, 286 boot, Mac or PC mouse movement, dragging, or release readiness. Continue A qualification before considering B; retain all original intake material.

## Slice A adoption - 2026-10-30

SRC-0566 / SPEC-0086 / IMPL-0096 / TEST-0115 supersede this intake's earlier slice-A deferral only. The original checked-access model is integrated with a 16-bit far-data guard and targeted qualification; the [canonical receipt](pc286-span-access-evidence.md) owns actual tests and limits. Slice B and placement/store remain proposals. No production 286 boot path or paired release criterion is closed.
