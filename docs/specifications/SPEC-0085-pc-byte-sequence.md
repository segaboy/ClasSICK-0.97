# SPEC-0085 - PC delivered-byte sequence diagnostic

Lead-adopted original engineering contract,2026-10-30. No historical or hardware
interface is added. Reuses the reviewed original SPEC-0040/0058 bus and
SPEC-0073/0076/0079 observer family;SRC-0558 r4 contributes a proposed recorder.
This is diagnostic qualification for the paired release,not a decoder-policy fix.
The affected Mac/ROM/68000 contribution lane stays excluded.

## Recorder and composition

CLASSICK_PC_INPUT_PROBE_SEQUENCE defaults0 and is exactly0/1. When1,it is
exclusive with MOTION,SKIP_FIRST_PACKET,REPORT and RAW_PACKET_PROBE. Both
preprocessor and C constraints refuse invalid numeric/composed flags. Length is
1..48,default48. raw-sequence selects48;raw-sequence-first is a3-BYTE threshold,
not a guarantee of a complete report. Existing modes/default images stay intact.

cs_pc_input_sequence_begin initializes an explicit caller-owned recorder;
NULL returns1,success0. Only begin/observe write it;the session is const and
unchanged. NULL/unstarted context never stops. The nonreturning native loop
keeps this context alive on its stack. No global state or hardware operation.

Observe once before each poll. A READY poll consumes at most one controller
byte. Between consecutive READY observations,one AUX-counter increment identifies
last_byte as the next observed stream byte. Keyboard bytes do not increment AUX.
Initial observation only primes;the startup-completion byte is not recorded.
Non-READY/fault transitions prevent recording;first READY after them re-primes.
Do not reuse a context across an unobserved reset or counter wrap.

For step>1,record only the latest byte and add step-1 to a gap count,saturating255.
Flag a byte iff the aggregate bus.errors counter changed;this is NOT a controller
status bit or a retained loss reason. Saturating AUX/errors counters can stop
producing steps;no step is not proof of no input. A run that does not reach the
byte threshold does not halt;an external host-time cap/cleanup remains necessary.

At the threshold the existing audited nonreturning leaf loads eight64-bit words:
r8=4353505345513031 (CSPSEQ01);r9 holds error-step flags bits0..47,gaps48..55,
count56..63;r10..r15 hold bytes0..47,little-endian within each word. Unused bytes
and flags are0. Do not alter the halt leaf,session,bus,decoder or presentation.

## Reader and limits

Read-PCInputSequence.py exports parse_registers(text),unpack(words),replay,
summarize and decode(text),plus FILE CLI with optional --json. FormatError is the
typed refusal;CLI0 success,2 malformed record. Register text supports r8..r15,
case-insensitive,name=value/name:value/name value,hex with optional0x. Blank or
unrelated lines may be ignored;any recognizable malformed known-register line
must refuse. Refuse duplicate/missing registers,wrong magic,count outside1..48,
and nonzero flags/data past count. unpack accepts exactly eight uint64 integers
(not bool). Format acceptance cannot establish the exact source/own-code spin.

Replay is a MODEL of the original strict decoder,not a new native policy:
header bit3 at sync;two payload bytes;header sign bits4/5 give9-bit signed deltas;
overflow bits6/7 make the whole modeled report a loss;low3 bits are buttons.
Model state starts at sync. On an error-step,reset modeled partial state.

Events retain index,byte,error,kind (0 partial,1 modeled packet,2 modeled loss):
- No error-step:header,x,packet;an otherwise modeled drop is
  model-mismatch-drop-unsynced or model-mismatch-drop-overflow.
- Error-step where the model would drop:loss-unsynced-compatible or
  loss-overflow-compatible. A controller-status loss can coincide with these
  bytes;the record cannot distinguish that cause from native decoder rejection.
- Error-step where the model would accept:lost-status-compatible. This is a
  compatibility classification under the model/history assumptions,not retained
  proof of the original controller status or host cause.

Overflow-compatible events keep header,raw_x/raw_y and the two overflow flags.
Packet events keep dx/dy/buttons. Missing history/gaps can invalidate modeled
alignment;summary alignment_qualified is true only for gaps0. Even then,image
identity,counter/non-reset assumptions and error saturation are external limits.
All counts/sums/deltas are modeled values,never physical or GUI acceptance.

Summary fields:packets,overflow_compatible,unsynced_compatible,status_compatible,
model_mismatches,trailing_partial_bytes,gaps,alignment_qualified,sum_dx,sum_dy,
max_abs_dx/max_abs_dy,button_states,notes,scope. Never label compatible losses as
proven drop causes. No proportional-delivery,warp,recentring or forwarding cause
is inferred from packet size alone. With gaps,notes explicitly limit alignment.

## Qualification and trials

Reuse sound original r4 C/native wiring and its author tests after lead review;
counts supplied in the archive are not project passes. Qualify actual recorder,
real bus/keyboard/status/error paths,length3,reader/refusals,exclusive flags,gap
saturation and partial/counter limits. Freeze independent literal reader vectors
before candidate outputs. Old modes need actual .text/default native identity;
native modes need source-pinned twins/import/stack/seal/omission/media checks.
Do not run contribution mutation scripts on the shared checkout.

Read-only Windows capture-context work D0 still needs eligible published API
facts. Any trial uses admitted original image/platform,windowed readiness,actual
button mapping,owned-client guards,bounded cleanup and separate E2/E3/E4 labels.
48bytes are not necessarily16reports;a3-byte timeout does not prove zero input
or locate a fault. Distinct cold starts cannot establish each other's counts.
No deliberate physical stimulus is fabricated;no default/decoder relaxation.

No emulator/reference/hardware/release permission is created by this contract.
