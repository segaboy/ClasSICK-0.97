# IMPL-0030 - Original typed register patch executor

2026-10-13; SPEC-0027 / ADR-0031 / TEST-0035 / SRC-0121/0122/0123.
Codex-assisted original implementation; lead self-review, human provenance
review pending. Published interface prose and our earlier original contracts
are inputs. No manual sample instructions, Apple/Microsoft implementation,
external executable, emulator source or copied assets are used.

personality/system1/guest-patches.c/.h resolves owned built-in and guest-code
entries, preflights collapsed A-lines and invokes guest patches through a new
trusted step variant of core/cpu/m68k-call.c/.h. Register signatures retain
explicit caller controls, depth and ancestor budgets. Matching saved-original
memory veneers validate their complete return before invoking the manager.
The trap wrapper supplies A1/D1/D2/A0 saves and D0 condition testing separately
from direct calls. A new read-only memory binding check supports initialization.

Pre-entry refusals preserve state. Late faults restore the caller frame while
retaining completed guest/manager/directory effects; reports expose entry and
nested service failure. The legacy atomic cs_s1_step path still refuses guest
targets. Explicit field copies avoid compiler-generated runtime helper imports.
Truthful caller-owned objects/maps, immutable active configuration and original
bounded metadata replace snapshots or inferred historical internal structures.

Original apps/guest-patch-probe assembly/bytes and tests/guest-patches.c cover
all eleven memory chains and selected nested/error behavior. CMake, compiler
verifiers and pinned CI provide hosted, freestanding and 68000 audits. These
are OS/core conformance tests, not native applications or an SDK. Get/Set direct
veneers, arbitrary direct guest calls, Pascal marshalling, actual manager
callbacks, CODE launch, QuickDraw and full CPU remain later work. No boot-entry
binding or VM/emulator/reference run. Six research drafts remain unchanged.
