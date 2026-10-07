# System 1 compatibility personality

Status: architecture plan; no historical API semantics have been implemented.

## Profiles

`system1-1984` will name an exact reference System/Finder/ROM/hardware combination
after legitimate sources and authorized observations establish the baseline.
The founding System 0.97 version statement is not yet an independently verified
source record. Later manual editions are candidates for interface descriptions,
not automatic evidence of January 1984 behavior.

`classick-extended` is an explicit opt-in profile. It may expose larger displays,
color, RAM, newer storage, or new APIs. Every difference has an extension contract;
it cannot silently change `system1-1984` results. Initial boot does not require a
fully instantiated System 1 personality.

## Manager responsibilities and dependencies

| Component | Independent responsibility | Principal prerequisites |
| --- | --- | --- |
| Guest ABI/trap gateway | Decode trap variants and marshalled arguments/results; manage callbacks and error behavior | Guest address space, approved trap and calling-convention specs |
| Memory Manager | Guest heap, handles, relocation/purge/lock semantics where specified | Native arenas, guest mapping and observable memory specs |
| QuickDraw | Port/bitmap/region/pattern/text behavior from versioned contracts | Surfaces, clipping, original assets, guest ABI |
| Event Manager | Event queue, masks, tick/modifier translation, cooperative application service | Normalized input and clock |
| File Manager | Historical file calls, errors, metadata and fork semantics | VFS, block devices, MFS mount |
| Resource Manager | Type/ID lookup, attributes, fork handling and handle ownership | Resource-fork decoder, File and Memory Managers |
| Window / Menu / Control / Dialog | UI state, ownership, hit testing, ordering, drawing and event behavior | QuickDraw and Event/Memory Managers; resources as needed |
| TextEdit / Scrap | Text state/editing, clipboard contracts and encoding | Drawing, events, memory; independently created font assets |
| Segment/application services | CODE/segment loading, transitions and callbacks | Resource/File/Memory Managers, guest runtime, ABI |
| Desktop shell | Independently designed Finder-compatible useful workflows | Working UI, files, application services |

Native manager internals use explicit core types. Guest records are parsed and
serialized at the boundary with documented field widths and byte order. Historical
low-memory globals are a specified compatibility view, not the native kernel's
global layout. Pointers, handles, Pascal strings, register results, stack cleanup,
and callbacks must be specified per operation; one generic guessed trap ABI is
insufficient.

Version-specific contracts identify supported operations, malformed inputs,
ownership, state transitions, errors, and observable effects. Trap decoding and
semantic implementation are separate so native callers and translated callers
converge on the same behavior. Unsupported traps fail deterministically and appear
in the compatibility report; they do not route to Apple ROM fallbacks.

## Historical fidelity without copying assets

Use independent glyphs, icons, cursor art, and sounds. Record where typography or
desktop appearance intentionally differs. Pixel comparisons can test primitive
geometry with original synthetic scenes; do not redistribute original desktop
screens or resource assets as public golden images.

Compatibility is per tested behavior. Direct hardware access, private ROM entry
points, undocumented globals, self-modifying code, or timing-sensitive programs
may remain unsupported. Research findings can extend the specification only from
eligible evidence and within the clean-room policy.
