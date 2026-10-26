# SPEC-0074 - Isolated polled-phase software model

Status:experimental engineering model;no hardware or boot binding.
SRC-0479/0480;IMPL-0082/TEST-0095. This does not replace SPEC-0072.

## Preconditions and representation

State is caller-owned,serialized and relocatable,with no retained pointers.
It contains ten uint32 fields in order:valid,invert,have_phase,x_phase,y_phase,
buttons,services,samples,baselines,losses (40 bytes). Output is the existing
12-byte cs_pointer_sample. Inputs and objects are valid,properly aligned C
objects in one flat numerical address model. Span checks do not validate
arbitrary native pointers or segmented aliases. No allocation,mutable global,
clock,IO,IRQ or owner call occurs. C99 is required.

The proposed normalization is Xphase=RR0A.DCD(bit3)+2*PB4,Yphase=RR0B.DCD(bit3)
+2*PB5;PB3 is active-low button. These reuse engineering interface assignments,
not newly verified physical pin or January1984 behavior. The future sampler
must independently establish usable live levels,coherence and sampling latency.

## Engineering transition rules

Forward Gray order is0,1,3,2,0;reverse reverses it. Only DCD-changing edges
emit a unit:X forward-1,Y forward+1. Invert bits0/1 reverse the corresponding
axis. PB-only transitions update history with no displacement. This matches
the current source direction/unit convention,not a historical mouse-gain claim.
An observed two-bit jump on either axis loses the entire sample:no other-axis
motion or button delivery. Missed full cycles are invisible;three missed steps
can resemble one reverse step. No timing/physical event-completeness guarantee.

begin rejects NULL/invert>3 unchanged,else initializes all fields without
reading old bytes and returns EMPTY. step checks NULL/raw>255,wrapped spans
and state/output overlap before state invariants;refusal changes no bytes.
State requires the POL1 marker,invert<=3,have<=1,phases<=3,buttons<=1.
Valid calls increment services saturating. The first call or call after loss
sets a baseline with zero displacement/current button and increments baselines.
Loss clears have_phase,sets zero displacement/button0 and increments losses.
Other valid calls commit phases/button and fully set output;SAMPLE iff there
is displacement or changed button,incrementing samples,else EMPTY. All counters
saturate independently. Return codes:SAMPLE0,EMPTY1,BASELINED2,LOST3,ARGUMENT4,STATE5.

BASELINED is not a press event. A future consumer must use baseline/cancel and
held-button suppression;it must not forward BASELINED or LOST as normal samples.
LOST requires whole-input cancellation/rebaseline before further delivery.

## Qualification boundary

This slice tests only the explicit software model. No SCC register settings,
IRQ ownership,live-level sampler,queue/foreground binding,poll cadence or
hardware compatibility is specified or implemented. C16 compilation does not
qualify far/segmented pointers or runtime stack policy. M68000 compilation is
pending;no target execution,whole-stack/128K or performance claim. Existing
strict driver,normal ROM/PC paths,native-app and release gates stay in force.
[Actual checks](../development/mac-polled-phase-model.md).
