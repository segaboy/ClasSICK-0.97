# Native Macintosh and x86 parity plan

Current owner authority2026-10-16: SRC-0217/0218 and
[ADR-0043](../adr/0043-coupled-native-mac-pc-parity.md).
The former blanket wait for a complete Mac experience no longer governs native
PC advancement or original native parity workloads. This began as a scope correction. SPEC-0038 now implements the paired native
drawing workload; target boots and broader parity remain separate evidence.

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
| Original native drawing workload using those APIs | Direct native C ROM candidate | Direct native C PC/Windows binding | SPEC-0038 compiler/hosted conformance passes; new target boots pending |
| Scene input | Shared native button core over existing Mac backend | Same core in Windows viewer; pure PS/2 decoder ready | SPEC-0039 hosted foundations; freestanding PC controller routing and target input unqualified |
| Full windows/menus/events/files/System disk experience | Incomplete | Incomplete | Develop each advancement on both native sides |

The renderer and new original drawing workload now execute as native C on both
targets under [SPEC-0038 evidence](native-drawing-evidence.md). The separate
guarded40-byte68k fixture still supplies compatibility/runtime coverage. The
new native Mac profile and native PC image are compiled candidates, not new
boot observations. No general external native application ABI/loader is claimed.

SPEC-0039 supplies shared button semantics, native hosted bindings and pure report
decoding. Next paired advancement: reviewed single-owner8042 mouse startup/AUX
routing and corresponding scene integration. Full pointer motion/events and target
qualification remain open. Keyboard counters or hosted tests cannot close them.
Actual target runs need fresh owner go-ahead. Keep24 originals/copies, UART
bodies, releases, previous results and both System1/ROM requirements intact.
