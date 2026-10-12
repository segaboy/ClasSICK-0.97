# SPEC-0020 - Shared trap-address services

Finalized v1, 2026-10-12; lead self-review, human provenance review pending.
SRC-0088-0090 / SRC-0069; ADR-0023 / IMPL-0024 / TEST-0028.
Profile `classick-trap-address-v1`: a bounded engineering slice adopting named
later-manual interfaces. It does not finalize SPEC-0017 or claim January 1984
compatibility, application execution or a complete historical trap gateway.

## Published interface facts and adopted behavior

SRC-0089's published format has A in the high nibble, bit 11 selecting word
format. OS number is bits 0-7, A0-result flag bit 8, routine-specific flags 9-10.
Toolbox number is bits 0-8, reserved bit 9, auto-pop bit 10. Decode all words
without implying bit 11 universally specifies the calling convention. Both
formats select a logical directory with 512 entries; OS accesses its first 256.

GetTrapAddress (A146) receives a number in D0's low word and returns the current
address in A0. SetTrapAddress (A047) receives that number and address A0, replacing
the entry. Direct calls bypass dispatcher register/condition-code handling.
For these two register services, preserve registers except the documented A0
result. Trap-mediated Get restores A0 when bit 8 is zero. Successful mediated
calls test D0's low word: N from bit15, Z from zero, V/C clear and X unchanged.
The two intrinsic operations leave D0/other registers unchanged by our explicit
decision; the manual does not specify D0 as a result for either service.

## Original native C directory

Caller supplies a live aligned `cs_trap_gateway`, separate configuration, frame
and report. All objects are nonoverlapping and truthful; callers synchronize.
Directory has 512 u32 logical addresses and configuration/magic, no native
pointers, heap, library calls or mutable global state. It is not the guest-visible
Apple table; direct low-memory table access is unsupported. Independent copies
have independent value storage. Initialization validates all configuration first
and then fills the directory with a caller-chosen unsupported-service token,
installing distinct Get/Set tokens in entries 0x46/0x47.

ROM range is caller-chosen nonzero even base through base+65536, wholly inside
24-bit logical addresses. RAM executable range is nonempty/even, at most 65536
bytes and disjoint from ROM; address zero excluded. Get/Set/unsupported tokens
are distinct even addresses within ROM. These are symbolic service identities,
not executable native pointers or promises of guest-readable ROM stubs.
Patch addresses must be even and in either declared executable range. Actual
guest mapping and execution remain the future CPU/address-space adapter's job.

Invalid configuration leaves the complete gateway unchanged. Decode accepts
only Axxx; rejection leaves its output unchanged. Address get/set accept numbers
0..511; invalid numbers/addresses/state leave state/output unchanged. Setting
one entry never changes another. State validation checks header/configuration;
caller ownership and each logical range's mapping cannot be established by C.

## Calls and failure reporting

`cs_trap_call` takes a decoded word, explicit D0-D7/A0-A7/SR/PC scalar frame and
a disjoint report. It decodes, validates state, selects the current entry, then
performs only Get/Set if the target equals one of their original tokens. This
allows remapping another OS entry to an intrinsic, patching either service's own
entry and restoring through a previously saved direct intrinsic token. The
current call's selected target is fixed before a Set changes an entry.

Toolbox format is decoded and its selected address reported, but returns
UNSUPPORTED_FORMAT: no stack, auto-pop or general callback ABI is implemented.
OS bits9-10 are decoded/reported but rejected with UNSUPPORTED_FLAGS for these
intrinsics, whose routine-specific meanings are not adopted. The A0 flag is
honored for Get. PC/A7 and every other register stay unchanged; callers own
instruction advancement, actual CPU frames and the guest stack.

The unsupported token returns UNIMPLEMENTED. Any other mapped patch address
returns NEEDS_EXECUTION, explicitly requesting the future guest executor; never
cast to a native function pointer or silently treat as successful. All failed
calls preserve gateway/frame; report contains status, word, decoded fields,
selected target and supplied PC. No historical system-error 9/12 presentation.
Null required objects return ARGUMENT without touching them. Direct intrinsic
calls use their saved token, perform the same checked Get/Set, and do not restore
A0 or change SR. Arbitrary direct targets likewise return NEEDS_EXECUTION.

## Probe and acceptance

One original bounded probe invokes both services through the common C boundary,
checks patch lookup/explicit unavailable execution, saves/restores a service's
own entry through its original direct token, and records ten checks, status zero
and signature 0x53564331. Both PC and 68000 images link this same probe/core;
it does not execute an A-line opcode or legacy application. PC storage comes
from a newly named 4-KiB loader-tail slice at 503808; Mac uses own static BSS
below 0x2000, keeping its ordinary 8-KiB stack and hardware loop.

Independent hosted tests cover all 65536 words, guards/failure stability,
configuration/24-bit boundaries, all 512 patch entries, same-number formats,
A0/CCR/register preservation, direct calls and self-patch restoration. Two
compilers, actual i686, sanitizers and ARM64 LE/BE compile audits retain distinct
claims. Own EFI/68000 images must close without runtime libraries and reproduce
per optimization. Native probe execution requires recorded image/configuration
admission and the owner's continuation go-ahead SRC-0088, preserving the already
cleared platforms. Qualify three PC cold starts by successful existing second-60
output before selected own register queries (RCX at result slice, R9=10/R10=0/
R11 signature, R14 marker 0x43535331). Require two stable observations per run.
Qualify two fresh Snow 128K starts by own scene and own diagnostic check/status/
signature values; inspect only our diagnostic RAM and own screen, never code or
disassembly. Compilation is not a boot observation. No generated guest input.

SPEC-0017 Q1-Q10 remain historical/open except the published cross-check
narrows Q9 and ISA word-test prose narrows part of Q8. General Line-A entry,
stack-based calls, callbacks, arbitrary guest patch execution, direct executable
stubs, low-memory view, individual managers and all full OS/edition gates remain.
