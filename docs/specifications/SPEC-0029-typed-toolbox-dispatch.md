# SPEC-0029 - Bounded typed Toolbox argument and completion dispatch

Finalized engineering v1, 2026-10-13; SRC-0139/0140, ADR-0033 / TEST-0037 /
IMPL-0033. Lead self-review; human provenance review pending. This is an original
hosted OS/core marshalling boundary, not a first-application manager implementation.
Reuse reviewed SRC-0137/0138 calling facts and SPEC-0022/0026/0028 services.
No new reference/manual/sample/implementation input.

## Types and ownership

A caller-owned immutable table has at most sixteen unique canonical Toolbox words
and even logical built-in identities in the gateway's ROM window. No identity is
a native function pointer. Each record explicitly selects stack or register
convention, returning/continuation/no-return completion, up to eight declaration-
ordered parameters and a result class. Toolbox bit11 alone never selects the ABI.
Register-convention records and loader continuations are recognized but refused
before effects in v1; unknown records refuse too. Bit9 is refused; bit10 is auto-pop.

Stack classes are raw int16/int32, bool16 (high-byte bit0), char16 (low byte),
pointer to a fixed span, counted-byte string, and a raw32 value (including Point).
Fixed spans carry explicit alignment1/2, read/write access and byte length. A
write-capable span contributes to one bounded sixteen-byte staged output stream,
in declaration order. Retained-pointer descriptors are explicitly refused;
manager handles, sentinels, private records and transitive callback support need
later routine contracts. Counted strings are read-only, one length byte plus its
unsigned byte count (at most256 bytes). No native packed-record casts or data24
masking of trap arguments. All scalar values preserve unsigned raw bits except
canonical bool/char decoding; native signed overflow is never used.

The table, map, options, session, gateway, CPU/report/runtime and borrowed native
buffers must be live, truthful, synchronized and disjoint. While running they are
immutable except explicit gateway entries and owned reports. Init checks every
record/map/identity before publishing the table's slots; other gateway entries
are unchanged. Init is inactive-only. Revalidate immutable config at entry.
The session stack must remain in one read/write, nonexecutable region; return
tokens and logical builtin placeholders remain immutable read/execute NOP words.
Selected slots must be unsupported or already hold their declared original
identity at init; never overwrite a preexisting patch or another service.
Inactive table changes are permitted only with complete revalidation at the next
entry; table/options/config remain immutable throughout active execution.
Native trusted handlers are bounded pure OS computations over decoded values and
read-only views. They return a result and exactly the staged output stream; no
guest execution, external mutation, retained view, manager allocation, host heap
allocation or runtime reentry. No function pointer comes from guest data.

## Frame and alias contract

At an intercepted A-line with incoming SP=S, parameters start at S (last declared
parameter nearest S), result space begins at S+P, and the logical normalized
routine entry has a return long at S-4. Thus a parameter's documented entry offset
is4 plus its offset from S. No physical guest callback frame is exposed or pushed
in this v1 builtin path. The completed collapsed exception scratch is S-6..S.
This distinction is explicit; it does not explain PE LoadSeg's24(SP).

Preflight the whole six-byte scratch, parameter/result/optional auto-pop-return
span inside the supplied owned stack, with checked arithmetic, even SP and full
permissions. A normal continuation PC=trapPC+2 must be strict/even and mapped
read/execute before invoking the handler. Every fixed/string span must be fully
mapped with the declared permissions/alignment and outside the entire owned
execution stack. Input/output pointer spans must not overlap one another;
reject even read/read aliases as a bounded engineering restriction. This excludes
scratch/result/control-token aliases. Caller must place heaps/persistent OS data
outside the stack; no metadata reconstruction or heap inference is performed.
Input code can be read through a read-only span but cannot be a write destination.

No-return and continuation/register refusals occur before pointed-memory access
or handler entry as appropriate. Contract failure leaves frame, gateway and guest
RAM unchanged; only the diagnostic report may change. A trusted-handler refusal
is also atomic because it returns staged data only. Reply output length must
exactly match the preflighted total before any commit. All fallible checks precede
result/pointer/scratch writes; no rollback allocation or full-heap snapshot.

## Completion kinds

Returning builtins write the typed result at S+P, staged fixed outputs, PC=trapPC+2
and A7=S+P. Result space remains for the caller. This engine preserves other
registers and SR/CCR as an explicit engineering superset, not a historical stack-
CCR promise. Result int16/raw32 retain low16/all32; bool and char use canonical
zero padding. The original profile does not execute installed guest stack patches:
changed gateway targets return needs-execution before effects; mismatched built-in
identities refuse as signature errors. Reading/patching slots remains observable.

Auto-pop is admitted only for returning procedures (no result). It reads the
caller's explicit return long at S+P, preflights a strict mapped executable target,
then sets PC=that target and A7=S+P+4. A-line instruction attempts still cost one
guest instruction. Function auto-pop, no-return auto-pop and continuation auto-pop
are refused. Auto-pop to a session's owned return tokens is also refused: current
SPEC-0026/0028 owned completion requires the immediately preceding successful RTS,
and this slice does not weaken that rule. No implicit stack scan or guessed frame.
Known logical built-in identities are also refused as continuation/auto-pop
targets before handler entry, because their direct-call contract is deferred.

The only no-return record admitted is parameterless/resultless A9F4 ExitToShell,
with no handler. During an active successful root session step it requests an
explicit exit status0 through SPEC-0028, then completes PC/scratch infallibly.
Known/unknown interrupt context, inactive/direct step, returning callbacks,
preexisting exit request or auto-pop refuse before effects. Caller restoration and
separate guest-stop diagnostics remain SPEC-0028; no Finder launch, heap teardown,
native variant selection or historical application-start/exit claim.

Continuation metadata reserves the LoadSeg completion shape; execution is an
explicit unsupported result pending a separate validated loader contract. Direct
execution of logical built-in identities is also refused, rather than letting
their placeholder bytes silently execute as a service. Saved original/direct-
veneer and Pascal guest-patch conventions remain later contracts.

Other instructions and OS-format traps retain the supplied bounded bus/options
path. Runtime driver is trusted OS code used by SPEC-0028 run_using; all guest
instructions share its existing stack/depth/ancestor budgets. No guest callback
or new callback signature is installed by this contract. Step diagnostics record
declared convention/completion, target, parameter/output counts and precise
unsupported/type/map/alias/reply/context cause. Whole bus reports are initialized
on every step; normal CPU failures retain their own diagnostics. Unadmitted
records have explicitly absent convention/completion fields. Missing mandatory
native pointers leave outputs untouched, as in the existing CPU/call contracts.

## Verification and limits

TEST-0037 uses wholly original hosted records/programs for mixed-width declaration
order, result slots/encodings, strict pointer/string spans, staged writes,
permissions/alias/overflow, malformed config/replies, patched/wrong identities,
explicit convention/continuation refusals, normal/auto-pop/ExitToShell completion,
callback exit refusal, retained preceding effects, budget/fault/caller restoration,
independent runtimes and original assembler bytes. No Claude HT/LO or launch
fixture is generated or executed. Verify strict width/optimization/sanitizer,
freestanding/endian imports, complete original68000 closure/encoding and partial
storage/frame costs; no emulator/native application execution. Source/role/owner,
native, physical/128K/parity/three-edition/B2criterion6 gates remain unchanged.
