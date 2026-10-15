# Focused check: SPEC-0034 draft - 2026-10-15

Owner SRC-0179 authorizes this second and last planned CLI call. Grok provides
an independent check, not another full draft. Codex owns disposition/publication.
Read this brief and `docs/specifications/SPEC-0034-owned-image-execution.md`
in bounded chunks of at most100 lines, at most three more Read calls. No other
files, searches, web, commands, edits, execution, subagents or new research.
Inputs are original project engineering material; historical questions stay open.
No HQ/raw research/manuals/Apple or Microsoft code/ROMs/binaries/disassembly/
disk images/emulator source/credentials/manual samples/private heap reconstruction.

Check only whether the proposed fetch/write/stop/retirement boundary is coherent.
Known reviewed own facts: each segment has a four-byte header; logical bytes
exclude its even-rounded physical padding. Up to64 eight-byte jump rows: BE16
segmentID, BE16 JMP0x4EF9, BE32 literal resource-start+routineOffset. The opcode
is row+2. SPEC33's saved map is non-EXEC in app/world/stack, immutable; its current
gate permits EMPTY only. It seals its inventory; unknown other FUTURE entries
refuse. RELEASE verifies loaded rows and zero world fields, preserves stack,
then disposes handles. It stores no loaded-payload digest after staging ends.
SPEC28 binds immutable default/trusted executors and actual stack/token checks;
its return tokens are intercepted after an immediately preceding balanced RTS.
SPEC29 already implements typed root-only A9F4 via a trusted driver, without
handler or auto-pop. That existing implementation does not supply new per-access
guards. Existing APIs do not automatically intercept every independent host entry.

Find up to five actionable defects in authority for opcode/extension bytes,
logical/physical aliases, instruction transaction ordering, single-use/stop state,
or safe retirement. For each give section, concrete trigger, required correction.
Do not review style, add a broad feature list, propose historical behavior, or
write a replacement contract. At most350 words. Clearly label unexecuted cases.
