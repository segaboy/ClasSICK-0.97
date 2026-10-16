# Native Macintosh and x86 parity plan

Current owner authority2026-10-16: SRC-0217/0218 and
[ADR-0043](../adr/0043-coupled-native-mac-pc-parity.md).
The former blanket wait for a complete Mac experience no longer governs native
PC advancement or original native parity workloads. This is a scope correction,
not a new code/build/test/boot result.

## Workstreams and completion

1. Native Macintosh: compile the portable OS/managers for68000 and supply the
   Mac hardware/ROM/ABI boundaries needed for the complete Mac experience.
2. Native PC: compile the same semantic implementation forx86 and supply PC
   hardware/ABI boundaries for each matching advancement. The OS executes
   natively; it must not depend on interpreting68k code to supply OS features.
3. PC legacy applications: interpret original-compatible68k application code
   through documented guest boundaries into the same native managers. Maintain
   this alongside the native PC path; it is not the PC OS implementation.

Each feature record identifies the native Mac path, native PC path, applicable
legacy68k path, expected behavior and tested gaps. Record compilation, hosted
conformance, actual target observations and physical/historical qualification
separately. An interpreted fixture or equal screenshot alone does not establish
native workload/API/input parity. A shared milestone stays incomplete if one
native side is missing. Exceptions need an explicit owner decision.

Use one portable C implementation for shared behavior and original workloads
where practical; compile it for both CPUs. Assembly is not required for ordinary
portable features. Native application ABI/SDK/container work is developed when
its bounded feature needs it, with reviewed contracts; no whole-Mac-completion
prerequisite and no implied claim of an already working general native loader.
Existing transparent-variant and licensing requirements remain unchanged.

## Current state and gaps

| Capability | Macintosh path | PC path | Disposition |
| --- | --- | --- | --- |
| Branding/layout renderer | Native68000 C | Native x86 C | Shared512x342 renderer; hosted exact-pixel parity and current boot captures recorded |
| Fixed-port PaintRect/EraseRect semantics | Native C manager | Native C manager | Selected bounded API profile; not general QuickDraw |
| Existing original40-byte application fixture | Guarded68k interpreter prototype | Guarded68k interpreter prototype | Compatibility/runtime coverage; not native application-execution parity |
| Original native drawing workload using those APIs | Paired native workload to add | Paired native workload to add | Next bounded parity slice; do not claim implemented |
| Scene input | Mac mouse backend | PC PS/2 keyboard counter | PC pointer/mouse-event parity missing; current image input reliability unqualified |
| Full windows/menus/events/files/System disk experience | Incomplete | Incomplete | Develop each advancement on both native sides |

The renderer is already native on both targets. The current PC demo still uses
the68k application fixture to produce its drawing result. Keep that fixture as
a compatibility test and add an original native C drawing workload compiled for
both CPUs, calling the same selected drawing service, with expected pixels,
result/error behavior and surface preservation checked against an independent
oracle. Run it directly through native service calls, without the interpreter.
This proves a bounded native service/workload path, not a general native app ABI
or external executable loader. Preserve and compare the separate68k fixture.
This is the next implementation slice to take before expanding drawing further.
PC mouse/event parity is a separately identified follow-up, not satisfied by a
keyboard counter or visual scene equality.

No implementation/fixtures/generated native app interface or new trial is part
of this documentation correction. Ordinary scoped implementation follows the
paired plan. Actual emulator/VM/reference trials still need fresh owner go-ahead.
Keep24 originals/copies, UART bodies, published assets/tags and all prior result
records unchanged. The standing canvas and both System1/ROM targets persist.
