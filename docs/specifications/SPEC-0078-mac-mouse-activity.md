# SPEC-0078 - Passive Mac mouse activity record

2026-10-28. Original engineering diagnostic, not a historical hardware contract.
Owner resumes release work (SRC-0521); reviewed original worker input/output and
lead integration are recorded in the canonical release-resumption receipt.
This does not alter input admission, loss recovery, SCC/VIA programming or cadence.

## Record and caller

MWA1 v1 is 80 bytes: twenty consecutive uint32 fields in activity.h. The native
68000 record is big-endian. Tag4D574131, version1 and length80 identify it; old
MWP1/MWR1 layouts and readers remain unchanged and cannot decode it as those formats.
Caller-owned state is serialized. No allocation, globals, callbacks, device access
or implicit timing. Begin initializes fresh state without reading prior contents.
Pair rejects bad pointer/header/raw bounds/flags before effects. Return0 success,
1 argument,2 state. Caller owns the other valid state fields and physical addresses.

Raw PB/RR0A/RR0B packs into24bits, with PB in the upper byte. Endpoint fields use
bit31 as VALID; bits24..30 remain zero. Previous *observed* and previous *accepted*
endpoints are different. The full actual accepted PB is retained, never rebuilt
from an incomplete phase description. HAD_PHASE is the decoder state before the
step, including before a Gray-loss reset.

Flags: STABLE1, ACCEPTED2, QUEUED4, MOTION8, LOST16, HAD_PHASE32. Queued requires
accepted; motion requires queued; accepted requires stable. Loss excludes these
three outcome flags. Reason1..6 accompanies loss only; other outcomes use0.
Stable is the existing masked pair-agreement result, not proof of atomicity,
physical coherence or missed-edge completeness. Accepted includes baseline/empty
endpoint advancement; queued means the inbox actually admitted a sample. Motion
means its dx or dy was nonzero. The observer records caller outcomes, not a second
decoder or a historical account of real edges.

Each observed pair increments pairs. Comparing successive observed second triples
counts PB4, PB5, RR0A DCD, RR0B DCD and PB3 changes separately. First observation
has no prior comparison. Stable-pair, nonzero-queued, zero-motion-queued and loss
counters follow the flags; all counters saturate. These are observations, not
physical pulse counts. Every call stores last observed; accepted updates previous
accepted; loss clears it. The first loss retains prior accepted (only if HAD_PHASE),
both current triples and the post-increment pair index; reason is written after
the other first-loss fields. Later losses retain the first loss unchanged.

## Opt-in controller and native checkpoint

CLASSICK_MAC_TIMER_ACTIVITY_PROBE is0 by default and accepts only0/1. Its enabled
controller adds80 bytes and observes already-read triples after the actual outcome;
it performs no additional IO. Default controller layout/behavior remains unchanged.
The native profile requires timer polling and the opt-in checkpoint switch. A
separate80-byte global snapshot is copied under IPL7 immediately before that
separate MAR1 checkpoint, after at least500 services plus a queued nonzero sample or loss, or
at50000 services as fallback. These are service counts, not wall time promises.
First activity need not have been caused by the intended host gesture.

MAR1 uses a separately audited74-byte original leaf: IPL7, own-record loads and
a self-branch at entry+72, no stack, MMIO or memory writes. D0=4D415231;
D1..D7=pairs,PB4,PB5,DCDa,DCDb,button changes,nonzero queued;
A0..A6=button-only queued,losses,first previous,s0,s1,reason,service.
A6 is loaded last. The old MWR1 leaf is excluded only in this new profile.
The MAR1 reader requires the verified spin and distinguishes this subset from
the full80-byte MWA1 record; omitted fields are never fabricated.

Only the known original checkpoint/record is inspected. A paused arbitrary
instruction is not a coherent snapshot. Native twins, default byte identity,
record bounds, source hashes, IRQ veneers and storage/stack costs precede a trial.
Instrumentation can perturb timing; it neither repairs the sampler nor proves a
root cause. Hosted models, native compilation and actual boot/input are separate.

## Verification and remaining gates

TEST-0099 covers wire/refusal/first-loss/saturation cases, existing fake-IO ordering,
positive/negative motion versus button-only admission, strict independent reader
refusals, native profile controls and actual observations when performed. Proposed
cases are not credited as passing. IMPL-0086 names the reviewed original module
and integration. No new manual/historical source or copied sample code is used.

Mac and PC visible movement/drag/release qualification remains required on the
exact candidates. Original/native-PC parity, physical/full128K, source/role,
edition, B2criterion6, native-app and both System1/ROM compatibility gates persist.
No release publication is authorized by this diagnostic contract.
