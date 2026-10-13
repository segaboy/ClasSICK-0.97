# 68000 application runtime plan - 2026-10-12

Owner-governed plan, SRC-0099 / ADR-0026. This incorporates Claude's returned
Memory Manager, application-loading and native-application research without
treating research proposals as finalized historical specifications.

## Current foundation

SPEC-0020 supplies selected Get/SetTrapAddress services; SPEC-0021 supplies an
original bounded A-line path and native Mac exception return. SPEC-0022 supplies
checked data regions, selected MOVE/MOVEA and byte/word branches, BSR/RTS,
transactional failures and a new original twelve-step program. That program is
hosted/core verification, not an x86 application or a new boot-image trial.
Most CPU instructions, an allocator, callbacks and application loading remain
unimplemented. [Memory Manager lead review](../research/memory-manager-lead-review.md)
records proposed representation/resize/flag decisions; exact published citations
and historical uncertainties still need review before manager finalization.

## Work allowed now, in dependency order

1. **Routine and record interface registry.** Start with the two finalized
   trap-address services; add the eleven phase-1 Memory Manager routines only
   after source/contract review. Per routine record trap word/number, parameter
   type/width/location, value/object/code class, procedure signature, result,
   result-code globals, preservation/stack cleanup, memory-movement status and
   profile availability. For guest records, record verified field widths,
   offsets/byte order and source locator. Unknown introduction release stays
   unknown; first documented edition is a separate field. Independently check
   68000 marshalling/guest parsing from the registry. No native headers/SDK.
2. **Original bounded Memory Manager.** Finalize pointer/handle/cell/zone
   representation over SPEC-0022, explicit invalid/zero/resize/error behavior,
   checked guest globals and allocation-only SYS/CLEAR routing. Test live-block
   nonoverlap, stale/foreign values, locks, prefix preservation, failed-call
   atomicity and copy overlap. Add explicit 24-bit data-address normalization
   before flagged-cell dereference fixtures. Budget every side-table byte on
   the 128K target. No Apple heap/header/list or placement algorithm adopted.
3. **JSR/JMP and call-into-guest service.** Review published original-68000
   control-address modes; implement checked targets/pushes separately from data
   access. Then run a guest address under an explicit signature until return to
   an owned sentinel. Each nested call has its own budget and bounded depth;
   preserve/restore A5/A7 exactly on the specified return/error paths. Tests
   cover nesting, early failure, forged/bad return, stack exhaustion, budget
   exhaustion and incompatible signatures. This serves 68000 procedures and
   patches; it introduces no native application entries.
4. **Typed dispatch.** Specify built-in identity and guest-code entries with
   consistent Get/Set/direct-call logical addresses. Route an installed guest
   patch through the bounded service, preserving per-routine calling rules.
   Reserve unspecified extension space; no third native kind, native marker or
   native-entry registry is populated. Test saved original-address chaining,
   unsupported types, independent directories and errors without partial writes.
5. **CODE-only launch selection and format prerequisites.** Make selection a
   separate stage with one permitted outcome, 68000 CODE, plus explicit refusal
   diagnostics. First implement checked read-only resource/MFS contracts using
   wholly original generators and fixtures. Verify CODE0 sizes, A5-world/stack
   arithmetic and segment/jump-table requirements from eligible published
   sections; keep unresolved offset/layout/entry-state questions explicit.
   Launch/LoadSeg/UnloadSeg/ExitToShell, Toolbox stack marshalling and heap-reset
   behavior each need their own spec/tests. No native descriptor or image loader.
6. **Useful original 68000 application, then complete experience.** Execute an
   independently generated resource-fork application through managers and
   lifecycle services, expand the missing ISA/QuickDraw/events/windows/menus/
   file contracts, then use a named legally supplied original application only
   under an approved local protocol. Record results and unsupported operations
   per exact profile. Optional debug interrupt-context checking uses verified
   routine metadata; a flag alone cannot infer every interrupt context.

Each step follows specifications, original code/tests, provenance and evidence;
this plan is not authority to skip those gates. New VM/emulator execution waits
for the owner's go-ahead and exact image/platform admission.

## Closed native-application gate

No x86 application/native test program or native image loader is built, loaded
or run until the owner explicitly confirms the 68k experience complete. Own
native OS/core/tools and hosted contract tests remain permitted. No native
converter/packager, descriptor/type code, entry registry/marker, SDK/header or
chosen x86 application ABI implementation is prepared meanwhile. D3/D5-D7 and
ISA-tag/native format choices remain proposals; later-era orientation sources
must be reviewed/registered before any specification relies on them.

When the 68k experience is near completion, propose measurable acceptance
criteria for the owner's approval, including the supported manager/API matrix,
M0.6/M0.7 results, his chosen application's real-Mac-equivalent workflow,
document/file results and known differences. Only his confirmation opens the
gate; neither a passing build nor a milestone automatically does so.

Future authorized variants use the ordinary transparent experience: one file/
identity, no choice prompt, matching results, shared guest-visible state,
working patches and an intact 68000 path elsewhere (T-1 through T-6). Existing
GPL-3.0-or-later terms remain unchanged; the project does not decide others'
licences. Do not infer January1984 application support from later published
examples. Full OS/physical/parity/three-edition gates persist.
