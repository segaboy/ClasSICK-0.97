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
