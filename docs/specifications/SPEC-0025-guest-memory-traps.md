# SPEC-0025 - Built-in guest memory trap execution

Finalized v1, 2026-10-13; lead self-review, human provenance review pending.
SRC-0110/0111; ADR-0029 / IMPL-0028 / TEST-0033.
Engineering profile `classick-guest-memory-traps-v1` extends SPEC-0022/0023.
Published facts are reused from SRC-0102 and their reviewed interface registry;
no new historical availability, callback convention or reference behavior claim.

## Original binding and CPU boundary

Bind an initialized gateway and manager using eleven unique even logical ROM
identities, consecutive two-byte values starting at a caller-chosen base. All
fit the configured ROM window and differ from Get/Set/unsupported identities.
Initialization validates all candidates and manager state before changing any
entry; only the eleven memory slots change. Preserve other entries. Caller owns
truthful, synchronized, disjoint live native objects and guest buffers. The bound
manager/map/gateway and identity configuration must outlive use.

The runtime resolves each fetched OS trap through the current directory. A memory
identity carries one routine signature; the requested routine must match it.
Another built-in signature is refused, unsupported identities remain unsupported,
and unbound guest code returns needs-execution without running it. Toolbox words
are refused. Allocation SYS/CLEAR and bit8 behavior use SPEC-0023 unchanged.
Get/Set use SPEC-0020 unchanged, so reading and patching memory slots is observable.
A saved original identity can be invoked through a value-frame direct service
with its explicit original trap word; that service does not advance PC or write
an exception frame. No guest-to-native-address cast or guest callback executes.

The bus gains an internal trusted OS dispatch hook and an explicit data-address
policy. The legacy entry remains strict and uses its legacy handler. The hook
is an implementation-owned C adapter, not an application callback or native SDK.
It must preserve PC/A7, preserve frame and external state on failure, and have
bounded work. The CPU checks fetch, mode, continuation and six-byte exception
scratch before invoking it; after success only infallible stores/copies remain.
The CPU alone advances PC by two and records completed SR/PC below unchanged A7.
This remains collapsed supervisor completion, not a general exception engine.

## Address and overlap decisions

Strict addressing remains the default. Opt-in `DATA24` masks only effective data
operand addresses to low24 bits before checked reads/writes. Full address
registers, loaded pointer values and pre/postincrement arithmetic retain all32
bits. Fetch/extensions, PC/branch/RTS targets and implicit stack accesses remain
strict; do not normalize control flow. An explicit MOVE through A7 is still a
data operand; implicit BSR/RTS/exception stack accesses are strict. Word/long
alignment and full-span bounds still apply; accesses do not wrap across 16MiB.
Guest map APIs and all Memory Manager trap arguments remain strict logical
addresses. This is a selected engineering boundary for documented flag-bearing
master cells, not a complete historical bus/address-error implementation.

For bound A-line steps, the six-byte scratch must be above the reserved low-memory
prefix [0,400 hex) and outside both configured heap zones. This protects the
manager's validated globals and the low-memory vectors. A positive BlockMove destination must not overlap this scratch;
refuse before any manager call or write. Invalid counts retain SPEC-0023's
defined errors. Input maps forbid native buffer aliases per SPEC-0022. These
rules prevent successful heap mutations being overwritten by deferred CPU
frame stores. No extra manager/heap snapshot is required for a step. Other
guest data instructions can deliberately alter their writable memory; later
manager calls independently validate cells/globals.

## Results, failures and tests

API errors (for example memFullErr) are successfully executed traps: update
documented registers/CCR, advance PC and publish the completed frame. Contract,
flags, interrupt, signature, map and unimplemented-target failures stop without
changing frame, directory, manager, guest RAM or stack. Runtime reports retain
the decoded word/target and separate memory status/called indicator. Invalid
top-level arguments leave outputs untouched. Interrupt context is explicit
caller input; never infer it solely from interrupt-mask bits. No callbacks,
purging/grow zones, JSR/JMP, guest patches, user-mode execution or boot-entry
integration are introduced.

Tests execute an independently authored program using real A-line words,
allocation/resize/size/locks/copy/disposal and flagged-cell data access. Cover
SYS/CLEAR, bit8 preservation, all eleven signatures, saved identity invocation,
Get/Set patches, wrong identities, interrupt/flags/state/map/scratch refusals,
Macintosh API errors, separate runtimes and full failure atomicity. Sweep tag
bytes and widths, odd addresses, end-of-space bounds and strict control/stack
paths under DATA24. Compare original assembly bytes with the independent GNU
68000 assembler, compile freestanding with Clang/GCC at both host widths and
endian targets, and audit 68000 relocatable linkage/frame bounds. Hosted own
core tests and compiler audits are permitted; native applications remain gated.
Any new VM/emulator execution needs the owner's separate go-ahead. Full OS,
January1984, physical/edition/parity/budget and B2 criterion6 gates stay open.
