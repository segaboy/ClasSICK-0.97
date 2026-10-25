# Bounded C99/C16 bank geometry and four-byte plan prototype

Windows11;public baseline462e3a0. Codex sole writer/integrator/verifier. One
Grok4.7 low bounded worker. Read this packet only,no commands,other reads,
writes,web,delegation or tests. No manuals,HQ,untracked originals,Apple/MS
implementation/media,emulator source or manual samples. Use original code.

The paired Mac/x64 release is primary;Codex handles its live input tests.
Deliver useful original files:probes/pc286/bank-plan.h/.c,
tests/pc286-bank-plan.c,and docs/development/pc286-bank-plan-contract.md.
This is an UNBOUND logical metadata/proposed write-plan prototype,not far
access,backing admission,commit execution or a production ABI. guest.c and
all existing runtime paths stay unchanged. No target execution or link helper.

Header API,exact names:
cs_bm_bank:{uint32_t id,bytes};
cs_bm_region:{uint32_t base,bytes,flags;uint16_t first_bank,bank_count};
cs_bm_map:{const cs_bm_region *regions;const cs_bm_bank *banks;
 uint16_t region_count,bank_count};
cs_bm_piece:{uint32_t bank,offset,length;uint8_t data[4]};
cs_bm_plan:{uint32_t count;cs_bm_piece pieces[4]};
enum cs_bm_result:CS_BM_OK=0,ARGUMENT=1,MAP=2,ADDRESS=3,PERMISSION=4
(use CS_BM_ prefix for every enumerator).
cs_bm_validate(const cs_bm_map*);
cs_bm_check(const cs_bm_map*,uint32_t at,uint32_t bytes,uint32_t access);
cs_bm_plan4(const cs_bm_map*,uint32_t at,uint32_t value,cs_bm_plan *out);
All return cs_bm_result. Include stdint only or the existing owned c16.h.

Logical geometry follows SPEC-0069:1..8 regions,1..64 banks;nonnull typed
arrays;region base/bytes even,bytes>0,24bit range,READ bit1 mandatory,only
flags1/2/4. Regions do not overlap. Every bank id is unique1..64,size1..65535.
Each region references one nonempty in-range bank-array slice;no slice
overlap,all banks covered exactly once;sum sizes exactly region bytes.
Validate entire geometry even when an operation touches an early bank.
Physical ownership/aliasing/backing existence/commit are NOT proven by this.

validate:null map ARGUMENT;invalid arrays/counts/geometry MAP.
check:access0 or outside7 ARGUMENT first;then validate;then zero bytes,
at>=2**24,span past2**24 or no ONE covering region ADDRESS;then missing
permission PERMISSION. Adjacent regions never merge.
plan4:null out ARGUMENT first;then validate;odd at or invalid4byte span
ADDRESS;then WRITE permission. A long may cross banks within ONE region.
Generate ordered bank-id/offset/length pieces,1..4,each contiguous within
one bank with length<=4. Encode value most-significant byte first;zero all
unused plan fields/data. On ANY refusal leave out byte-for-byte unchanged.
Map/descriptors/out are truthful,live,aligned,immutable/disjoint during call;
no hostile-pointer or forged plan defense. Plan expires with any map change;
it is NOT a capability,commit token or generation/lifetime implementation.

C16:include ../../arch/i286/c16.h for owned assertions,assert plan<=80 bytes.
Use full32bit masks e.g.0xFFFFFFF8UL,not ~7u. No division/modulo,variable32bit
shift,unbounded loop,aggregate assignment,memcpy/memset,allocation,IO,MMIO,
far casts or compiler-helper bodies. Extract four data bytes with constant
24/16/8/0 shifts. Use bounded unsigned indices after checking counts/slices.
No per-byte callback policy imposed on native production. No model choice.
Stage a local complete plan,then explicit field/byte copies on success only.
Do not claim staging this metadata makes a fallible native bank commit atomic.

Tests:original standalone C99 main with concise literal expected checks for
cross-bank big-endian plan,whole-map late invalidity,no partial output,
high flag bits,24bit boundary,odd address,adjacent region refusal,permissions,
duplicate bank IDs,slice overlap/hole,over/under bank sum,null args and normal
contiguous plan. Worker tests are proposals,not passing. Contract<=700words,
combined header/source/tests<=360lines. Return JSON ONLY
{files:[{path,content}],handoff:{result,tests_executed:[],blockers:[],decisions_needed:[]}}.
No CMake edits/stable IDs. Codex will freeze independent checks,integrate,
run hosted/sanitizer tests and compiler-only pinned WCC helper audits.
