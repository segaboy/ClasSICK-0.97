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
SPEC-0023 now supplies eleven bounded pointer/handle/size/lock/copy services,
checked globals and a separate register adapter. Published interface review and
the original 32-object/fixed-zone engineering contract are finalized; historical
uncertainties remain explicit. [Memory Manager evidence](memory-manager-evidence.md)
records 1,245 local checks and both successful CI jobs with an exactly matching
68000 manifest. The registry contains thirteen guest contracts. Typed guest
trap binding and explicit CPU data-address normalization remain next. Most CPU
instructions, purging/growth/callbacks and application loading are unimplemented.
[Lead review](../research/memory-manager-lead-review.md) distinguishes the final
contract from Claude's preserved research proposals.

Gemini's [MFS research intake review](../research/mfs-gemini-lead-review.md)
checks the supplied proposals against published format diagrams. It finds
reversed lock bits, invalid fixture geometry and incomplete directory/empty-fork/
name rules. Test categories are useful; no parser contract or fixture is finalized,
and 12-bit packing remains unverified. Filename-comparison research is a separate
follow-up. The 2026-10-13 [filename review](../research/mfs-filenames-lead-review.md)
(SRC-0109) corrects the report's case flag and version/diacritic claims. Future
reader choices preserve counted raw names and versions, identify ASCII-only
comparison as incomplete, and refuse an ambiguous selected identity. No MFS API
or decoder is finalized. Guest-trap integration remains the next dependency.

2026-10-13: [Claude's resource-fork review](../research/resourcefork-lead-review.md)
adopts SPEC-0024 / ADR-0028 and settles L-1..12 for a tolerant bounded reader.
TEST-0032 is a conformance plan; no parser or audit has executed. The optional
full audit remains deferred with explicit scratch/work requirements. The
[asset register](../compatibility/assets.md) tracks future original content;
its [source review](../research/asset-replacement-lead-review.md) does not create
new legal approval gates. Existing compiled Memory Manager/native results and
the implementation order remain unchanged.

## Work allowed now, in dependency order

1. **Routine and record interface registry.** The two trap-address services and
   eleven phase-1 Memory Manager routines are now recorded after source/contract
   review, with thirteen records and eight rejection controls passing. Expand
   through further reviewed contracts. Per routine record trap word/number, parameter
   type/width/location, value/object/code class, procedure signature, result,
   result-code globals, preservation/stack cleanup, memory-movement status and
   profile availability. For guest records, record verified field widths,
   offsets/byte order and source locator. Unknown introduction release stays
   unknown; first documented edition is a separate field. Independently check
   68000 marshalling/guest parsing from the registry. No native headers/SDK.
2. **Original bounded Memory Manager.** SPEC-0023 finalizes pointer/handle/cell/
   zone representation over SPEC-0022, invalid/zero/resize/error behavior,
   checked guest globals and allocation-only SYS/CLEAR routing. Live-block
   nonoverlap, foreign values, locks, prefix preservation, failed-call atomicity
   and copy overlap pass original tests. Raw stale aliases remain indistinguishable
   after address reuse. Connect the separate register adapter through typed guest
   traps; add explicit 24-bit CPU data-address normalization before flagged-cell
   dereference fixtures. Current native metadata is 672 bytes and guest cell bands
   256; final linked 128K budgets remain open. No Apple heap/header/list or
   placement algorithm adopted. Purging/growth/callbacks need later contracts.
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
