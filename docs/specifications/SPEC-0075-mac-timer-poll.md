# SPEC-0075 - Separate timer-driven Mac input experiment

Status:original engineering experiment,not qualified hardware behavior.
SRC-0482..0484;IMPL-0083/TEST-0096. SPEC-0072 normal driver remains available.

The controller uses SPEC-0074's phase model and the existing ordered inbox.
It borrows live/aligned/nonoverlapping trusted objects/callbacks and exclusive
SCC/VIA T1/ACR ownership. Prepare/arm/stop and serialized ISR service require
IPL7;foreground inbox operations retain existing short exclusion. No rendering,
owner calls,allocation or arbitrary callbacks beyond the copied IO interface.
No hostile-pointer or lock-free claim. The32-bit controller is bounded320B;
native entry adds its128B trace under448B combined allowance.

Prepare rejects null/missing callbacks,invert>3,reload outside1..65535,or inbox
not valid/WAIT/nonzero epoch,without effects. Copies both IO structs before
fresh initialization. Masks T1 and acknowledges it,then resets SCC with master
off. Both distinct A/B DCD mask writes precede both alias readbacks (8/0).
Then disables both DCD masks and verifies zero,so the chosen published interface
reports live DCD levels. This is an original configuration strategy,not a
manufacturer or Apple initialization routine. No fallback for failed readback.
ACR0x40 gives continuous T1 without PB7 output;writes low/high counter reload,
checks ACR and both latches. Reload4096 is an engineering count,not a frequency.
No IER read or reg7 write. Reached stage is recorded before each IO group.
Hard failure retains reached state,cause,FAILED and T1/SCC-master-off cleanup.

Arm requires prepared/same-epoch WAIT,otherwise no-effect STATE. Acknowledges
T1 then enables only T1 via IER0xC0. CA1 is handled separately by the existing
native entry. SCC master remains disabled for this entire profile.

Service refuses invalid lifecycle/epoch without IO. Empty T1 flag returnsEMPTY
without phase reads/baseline/service increment. Actual T1 is acknowledged and
services saturates. Sticky inboxLOSS clears phase history and skips phase reads
until foreground cancellation/discard/resume yieldsWAIT. Other admitted phases
are PRIMING/READY;unexpected phase hard-faults. WAIT forces fresh decoder baseline.
Takes two PB/RR0A/RR0B triples;compares only PB3..5 and both DCD bits. Unstable
pair loses whole input;otherwise a recurring T1 flag loses whole input as overrun.
UNSTABLE has precedence when both occur. No partial motion/button admission.

Stable endpoints feed SPEC-0074 exactly once. BASELINED requiresWAIT and queues
only existing inbox baseline;SAMPLE queues one ordered record,never coalesces.
EMPTY changes no queue. Model Gray loss,unstable/overrun or queue overflow calls
existing uncertainty once,clears both phase histories and increments loss count.
Timer stays armed through visible loss so foreground can resume. Model refusal
or unexpected inbox result hard-faults. Samples counts successfully queued sample
records;peak includes baseline. All counters saturate. Stop masks/acknowledges T1,
disables SCC master and channel WR1/WR15,then STOPPED;repeat is no-effect OK.
Owner/inbox stop cancellation remains foreground work.

First-loss reason written last and retained. Snapshot0/1 are the two triples,
snapshot2=0,RR3=0 because not read. Previous stores prior have/raw-DCD/button.
Reasons5/6 add timer recurrence/Gray loss to existing instability/overflow reasons.
MWP1/MWR1 diagnostics remain original project data;services here counts actual
T1 flags,not SCC/VBL service calls. The idle control stops after500 such services,
not a wall-time promise or a usable human-motion window.

The opt-in builder permits timer alone or timer+idle and refuses combinations
with SCC defer/phase/first-input experiments. Default macro0 retains the existing
driver path. Source-level binding is present;native image proof and default byte
identity must pass before a target trial. No target trial has occurred here.
Endpoint checks cannot detect missed full/three-step cycles,prove coherent
cross-chip observation,clock rate,latency,physical128K fit or January1984 behavior.
The experiment is not an all-edge capture method or a release qualification.
