# Bounded guest-session checkpoint - 2026-10-13

SPEC-0028 / ADR-0032 / IMPL-0032 / TEST-0036 / SRC-0132-0134.
Compiled source `d9c0f55cc08e11306463f7721afece90013cf01f`; [CI](https://github.com/segaboy/ClasSICK-0.97/actions/runs/38470853695)
passes both jobs. Complete original68000 manifests match local/CI.
Lead self-review; human provenance review pending.

The original session composes existing checked calls and CPU steps without
changing their semantics. Root and returning callbacks share owned stack/token,
depth and instruction budgets. A valid RTS return or successful trusted root-step
exit ends execution; faults/exhaustion/invalid returns are distinct. A separate
root-stop frame preserves diagnostics while every caller register restores.
Completed guest/service effects survive; no heap teardown or whole-session rollback.
No-return requests from returning callbacks are explicitly refused in this profile.
Actual ExitToShell/Toolbox dispatch, loader and heap reset remain unbound.

Initial complete nine-profile1542 CTests pass. After header/contract ownership
clarification all63 affected final session checks repeat; unchanged1479 checks
are retained. Final ten width/endian object audits and both compiler debug twins
pass. The original18-byte push/pop/BSR/RTS fixture returns in seven attempts;
the four-byte push instruction consumes two stack bytes. Exit/fault precedence,
last-attempt exit, malformed returns, restoration, retained writes/trap effects,
early refusal, nesting/depth/ancestor exhaustion and reusable state are covered.

GNU68000 twelve module audits/two complete relocatable links and18 assembler
bytes pass. Session/report/request storage is124/98/16 bytes (238 total; session
includes the92-byte call runtime). Closure compiler frame maxima are228/O0 and
252/Os. These are partial costs, not cumulative stack or final128K measurements.

CI runs the complete final-source1542 matrix plus368 bootstrap checks. Six
existing EFI profiles and both own ROM profiles/input fingerprints are unchanged.
Eight prior research originals are unchanged/untracked; newly arriving Claude
heap/convention research remains separate and unreviewed. Preparation corrections
and superseded native metadata are retained in the JSON record.

No native application/SDK, resource loading, MFS/QuickDraw, boot-entry binding,
new VM/emulator/reference trial, UART body work, prohibited code/assets, delegation,
chats, automation or OneNote. Native owner, historical/full OS/physical/final128K/
parity/three editions and B2criterion6 gates persist. Next: a separately reviewed
typed Toolbox stack/completion contract; heap lifecycle research feeds later reset.

[Machine-readable evidence](guest-sessions-evidence.json).
