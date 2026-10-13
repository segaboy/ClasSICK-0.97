# SPEC-0027 - Typed bounded register trap patches

Finalized engineering profile `classick-register-patches-v1`, 2026-10-13.
SRC-0121/0122/0123; ADR-0031 / IMPL-0030 / TEST-0035.
Extends SPEC-0025/0026 through a separate opt-in personality executor.

Published SRC-0071 I-94/95 documents register-trap preservation of A1/D1/D2,
bit8-controlled A0 and the low-word D0 condition test. II-383/384 documents
saved-original chaining and direct calls without the dispatcher's extra saves.
These later-edition interfaces do not prove January1984 behavior. No sample
instructions, implementation structures or external executable are adopted.

The new resolver returns an owned built-in or checked guest-code entry with an
explicit OS trap word. Only eleven existing memory signatures plus Get/Set are
admitted; Toolbox, unknown signatures and unsupported identities are refused.
Allocation alone admits SYS/CLEAR. Logical identity/signature mismatches fail.
Guest entries require strict even mapped READ|EXECUTE addresses inside the
gateway's eligible address windows. Get/Set still use the existing directory;
saved built-ins bypass it. No guest address is cast to a native function pointer.

The runtime owns a bounded call service and a per-call instruction budget. Its
stack must be outside both heap zones and reserved low memory. Built-in memory
identities must have immutable mapped RTS words, used as logical veneers, not
native pointers. Get/Set direct veneers are deferred. A guest patch receives
the register inputs and explicit current A5, returns by RTS, and may call its
own saved memory identity by JSR or tail JMP. That veneer uses the active trap
word, including allocation flags. Calls to another memory identity are refused
as a signature mismatch; use its A-line interface instead. Direct guest calls
outside an active patch require a future contract.

A trusted internal step callback lets the bounded call service execute these
logical veneers and nested patched A-lines. This callback is supplied by the
OS, never by guest code; it is distinct from the legacy atomic dispatch hook.
Each step charges all active ancestor budgets. Synthetic veneer returns first
validate the complete return read, strict executable target, stack transition
and signature, then invoke the built-in; no fallible work follows its success.
The ordinary PROC0 API retains its behavior. A second owned register-return
signature adds no Pascal parameters or results. Both retain preservation
diagnostics, bounds, owned completion, A5/A7 and failure restoration.

For patched A-lines, preflight fetch/mode, continuation, bounded stack and six
scratch bytes outside low memory/heaps and BlockMove destination. Refuse
interrupt context before any patch entry. Execute the selected guest under a
depth-specific sentinel. On success restore A1/D1/D2, A0 when bit8 is clear,
caller PC/A5/A7; preserve caller SR high bits, keep returned X, and compute
N/Z/V/C from low-word D0 (V/C clear). Advance PC two bytes and publish the
collapsed exception scratch as in SPEC-0025. Callee D3-D7/A2-A6 preservation
violations are diagnostic, not historical correctness guarantees.

Before guest entry, failures preserve frame, manager, directory and guest RAM.
After entry, failure restores the complete caller frame and unwinds service
state, but retains completed guest RAM/manager/directory effects and return
token writes. Reports explicitly distinguish entered versus refused, retain
call/CPU/service causes, and never claim whole-call rollback. A nested failure
stops the active patch; the same retention rule applies. Legacy SPEC-0025 APIs
continue to return needs-execution atomically for guest targets. This executor
does not invoke a fallible guest through their atomic dispatch hook.

Truthful disjoint caller-owned live objects/maps are required; active runtime
configuration is immutable and synchronous reentry only. Guest memory changes
may invalidate manager state, which later calls independently validate. Tests
use original bytes and assembly, all eleven original-chain signatures, flags,
register/CCR rules, nested patches, depth/ancestor budgets, map/signature/scratch
refusals, direct-return preflight and retained effects on late faults. Hosted
compiler/sanitizer, freestanding/import/endian and original68000 linkage checks
are required. No new boot binding, VM/emulator/reference trial, native application,
SDK, QuickDraw, full CPU or historical/physical/edition/budget claim.
