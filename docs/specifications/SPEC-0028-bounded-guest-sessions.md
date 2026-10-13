# SPEC-0028 - Bounded 68000 sessions

Finalized v1, 2026-10-13; lead self-review, human provenance review pending.
Profile `classick-guest-sessions-v1`; SRC-0132/0133 / ADR-0032 /
IMPL-0032 / TEST-0036. This is an original OS/core execution boundary, not an
application loader or a historical startup ABI. SRC-0130's intake and the
existing SPEC-0022/0025/0026/0027 interfaces supply prerequisites; no new manual,
sample code, external implementation or executable is an input.

## Ownership and admission

Caller owns a session, immutable truthful map/config/options, gateway, CPU frame
and report. Native objects and borrowed buffers are disjoint, live and synchronized
as in SPEC-0022/0026. No allocation, global state or native function pointer
derived from guest data. No concurrent use, active reinitialization or mutation
of borrowed configuration. Sessions admit ordinary supervisor execution only.

Initialization reuses SPEC-0026's validated read/write nonexecutable stack,
four immutable executable NOP return tokens and configured depth 1 through 4.
The caller keeps stack/token storage separate from heaps and persistent OS data.
The session owns its returning-call depth/budget state; callers must not invoke
the embedded call API or modify execution state directly. While inactive, the
host may change its option/configuration values; entry revalidates them and they
remain immutable while active. Its first depth slot belongs to the session.

A request supplies an entry, explicit A5, instruction budget 1 through 1,000,000
and ordinary context. Initial scratch registers, SR and incoming A7 are supplied
by the caller; this service neither zeroes application globals nor claims that
this is the Macintosh application's startup state. Validate the complete entry,
A5 slot, supervisor mode, stack/token push, budget and configuration before entry.
Pre-entry refusal leaves frame, runtime, RAM and gateway unchanged; the report
may change. Missing mandatory native pointers leave the report untouched too.

## Execution and termination

Reuse SPEC-0026's token push and bounded CPU execution. The root return token is
an owned exit veneer, intercepted rather than executed. A valid main return must
arrive by the immediately preceding successful RTS with exactly the incoming A7.
A jump/branch to any token, a wrong depth token or unbalanced return is refused.
These checks are explicit completion conditions, not proof of every guest frame.

Each attempted instruction, including failures, charges all active ancestor
budgets. A session may enter ordinary returning callbacks synchronously through
the session's call API. The same trusted executor and budget tree apply to them;
callbacks cannot choose an independent executor or reset a parent budget. The
root counts toward the configured service depth. Guest BSR/JSR nesting is instead
bounded by the stack and the shared instruction budget.

The default executor is the existing bounded bus interpreter. An optional trusted
OS executor has SPEC-0027's step shape and must fill the whole bus report, obey
stack/control rules and handle its own completed effects. It is selected by the
host, never by casting a guest token. This slice installs no manager callback or
guest patch binding; future consumers need their own typed contract.

A synchronous trusted root-step service can request an explicit no-return exit
with an opaque unsigned32 host status. The request must occur during a root
instruction, with an active session and no prior request. Requests made outside
execution, in a returning service callback, or repeated within the same instruction
are refused without changing the pending exit. Ordinary guest subroutines remain
in the root execution slot and can reach this service. No-return propagation
through returning callbacks is deferred until their completion-kind contract.

An accepted exit ends the session only if the requesting instruction completes
successfully; an actual CPU/service failure takes precedence. The executor stops
before another instruction or budget charge. An explicit exit on the final allowed
attempt is valid. This is original stop plumbing, not an implemented A9F4 trap:
actual ExitToShell, Toolbox stack dispatch/auto-pop, continuation transfers and
guest Launch/Chain binding remain separate. No guest instruction directly invokes
a native exit-request function.

## State and reporting

Every entered termination restores the entire caller CPU frame, including scratch
registers and SR. A separate guest-stop frame in the report exposes the last root
instruction's state before caller restoration. Callback completions are not
misreported as the root stop. Reports distinguish valid return, explicit exit,
argument/config/context/address/depth refusal, exhausted budget, CPU/service fault
and invalid return. Report attempted root-plus-callback instructions, stop PC,
actual CPU/memory cause, entered flag and exit status only for accepted exit.
The internal stop signal must not be reported as a CPU fault on successful exit.

Unwind all owned depth slots and clear transient executor/exit state on every
entered termination, so the session can be reused. Preserve completed guest RAM,
gateway and trusted-service effects, including the initial return-token write.
No whole-session rollback or automatic heap teardown is promised. Individual CPU
instructions retain their existing transactional obligations. A fault snapshot is
distinct from a later explicit application-lifetime teardown.

## Required verification and scope

Original hosted tests cover valid return and guest snapshot, explicit exit inside
ordinary guest subroutines, last-attempt exit, failure precedence, exit admission,
complete caller restoration, pre-entry refusal, reused sessions, immutable token
checks, forged/unbalanced returns, retained RAM/gateway effects, stack overflow,
shared ancestor budgets and service depth. An original 18-byte assembler fixture
checks a four-byte immediate-word push to predecrement A7, its two-byte stack
effect, pop, BSR and RTS. It is independent of Claude's unexecuted launch fixtures
and supplies no evidence for AL-2 or a historical jump-table offset.

Verify strict hosted compiler/sanitizer profiles, standalone width/endian imports
and state, original-68000 objects, complete relocatable links, assembler encoding
and partial metadata/stack costs. No native application, loader, heap reset,
Resource Manager/MFS/QuickDraw, boot-entry integration, emulator/VM/reference run
or historical/physical/final128K/parity/three-edition certification. Existing
source/UART boundaries, owner native gate and B2 criterion6 remain unchanged.
