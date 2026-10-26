# PC286 checked four-byte write experiment

2026-10-25;SPEC-0069/SRC-0466..0467/IMPL-0079/TEST-0092.
Implemented as an original HOST-MEMORY experiment under the existing286
preparation scope. It is not a native far-memory provider,production binding,
guest ABI,boot,physical alias/lifetime proof or a performance claim. Current
68000/x64 flat paths and all guest/runtime files are unchanged.

## Contract
`probes/pc286/store4-model.h/.c` add cs_bs_store4 and cs_bs_epoch_next.
The checked store takes the existing logical map/numeric catalog,RAM envelope,
borrowed spans,current/expected epoch,address and32-bit value. A span names a
bank,offset,positive length<=32767 and a truthful live native byte-object range.
Arrays and native ranges are caller-owned,immutable/stable during the call,
with truthful capacities and no alias to metadata or one another. No callback,
concurrent remap or reentry is allowed. Numeric checks do not establish these
native obligations. No forged pointer is dereferenced as a validation test.

Refusal order:NULL arguments,zero/excessive span count or zero epochs give
ARGUMENT;then complete map/catalog numeric validation gives ARGUMENT/MAP/BACKING;
then epoch mismatch gives STALE;then existing plan4 alignment/range/permission
checks give ADDRESS/PERMISSION;then the entire span binding gives BINDING.
Every refusal leaves memory unchanged. New result numbers are model-only.

There are1..192 spans,ordered by map bank-array order (not numeric ID order).
Each bank has1..3 contiguous spans,starting at0,covering exactly its declared
bytes. Non-NULL data,positive lengths and the32767 ceiling are mandatory. All
spans,including later unrelated ones,are checked before native data access.
Bank IDs remain1..64,bank bytes<=65535. A65535 bank requires three maximum
chunks32767+32767+1;larger regions use multiple banks. Adjacent regions never
merge. Zero address is valid within an admitted range;long addresses are even.

The implementation resolves all four native destinations and stages their
big-endian bytes BEFORE the first store. Its final four direct byte stores
perform no lookup,callback,IO,allocation or refusal under the caller obligations.
Each pointer addition is inside one admitted object;the checked offset is
narrowed to16bits. There is no whole-bank pointer,far cast,uintptr comparison,
copy helper,division,modulo,variable shift or64-bit arithmetic in the model.
The local index arrays cover at most64 banks;no registry/global allocation.

`cs_bs_epoch_next` only preflights a new numeric epoch:NULL out gives ARGUMENT;
UINT32_MAX gives EXHAUSTED without output change;otherwise it writes current+1.
It does not remap memory or confer ownership. The owner must exclude callbacks,
advance BEFORE binding changes and never recycle epochs;0->1 initializes a new
owner lifetime only. Guest content/handle moves within fixed RAM are unrelated.
No pointer/capability survives the operation.

## Actual verification
Author suite and independent oracle pass in Clang Debug,GCC x64 Release,Clang
ASan/UBSan and GCC i686 Debug. The oracle executes69 assertions:30 literal cases
frozen before worker output (60 assertions),plus nine post-review assertions for
an unrelated late NULL span,epoch behavior and the full192-span limit. These
overlap author requirements,not69 extra unique features. Source expectations are
[retained unchanged](../../tests/data/pc286-store4-oracle.json);the runner was
written after API/implementation review and is not claimed wholly author-blind.

Coverage includes BE writes across banks/chunks,the32767 boundary,maximum65535
bank plus a neighboring bank,nonzero bases,last valid24-bit write,adjacent-region
refusal,odd/outside addresses,permissions,whole-map/catalog failures,late invalid
bindings,error precedence,epoch mismatch/exhaustion and unchanged bytes.
Author preservation originally used a32-bit hash;lead replaces it with bytewise
buffer snapshots/memcmp. Tests with invalid native extents/aliases remain outside
the contract. No provider obligation becomes true because a mock passed.

Three compiled faulty draft controls are detected:write only three bytes,ignore
late NULL spans,and ignore stale epoch. Final integration adds only the checked
16-bit offset narrowing and passes all four host profiles. Six focused CTests
(two store suites plus the four helper suites) pass per profile.

`scripts/Probe-PC286BankPlan.py --store4-model` verifies pinned tool/header
identities,then compiles a closure with the unchanged bank-plan/catalog. Six
objects (ms/mm/ml,od/os) report80286/USE16,expected publics and zero unresolved
imports. They are NOT linked or executed. Near/far representation,actual span
construction,physical ownership/nonaliasing,link/ABI and measured costs remain
open. The existing --catalog and default modes retain their implementation;
using --catalog with --store4-model refuses. No production stack-check waiver.

One fresh Copilot Sonnet5.5 Low primary returns24 header/151 implementation/146
author-test lines in109.70s from17161B original-only input. One permitted view,
zero worker writes,commands or tests;297 code/test lines meet500 cap. Codex
integrates sound artifacts,adds literal-oracle runner/bytewise preservation,
CMake,C16 closure controls and verifies. No second worker or provider retry.
One reported premium request;all-four remaining balances unavailable,no quota
equivalence or measured Codex saving. Verbose captures remain outside Git.
[Actual machine receipt](input-review-and-store4.json).

Next286 task:qualify a small native span construction/conversion experiment for
the pinned compiler under explicit owned-memory assumptions. This model does
not clear that far-provider gate. Main release input investigation remains
separate;no freeze,release,hardware,reference or native-application permission.
