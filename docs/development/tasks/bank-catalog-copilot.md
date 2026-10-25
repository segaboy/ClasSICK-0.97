# Bounded 286 backing-catalog preflight prototype

Windows11,baseline38e938b. Codex is sole writer/integrator/verifier. Read this
frozen packet once only;no commands,other reads,writes,web,MCP,delegation or
tests. No manuals,HQ,untracked research,Apple/MS implementation,emulator source
or sample code. Return original proposed artifacts only,not execution claims.

Deliver probes/pc286/bank-catalog.h/.c,tests/pc286-bank-catalog.c,and a concise
docs/development/pc286-bank-catalog-contract.md. Combined C/header/test<=340
lines,contract<=550 words. Use existing bank-plan.h/.c interfaces,do not edit
them. Output JSON {files:[{path,content}],handoff:{result,tests_executed:[],
blockers:[],decisions_needed:[]}} only.

This is an unbound NUMERIC backing-catalog preflight under SPEC-0069. No native
pointer,physical memory read/write,far conversion,backing allocation,commit,
BIOS,boot or production ABI. The below1MiB envelope and caller-supplied
canonical-base/native-offset relationship are explicit engineering inputs,
not a discovered RAM map or a proof of hardware/ownership/alias freedom.

API exact:
cs_bc_bank {uint32_t id,physical_base,native_offset;};
enum cs_bc_result {CS_BC_OK=0,CS_BC_ARGUMENT=1,CS_BC_MAP=2,CS_BC_BACKING=3};
cs_bc_result cs_bc_validate(const cs_bm_map *map,const cs_bc_bank *banks,
 uint16_t count,uint32_t ram_first,uint32_t ram_bytes);

Include bank-plan.h. No side effects/output writes. Error order:
1 null banks ARGUMENT;then cs_bm_validate map: ARGUMENT->ARGUMENT,MAP->MAP.
2 count must equal map.bank_count;RAM interval nonempty,ram_first<0x100000,
 ram_bytes<=0x100000-ram_first. Otherwise BACKING.
3 each descriptor references a unique existing logical bank id;its byte length
 comes from map.banks. native_offset<=65535,bytes<=65536-native_offset.
 physical_base>=ram_first,physical_base<0x100000,and full bank fits the one
 owner-supplied RAM interval,using checked subtraction bounds. No wrap.
4 reject overlap of any two canonical physical spans,even if bank IDs differ.
 Count+unique/existing IDs establishes exact logical-bank coverage,no extras.

Caller supplies truthful live aligned immutable disjoint map/descriptor arrays,
real capacities,canonical physical bases,a qualified relation to future native
representation,and an independently owned usable RAM interval. This function
does NOT verify those external obligations. Native chunks still<=32767;bank
admission alone does not validate a complete bank as one C16 pointer span.
Lifetime/generation/stable ownership/infallible commit remain prerequisites.

C99,C16 safe:full32 arithmetic/masks,constant shifts only if any,no division,
modulo,variable32shift,allocation,IO,far cast,aggregate assignment or memory
copy helpers in core. Bounded unsigned loops;all additions used for overlap
must follow individually checked intervals. No per-byte native overhead.

Tests:literal valid128KiB map over6 odd-size banks;exact end1MiB;native offsets
0/1/65535 and lengths at wrap boundary;wrong counts/IDs/duplicates;overlaps
including differently based partial containment;RAM gaps/outside/end overflow;
malformed late logical bank precedence;null args;input bytes unchanged. Tests
can use libc and are proposed,not passing. Codex supplies independent oracle,
host/sanitizer/C16 compiler checks;worker must not execute anything.
