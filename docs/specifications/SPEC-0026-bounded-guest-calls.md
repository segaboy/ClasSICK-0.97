# SPEC-0026 - Bounded returning 68000 calls

Profile: `classick-guest-calls-v1`; original engineering contract, 2026-10-13.
Sources SRC-0113, SRC-0116, SRC-0118. ADR-0030; TEST-0034; IMPL-0029.
This slice implements runtime-plan step 3. It does not install guest patches,
implement QuickDraw, load an application, bind a boot entry or qualify January 1984.

## CPU control instructions

Extend SPEC-0022/0025's checked bus interpreter with JMP/JSR control modes:
(An), signed d16(An), canonical brief d8(An,Xn), absolute short/long,
signed d16(PC), canonical brief d8(PC,Xn). D/A word indexes are sign-extended;
long indexes use their signed 32-bit value. PC bases name the extension word.
Absolute short is sign-extended. JSR saves the address after all extensions;
JMP/JSR leave condition codes unchanged. Reject other effective-address modes.
Canonical brief words require bits 10-8 zero. Reject full/scaled encodings as
unsupported by this profile; this is not a claim that original hardware traps
on those bits (the published manual says earlier processors ignore extensions).

Signed address additions reject 32-bit wrap at each addition. Final targets and
all extension fetches are even, strict untagged 24-bit logical addresses with
read/execute permission. No DATA24 control normalization. Negative absolute-short
addresses are consequently outside this profile's strict map. JSR checks its
return address and complete four-byte writable push before any state/store commit.
Instruction failures retain CPU and RAM as before.

A separate bounded-step API borrows explicit stack base/size without changing
the existing options ABI. Before execution and before commit, A7 must be even
and within the configured interval, including its one-past top. BSR/JSR pushes,
RTS reads and A-line exception scratch must lie completely inside it. A MOVE
that leaves A7 outside it fails before its deferred store. This bounds implicit
stack operations; ordinary guest data operands still use the full borrowed map.
Existing unbounded-by-stack-interval entry points retain their prior semantics.

## Returning-call service

The caller owns a runtime, immutable truthful map/config/options, gateway,
CPU frame and report. Native objects/output/buffers are live and disjoint as
in SPEC-0022; guest addresses never become native function pointers. Concurrent
use, reinitializing an active runtime and mutation of its borrowed configuration
are outside the API contract. A synchronous trusted OS dispatch may reenter
the service with the same runtime and its own frame/report.

Initialization validates a read/write nonexecutable stack interval above low
memory (base at least 0x400), and four consecutive NOP words in a read/execute
nonwritable sentinel interval. Both intervals lie in single mapped regions.
Depth is configured from 1 to 4. No allocation or mutable global state.
The owner must place the stack outside heaps and other persistent OS storage;
the generic service does not infer allocator ownership from a RAM span.

The only supported signature is an ordinary zero-parameter, no-result returning
procedure, represented by an owned enum. Its Pascal shape is documented for
DragGrayRgn's action procedure (I-294/295); manager installation is not included.
Unknown signatures, interrupt contexts and nonreturning entry kinds are refused,
not inferred from a pointer. Parameter/result marshalling and other signatures
need later contracts. A5 is supplied explicitly; its four-byte aligned writable
slot is checked. Initialization of that slot and CurrentA5 belongs to the caller.

Each request has a budget from 1 to 1,000,000 instruction attempts. Before entry,
validate runtime/map/sentinels/options, signature, budget, depth, entry target,
A5 slot, caller mode and A7, plus a complete return-token push. A nested request
also refuses before entry if any ancestor budget is exhausted. Nested incoming
A7 may not exceed its parent's saved return-token address. Pre-entry failures
change only the report. The service then pushes its depth-specific guest token,
sets guest PC/A5 and executes with stack limits whose top is the incoming A7.
Every attempted instruction charges all active ancestor budgets, including
failing instructions. Nested calls cannot reset an ancestor's execution limit.

Completion requires reaching this call's sentinel via the immediately preceding
successful RTS with exactly the incoming A7. Reaching any other sentinel, a JMP
or branch into the sentinel, or an unbalanced RTS fails. The sentinel is never
executed. These are explicit completion checks, not a cryptographic proof of
the guest's return sequence. Recursive guest subroutines are bounded by the
stack interval and shared instruction budget; service nesting by depth as well.

On success expose the resulting guest scratch registers/CCR, restore caller
PC/A5/A7, and report an optional diagnostic mask for changed Pascal-preserved
D3-D7/A2-A6 (A5 compared with the requested context). Diagnostics do not enforce
historical register correctness. On execution failure restore the entire caller
frame and unwind the owned depth slot. Retain prior guest RAM/gateway/service
side effects, including the initial return-token write; no whole-call rollback
is promised. A failed individual CPU instruction remains transactional within
its documented dispatch contract. Trusted dispatches retain their own failure
obligations; adapters must handle nested-call failures explicitly.

Reports distinguish bad argument/config, unsupported signature, invalid address,
depth, budget, CPU fault and invalid return; include attempted instruction count,
stopped PC, CPU/memory cause and preservation diagnostic. No guest callback,
result global, signature or pointer is automatically registered from a draft.

## Required verification and limits

Original tests cover every opcode EA field, canonical indexed sign/width/register
variants, extension truncation, permissions/odd/tagged/wrapping targets, exact
JSR return bytes, rollback, stack limits, normal/forged/unbalanced returns,
early refusal, retained side effects on late failure, nesting/depth/ancestor
budget exhaustion, A5 restoration, successful traps inside calls and reuse.
An original assembler fixture independently checks JSR/JMP encoding. Hosted
compiler/sanitizer matrix, freestanding import/global audits and original-68000
objects/complete relocatable links validate portability within stated scope.
No VM/emulator or reference-system run is part of this authorization. Native
applications, full ISA/managers/OS, physical/128K budgets/parity/three editions
and B2 criterion 6 remain open.
