# SPEC-0072 - Original Mac window producer and binding

2026-10-23; SRC-0402/0408..0411 / ADR-0083 / IMPL-0074 / TEST-0087.

The owner explicitly approves original SCC/VIA implementation from independently
reviewed published interface facts, paraphrased reviewed summaries for fresh
supervised workers with manual pages lead-only, and isolated automated Snow/
VirtualBox trials of verified original candidates needed for this release.
Publication, reference systems and physical hardware remain separate. Excluded
cached hardware-summary definitions remain excluded. No sample code or private
heap/OS algorithm is admitted. Older held checkpoints are historical.

## Portable producer

motion.h/.c inject complete byte SCC-register transactions and one VIA portB
read callback. The target supplies guards, whole-chip ownership and a single
non-nesting producer. Caller objects are live/aligned/disjoint and fresh at
prepare; this is not hostile-pointer validation. The bounded descriptor owns
no canvas, allocator, owner/render callback or MMIO address.

Prepare requires a valid nonzero-epoch WAIT inbox, copies IO, resets SCC,
keeps MIE off, configures only external DCD interrupts, and writes distinct
WR12 markers to both channels before reading both back. Failure retains
first stage/cause with master disabled. Readback rejects an aliased control
port; it does not establish board channel assignment. Arm enables MIE last
only for matching WAIT. No fabricated baseline or automatic rearm.

Each service is one finite pass: read A RR3; reject unsupported pending bits;
read PB/RR0A/RR0B twice; acknowledge pending external latches and highest IUS;
read a third snapshot. Compare PB bits3..5 and RR0 DCD bit3. WAIT plus unstable
snapshots returns EMPTY and stays WAIT. Stable WAIT establishes one baseline.
LOSS drops observations until foreground discard/resume; never replay.
PRIMING/READY require remembered phases and exact agreement between changed
axes and RR3 pending axes, otherwise mark inbox uncertainty. Hidden even
edges and cross-chip coherence cannot be proven by these stable reads.

Apply explicit per-axis raw-DCD inversion bits. For a changed X phase, right
is normalized X1 equal to PB4; for changed Y, down is normalized Y1 unequal
to PB5. PB3 low is left-down. One stable change yields one record containing
both axes and current button; it is an original sample definition, not proof
of physical chronology. Silence yields EMPTY, no VBL queue churn. Overflow
retains LOSS and clears phase validity. Invalid epoch/terminal inbox or
unsupported pending status disables MIE and fails. Diagnostics saturate.

Stop, while quiescent, disables MIE and both external masks, then STOPPED;
repeat does no IO. It does not stop the inbox/helper/owner. A new lifetime
requires a fresh controller and matching new inbox epoch.

## Hardware facts and original profile

SRC-0080 III17/25/27..29 documents clock7.8336MHz, SCC even-byte reads/
odd-byte writes, >=2.2us gap, PCLK3.672MHz, DCD A/B first mouse phases,
PB4/5 second phases and PB3 active-low button. SRC-0408 Figure11.1 confirms
WR0 highest-IUS opcode111 (0x38), external reset010 (0x10), against the old
SRC-0363 prose/diagram disagreement. Figures10.1/10.2 identify RR0DCD,
RR3A external bits and readable RR12. No manufacturer's ISR recipe is used.
SRC-0409 Table3-1 p92 corroborates VIA/SCC/transient-combined levels1/2/3,
autovectors0x19/1A/1B, locations0x64/68/6C. Later publication is hardware
interface evidence, not proof of January1984 OS behavior.

Exact SCC channel/control offsets and raw DCD voltage sense remain unverified.
An original lowest-alias hypothesis within the published read/write windows
may be tested with our ROM: readB900000/readA900002; writeB B00001/writeA
B00003. This is an explicit engineering hypothesis, not a copied equate or
established physical mapping. Distinct-channel readback must gate arming.
Own-candidate emulator observations cannot close physical validation.

Three original GNU veneers raise IPL7 first, save D0-D7/A0-A6, pass source1/2/3,
restore registers and RTE. Before the first mask instruction higher IRQ can
enter, but no producer state has been touched. Level7/exceptions/DMA remain
outside exclusion. The native binding must install vectors while quiescent,
guard every chip byte (including selectors/reset), acknowledge VBL by the
existing portA read, keep one portB/SCC owner, and render only in unmasked
foreground. Native compile/build/target observations are recorded separately;
no physical latency, full128K OS or System1 compatibility follows.

## Opt-in native binding

Build-LoadedAppROM.py --window-drawing links only the shared native window
closure and original platform/runtime code. Existing modes remain separate.
window-entry.c owns the graph in the existing NOLOAD workspace, controller and
MWP1 diagnostics in bounded BSS. It checks the ROM seal and original trap probe,
initializes the graph with producers off, installs vectors, arms SCC and CA1,
then lowers IPL. Each SCC byte is followed by16 original MC68000 NOPs:
64 clocks, at least8us at CPU<=8MHz, covering the documented gap/reset recovery.
This lower bound does not prove maximum ISR latency or physical operation.

Interrupts acknowledge CA1 by portA read and call one controller service;
no render/owner/helper call occurs there. The foreground step is unmasked.
Fatal IRQ quiesces SCC/VIA; foreground closes the helper outside exclusion.
Trace snapshots and terminal checks use the exclusion pair. No competing old
poll loop, no extra framebuffer, no guest application or trap-dispatch code.

The builder checks exact linked veneer opcodes (IPL7, all15 saved registers,
correct source argument, exact handler call, balanced pop/restore, RTE), existing
ROM seal/eight corruption controls and deterministic twins. Sum of all bounded
nonrecursive C frames plus256B interrupt/assembly margin must fit24320B stack
reserve. This is a conservative static budget, not measured worst-case timing
or complete128K System1 certification. Source hashes and actual native RAM/ROM
sizes must be reviewed before any boot trial.
