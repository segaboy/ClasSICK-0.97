# Native Macintosh and x86 parity plan

2026-10-17: SPEC-0043 implements pure shared native68000/x86 drawing-port selection plans; no guest memory access or trap binding. 810 local CTests,10 width/endian object audits and8 broken-core controls pass; CI passes3013 CTests across11 profiles and six68000 objects (no target execution). Existing scenes/workloads/ROM/EFI inputs unchanged. [Checkpoint](native-selection-evidence.md).

2026-10-17: SPEC-0041 adds a borrowed native drawing view with coordinate translation and rectangular clipping. The same68000/x86 workload exercises both while preserving every scene pixel and the independent old68k fixture.804 local tests,70 object audits and unloaded EFI controls pass; full CI and native68000 cross-build pass. New candidates are unbooted; no target trial or release.

[Drawing context checkpoint](drawing-context-evidence.md).

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
| Native coordinate view and rectangular clip | Same original view in native68000 workload | Same view in native x86 workload | SPEC-0041; exact old image retained, compiled candidates require fresh boot observations |
| Scene input | Shared native button core over existing Mac backend | Same core in Windows viewer and native x64 controller profile | SPEC-0040 native PC boot/direct keyboard+mouse API observed; desktop/human input and full pointer/event parity unqualified |
| Full windows/menus/events/files/System disk experience | Incomplete | Incomplete | Develop each advancement on both native sides |

The renderer and new original drawing workload now execute as native C on both
targets under [SPEC-0038 evidence](native-drawing-evidence.md). The separate
guarded40-byte68k fixture still supplies compatibility/runtime coverage. The
new native Mac profile remains a compiled candidate. The earlier native PC mouse candidate
has a scoped [boot/device-input observation](pc-mouse-boot-trial.md); the current
drawing-view candidates require fresh boot observations. Desktop/human input
delivery remains unqualified. No general external native application ABI/loader is claimed.

SPEC-0039 supplies shared button semantics, native hosted bindings and pure report
decoding. SPEC-0040 now implements single-owner8042 startup/routing and scene integration
in an explicit native x64 profile. One approved native PC boot now passes direct
Space and mouse API edge/held/release checks with static pixels preserved. The
session is closed. Full pointer motion/events, human input, the current native
Mac profile boot and broader target qualification remain open; synthetic device
events cannot close them.
Actual target runs need fresh owner go-ahead. Keep24 originals/copies, UART
bodies, releases, previous results and both System1/ROM requirements intact.

2026-10-17: SPEC-0042 / ADR-0048 finalize the prepared guest GetPort/SetPort selection contract. Same native semantic core planned for68000/x86; exact A5 anchor/current-cell ownership and total-commit rules specified. No implementation, tests, registry binding, build or target run in this slice. Existing drawing candidates unchanged/unbooted; full InitGraf and selected-port drawing adapter remain prerequisites. [Lead review](guest-port-lead-review.md).
