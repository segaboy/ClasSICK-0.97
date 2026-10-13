# TEST-0036 - Bounded session conformance

SPEC-0028 / IMPL-0032 / SRC-0133/0134; original OS/core hosted conformance.
This record covers implemented tests/m68k-session.c, not Claude's case-only
launch/heap research. There are no historical observation probes or CODE files.

Six independently authored suites:

- programs: original18-byte fixture, seven attempts, two-byte word push,
  four-byte instruction length, CCR/pop/BSR/RTS, stopped scratch registers/A5/A7,
  complete caller restoration, ordinary built-in trap and repeated runs.
- exits: explicit trusted root exit in direct and BSR-nested guest execution,
  last-attempt success, opaque status, duplicate request refusal, stop snapshot
  and runtime reuse. No actual ExitToShell binding is implied.
- admission: invalid budgets/entry/A5/mode/stack/context, altered tokens/config,
  null arguments/constructor and exact pre-entry frame/runtime/RAM/gateway refusal.
- failures: loops, forged/wrong/unbalanced token transitions, illegal instructions,
  retained writes/trap-directory updates, stack exhaustion and failing push.
- nesting: shared root/callback attempts, ancestor budget exhaustion, refusal before
  a callback attempt, four-slot depth, root reentry refusal, callback exit refusal,
  root stop distinct from callback completion, clean unwind and caller isolation.
- boundaries: successful request versus actual instruction-fault precedence,
  callback refusal after pending exit, truncated extension fetch, exact budget and
  depth-one session. Freestanding import/global-state audit is an additional check.

Compiler-profile counts and exact accepted revision, artifacts, native closure,
partial storage/frame measurements and preparation corrections are reported by
the checkpoint evidence. A passing hosted fixture verifies this selected execution
policy, not Macintosh startup state, AL-2, final RAM/stack budgets or an OS boot.
Native applications, new VM/emulator/reference observation and all edition gates
remain separate; no research case is counted as executed by this record.
