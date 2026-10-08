# TEST-0009 — Hosted debugger launch, source, state, step and resume

Date 2026-10-07. IMPL-0007, existing SPEC-0002–0005. Original Codex commands and
evidence predicates; self-review, human review pending. Tool interfaces SRC-0026/0027.

Local pinned LLVM-MinGW 20260908 / Clang and LLDB 23.1.1, CMake 4.4.4,
Ninja 1.13.2 and Windows PowerShell 5.1. The five-configuration matrix passes
34/34 checks each: Debug twins, Release, actual i686 and validated x64 ASan/UBSan.
Sanitizer controls and optimized core/endian audits pass; ARM64 compile-only.

The initial viewer debugger probe failed: raw WinMain had a trailing delimiter
and returned 2. Parsed runtime arguments fixed that launch. Six x64 scripted
sessions then pass across fresh Debug twins:

- Core: source breakpoint at cs_clock_advance, typed initial 7s/900000000ns and
  0s/200000000ns duration, source/stack inspection, step over/in/out through our
  addition, completed result assignment, sum/first clock 8s/100000000ns and
  independent second clock 1s/0ns, resume, PASS and child exit zero.
- Viewer controlled time: source breakpoint at verify_print, 1342744 arena bytes,
  typed first input record/source/sequence, exact deadline; on at 249999999ns,
  off at 250000000ns, correct stack/callers and child exit zero after pixel/timer
  assertions. The raw launch delimiter is explicitly observed in the stack.
- Viewer live time: a real delivered Windows timer/QPC sample, positive wakeup
  count/frequency, failed flag zero, then resume and child exit zero.

Three more debugger launches verify unknown, extra and empty options reject with
child exit 2. LLDB command failure, missing stop/value evidence or wrong child exit
fails verification. Logs/commands/hash summary are local, not tracked payloads.

A local interactive console core session separately exercised launch, source,
typed inputs, step over/in/out, sum/independent state, resume/PASS/exit zero and quit.
Core sources were unchanged from clock source 958e479; interactive executable came
from that previously verified matrix, followed by final fresh-build replay.

An optional i686 debugger extension failed in WOW64 at 0x4000001f with unavailable
original frame variables. The full core matrix still passed. Supported x64 replay
was rerun against those same tested binaries after restricting replay to the actual
verified scope; existing failure logs were preserved. No failed trial is promoted
to a pass, no tool workaround/upgrade is claimed. The upstream issue is a symptom
lead, not proof of root cause. The final wrapper's remote result is recorded only
after observed completion in [debugger evidence](../../docs/development/debugger-evidence.md).

Updated viewer Debug twins match; seven other named executable hashes retain
their clock values. One pinned compiler only; archives/OS images/general compiler
or cross-machine reproducibility unproven. No physical keyboard/display acceptance,
guest/remote/m68k debugging, B1 closure, historical timing or native edition boot.
