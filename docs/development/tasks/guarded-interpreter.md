# Task brief: guarded interpreter X-1 tests - 2026-10-15

Owner SRC-0183: "Do it." implements SPEC-0034 dependency X-1 only. Codex
authors/integrates the primitive and is the sole repository writer. Claude
provides the primary original conformance-test artifact; Grok independently
checks one bypass/atomicity risk after integration. Initial budget two CLI calls.

Read this brief, then only these public original project files, in chunks of
at most120 lines per Read (all paths C:/Repos/ClasSICK-0.97):
- core/cpu/m68k-guard.h (44 lines), core/cpu/m68k-bus.h (28 lines).
- core/memory/guest.h (35 lines), core/services/traps.h (39 lines),
  core/cpu/m68k.h (20 lines). Read these small files only if needed.
- tests/m68k-bus.c lines1..92: existing fixture and encoding patterns.
Do not read implementation files, manuals, untracked research, HQ or other files.
No web, commands, edits, MCP, subagents/chats, automation or OneNote. No Apple/
Microsoft code/ROM/binaries/resources/disk images/disassembly/emulator source,
manual samples or private heap reconstruction. Standing SRC-0149 public/original
scoped input transfer applies. No native app, loader/session/image execution,
emulator/VM/reference-system run; these are hosted instruction conformance tests.

## Deliverable

Return ONLY the full original C file for tests/m68k-guard.c, <=380 lines, with
SPDX/copyright and standard C11, -Wall -Wextra -Wpedantic -Werror -Wconversion
-Wsign-conversion. Tests may use libc. Include ../core/cpu/m68k-guard.h. Main
accepts one suite: events, atomicity, control, services, aliases. REQUIRE prints
failure line. No external fixtures. No tests executed by you.

Create separate code RX, data RW and stack RW buffers and live disjoint native
objects. Frame starts PC0x401000, SR0x2700, A7 inside stack. Policy is PURE: no
event logging or counter mutation. Context is immutable and explicitly sized.
Use selected kind/address/width/word and proposed frame predicates to deny.
When asserting event coverage, deny one event that must occur: observable POLICY
and unchanged RAM/frame/gateway establishes interception without a logging hook.
Permit-all remains valid but cannot override map permissions or A-line ban.

Events cover opcode, immediate/absolute extension (four bytes as one whole span),
source read, destination write, taken branch/BSR/JSR/RTS target and instruction
class. COMMIT policy denial must withhold pending store, flags, PC, A-register
increments. Late extension denial after staged source increment must also be
atomic. A denied JSR/BSR target must leave stack/A7 unchanged. RTS stack read
and target independently deny; distinguish TARGET from OPCODE. Exact instruction
encoding may use existing original fixture helpers. Only already supported
MOVE/MOVEA/MOVEQ/NOP/branches/BSR/JSR/JMP/RTS are in scope.

Control: register-only MOVEA to odd/out-of-bounds A7 fails ADDRESS, initial
S/T/I mismatch fails MODE; CCR changes allowed. Strict24 never masks high data
address, denied map R/W/X still fails ADDRESS, unknown/F-line/TRAP opcodes fail
OPCODE without effects. Every A-line (A046/A047/A9F4 etc.) fails POLICY before
gateway mutation/service_called/stack write even permit-all; legacy unguarded
behavior remains covered by its existing suite. New API has no dispatch hook.

Aliases: f/report overlap, report in guest backing, context in writable backing,
overlap of distinct native backing spans at distinct guest addresses, and
missing policy/context extent mismatch must return ARGUMENT and preserve report
bytes (as well as other live outputs). Do not construct invalid/dangling native
pointers or rely on misaligned dereferences/undefined behavior; use unions for
aligned overlap cases and ensure each backing declared extent is actually live.
Include success cases to prevent an always-deny implementation passing.
