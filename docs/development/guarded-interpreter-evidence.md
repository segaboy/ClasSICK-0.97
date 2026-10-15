# Guarded interpreter verification - 2026-10-15

SPEC-0034 X-1 / IMPL-0038 / TEST-0042 implements guarded single-instruction interpreter transactions. Actual opcode, extension, read, write, target, instruction and final-state checks precede staged RAM/frame publication. All A-lines refuse before either service route; strict24, bounded even A7 and S=1/T=0/I=7 apply. Existing unguarded APIs retain their semantics. X-2..X-5 and full owned-image execution remain unimplemented. No application/image/session entry, reset/boot integration, native application or VM/emulator/reference trial. All24 research originals/copies and source/UART/native/physical/full128K/parity/edition/B2/human gates and both untested System1/ROM goals remain.

All2055 CTests pass across nine configurations: [241, 241, 241, 214, 241, 226, 226, 226, 199]. Both compilers' fresh
debug guard-test twins match. Full inherited verification passes, including
30 actual guest/bus/probe object audits across32/64-bit O0/O2 and ARM64 little/
big-endian targets, with permitted own imports and no mutable process globals.
ASan/UBSan profiles and their existing positive controls pass. The first focused
attempt omitted the probe object target; building that target fixed the audit
invocation. It was not a code/test failure and is excluded from final proof.
Compiled revision and publication CI are pending recording.

[API and lead dispositions](guarded-interpreter-implementation.md);
[machine-readable evidence](guarded-interpreter-evidence.json);
[original test record](../../provenance/records/TEST-0042-guarded-interpreter.md).

Five new suites cover actual spans and all seven event kinds, withheld pending
stores/flags/register increments/control changes, target checks before BSR/JSR
stack writes, RTS read versus target, register-only A7 writes, S/T/I and free CCR,
strict24/map permissions, unsupported exceptions, every4096 A-line opcode,
retained earlier success, and native alias/no-report-write refusals. Original
unguarded interpreter/service suites still pass. These are original hosted
instruction fixtures, not a loaded application, bridge/session or reference run.

GNU68000 O0/Os passes ten module import/global audits and two complete zero-import
relocatable links through the actual decoder, guest memory, legacy CPU/services
and original36-byte assembler fixture. Largest individual closure frame is348
bytes in each profile, not cumulative stack or complete128K fit. The guard policy
has no standalone68000 runtime trial. Local/CI guard manifest matching is pending.
The complete local boot-media result equals the previous checkpoint. Own-ROM
binary and known shared-header input comparison will be recorded with CI;
no unchanged complete-ROM-manifest claim is made before that comparison.

Exactly two supervised CLI calls: Claude supplies the primary290-line test file,
Codex reviews/integrates/adds complete A-line/proposed-state cases, Grok checks the
commit/access/service boundary. Seven Reads each, no truncation/retry or other
recorded tool kind. Raw streams, reports, byte-identical input copies and selected
range identities remain private; sanitized metadata is above. Model-reported usage
and list estimates are not subscription quota, bills or a scope-normalized savings
benchmark. Worker tests executed0; code/expected outcomes are lead-verified.

All24 originals/private copies and prior build/edition/target records are preserved.
Existing UART bodies remain uninspected/unedited; full mechanical regression is
allowed. Historical R-001/AL-2/LQ and prior source questions, native-app, physical,
full128K/parity/three-edition/B2criterion6 and human provenance gates remain open.
Both required System1/ROM boot directions remain untested. X-2..X-5 remain future
work; no loaded-image authority, session substitution, boot binding or new trial.
