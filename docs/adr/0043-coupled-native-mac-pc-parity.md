# ADR-0043 - Coupled native Macintosh and PC advancement

Accepted2026-10-16; owner SRC-0217, lead reconciliation SRC-0218.
Clarifies ADR-0001/0007 and supersedes the blanket Mac-completion prerequisite
in ADR-0026, SRC-0099's recorded gate and later statements carrying that gate.
Their original dates, source quotations and implementation results remain historical records.

The Mac goal is a complete native68000 System1 experience. The PC goal is the
same experience implemented as a native x86 OS. Advance both together: each OS
feature needs corresponding native implementations/adapters, original workload
coverage and a recorded comparison before its parity milestone is complete.
Use one portable semantic implementation where possible, compiling it for each
CPU; assembly belongs to necessary architecture/platform boundaries.

On PC the interpreter executes legacy68k application instructions. OS managers,
drawing, events, filesystems, drivers and desktop policy execute as native x86
code. A68k-only service or test workload is not a substitute for a native PC
feature/workload path. Preserve legacy application compatibility as a separate
required lane exercising the same manager semantics.

Native PC counterparts and original native parity workloads may be designed,
built and tested alongside Macintosh work now. Completing the whole Mac
experience is not their prerequisite. This does not declare a general native
application ABI/SDK/loader implemented or select an unreviewed image format;
each needed interface/loading feature still receives its bounded contract and
ordinary validation. No arbitrary third-party program or reference run is admitted.

Track native Mac, native PC and PC legacy68k compatibility separately. A feature
can be implemented incrementally, but missing PC behavior remains an open parity
item rather than a completed shared milestone. Only the owner can accept a
scope exception. [Current gaps and next slice](../development/native-parity-plan.md).

Standing512x342 PC canvas, monochrome branding, System1/ROM interoperability,
source/UART/physical/full128K/edition/B2/human requirements remain. No current
binary/tag/release changes or new emulator/VM/reference run follows from this
decision. Existing releases and observations keep their exact conditions.
