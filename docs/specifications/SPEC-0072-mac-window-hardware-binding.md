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
distinct DCD masks (A WR15=8, B WR15=0) before reading A RR15=8 and B
RR15=0. Only after both pass, enable B WR15=8, still with MIE off. Failure
retains first stage/cause with master disabled. Readback rejects aliased
controls; it does not establish physical channel assignment. No baud generator
access or unsupported-register fallback occurs. Arm enables MIE last
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
RR3A external bits and RR15 DCD interrupt-enable readback fields. No manufacturer's ISR recipe is used.
SRC-0409 Table3-1 p92 corroborates VIA/SCC/transient-combined levels1/2/3,
autovectors0x19/1A/1B, locations0x64/68/6C. Later publication is hardware
interface evidence, not proof of January1984 OS behavior.

Exact SCC channel/control offsets and raw DCD voltage sense remain unverified.
The original lowest-alias hypothesis (readB900000/readA900002; writeB B00001/
writeA B00003) failed in the approved Snow trial: unimplemented-access warnings,
no input and an own-ROM terminal spin. Keep that candidate as negative evidence.
The next bounded engineering hypothesis uses the top eight-byte cell of each
same published window: readB9FFFF8/readA9FFFFA; writeB BFFFF9/writeA BFFFFB.
It retains the same inferred select bits and read-even/write-odd rule. This is
an original experimental choice, not a copied equate or established physical
mapping. No address search, fallback or silent retry occurs in the ROM.
Distinct-channel readback still gates arming; actual native build and target
observations are required. Own-candidate emulator observations cannot close
physical validation. See SRC-0416..0418 and the boot/input follow-up receipt.

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


## First-loss observation (engineering, SRC-0424)

Append28B to the motion state: reason, raw RR3, three packed snapshots
(PB<<16|RR0A<<8|RR0B), previous(have_phase<<24|last_raw_a<<16|last_raw_b<<8|last_buttons),
and entry inbox phase. NONE0/NO_PHASE1/UNSTABLE2/PENDING3/OVERFLOW4 follow existing
decision precedence. Record only the first accepted transition to loss, reason
last, using existing sampled bytes and exclusion. No new MMIO/callback/allocation
or admission-policy change. Retain through recovery/stop; fresh prepare clears.
Unstable WAIT/already-LOSS/hard fault do not create a record. MWP1 v2 appends
these28B after its unchanged100B prefix; separate diagnostics unchanged. Existing
192B motion/320B combined bound remains; native sizes/stack must reverify.


## Loss quiescence (engineering, SRC-0427)

A valid LOSS transition disables whole-chip SCC master once (WR9=02), retaining
a quiesced flag. Already-LOSS service does so once for externally originated loss
without inventing a first-loss record. VIA remains independently available for
finite service/baseline work while foreground cancel/discard/resume proceeds.
Keep master off during unstable WAIT. Only a successfully queued stable fresh
baseline reenables master (WR9=0A) last and clears the flag. Initial arm and
FAILED/STOPPED refusal remain; no extra sample read or relaxed phase/pending/
overflow policy. Native sizes/stack and actual recovery must reverify. This
changes loss interrupt gating deliberately; the prior observation-only slice
remains separately identified. Compact samples are passed by pointer, avoiding
the observed odd-address native aggregate copy.


2026-10-24,SRC-0434/0435 audit adds no behavior: RR0 selector skip already exists,strict sampling/loss checks retained. Reused-ROM explicit button comparison confirms one press/two records,zero accepted displacement;UI pause can contaminate an idle observation. An automatic idle checkpoint is a future proposed slice,not implemented here. [Audit](../development/mac-motion-audit-evidence.md).

## Automatic idle checkpoint (engineering diagnostic, SRC-0436)

Optional `--window-idle-probe` requires `--window-drawing`. Normal builds
exclude its call and symbol. With `CLASSICK_MAC_IDLE_PROBE=1`, the existing
foreground observer copies the trace with IPL7 held, then, only on its
nonterminal path and at motion service count >=500, enters the original
`cs_mac_window_idle_checkpoint` self-branch. Do not restore the exclusion
token or perform additional MMIO, recovery, input delivery or allocations.
The foreground may observe a count above500; this is not a clock promise.

This is a frozen diagnostic, not a session STOPPED or input qualification.
Retain the128B MWP1 v2 layout,304B total native BSS and964B session workspace.
Require unchanged default ROM bytes, exact own checkpoint bytes60FE, existing
IRQ audits and a resealed checkpoint-corruption refusal. A trial must check
the known own PC and IPL7 before interpreting the frozen original RAM.
Do not touch the guest window before the automatic checkpoint. Inspecting
the frozen record afterwards cannot create earlier guest input. No historical,
hardware, motion, release or other gate is advanced by a diagnostic halt.


## Opt-in finite pending-edge experiment (engineering,SRC-0449)

CLASSICK_MAC_PENDING_DEFER_PROBE defaults0 and rejects values outside0/1.
Default controller layout and acceptance policy remain. Flag1 adds two32-bit
fields after first_loss. READY plus remembered phase and unquiesced master may
defer one nonstable RR3zero pass only when s1 DCDs match old phase,exactly one
final DCD differs,the other DCD/direction and button stay steady,and the moving
direction bit changes at most once. Keep old phase/buttons,queue and counters
except services;store masked PB/RR0 endpoint and single expected pending bit.

The very next service cannot defer again. It must remain READY/have_phase,
have three stable samples at that endpoint and exact matching RR3 bit,then
pass the unchanged phase/pending admission path. Otherwise clear state and
retain uncertainty/quiescence (UNSTABLE when nonstable,otherwise PENDING).
Original invalid/epoch/unsupported-pending hard-fault precedence remains.
Clear on WAIT,LOSS,hard fail,stop and fresh prepare. PRIMING cannot defer.

No extra IO or unstable record acceptance,coalescing,timestamp guarantee,
hidden-even-edge recovery or physical chronology. This is a new experimental
policy,not a diagnosis of the earlier ambiguous trace. Native builder option
--window-pending-defer-probe requires --window-drawing and refuses idle-probe
combination. Distinct profile name;normal byte identity,expected+8B BSS,current
source/IRQ/stack proofs and exact target qualification are mandatory before
any release adoption. Host tests pass;native proof/trial pending at this record.


## First-input record checkpoint (engineering diagnostic,SRC-0451)

--window-first-input-probe requires the window profile and refuses idle-probe;
it may combine with pending-defer under a distinct profile name. It sets the
validated FIRST_INPUT macro and the existing checkpoint-support macro. After
the ordinary complete trace copy,only on the nonterminal observer path with
IPL7 held,freeze if deliveries!=0 OR losses!=0 OR services>=5000. Keep the
token held and enter the existing60FE own diagnostic spin;do not perform
additional MMIO,delivery,recovery or allocations. No semantic STOPPED claim.
The linked noinline gate must survive GC only in this mode. Header predicate
tests do not replace native flag/link/spin verification. No new trace/RAM.

This bounds by observed service count,not elapsed time or physical events.
Require exact own PC/IPL7 and full original-record guards in a later trial.
A first delivered sample may be a button;unread prefix/first loss remain
visible. Default and finite-defer baseline images must remain byte-identical
when the observation flag is off. Native cost and target behavior must be
recorded;host tests alone do not qualify input. [Evidence](../development/mac-first-input-probe-evidence.md).
