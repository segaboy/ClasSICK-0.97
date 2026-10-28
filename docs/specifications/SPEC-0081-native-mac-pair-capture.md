# SPEC-0081 - Opt-in native paired-read capture

Original engineering contract,2026-10-28. Existing timer reload comparisons did
not queue movement;the256-count profile overruns before the intended gesture.
Reduce callback dispatch in acquisition without changing admission or register IO.
This is an experiment,not a claim that overhead is the proven cause.

CLASSICK_MAC_TIMER_PAIR_PROBE defaults0 and accepts0/1. At0,the motion IO layout
and compiled legacy profiles remain unchanged. At1,append one trusted callback
void pair(void *cookie,uint8_t out[6]) to cs_mac_motion_io. Prepare requires it
before any effects and copies it with the existing IO. Callback writes exactly
PB,RR0A,RR0B,PB,RR0A,RR0B in that order. No extra reset,acknowledgement,device
read,allocation or arbitrary callback. It is caller serialized with the same
IRQ/foreground exclusion and trusted-object requirements as existing IO.

Timer service invokes it once instead of six indirect read callbacks,then uses
the same recurring-T1 check,unstable/overrun/Gray precedence,decoder,inbox and
activity observer. No unstable data is newly admitted;no loss policy is relaxed.
The native implementation uses the existing original byte-read helpers and
constant register/channel arguments. Original volatile access order and sixteen
NOP recovery guards after each SCC byte access must be preserved. No new assembly.
Only this profile forces the needed helpers inline. Native proof rejects calls
inside the resulting pair routine and retains existing IRQ/MAR1 encoding audits.

--window-timer-pair-probe requires the recovery and refresh profiles. Existing
reload divisors remain available for matched comparison. Profile identity and
manifest record the callback/IO count and experimental status. Default hashes,
hosted callback/order/refusal controls,native source/import/IRQ/stack/RAM bounds
and exact identities precede a trial. Added function pointer and frame/ROM costs
are measured,not assumed. This does not qualify physical timing,coherence,latency,
128K fit,part identity or historical behavior. All release/source/role/input gates
remain. No reference/emulator implementation or manual sample is an input.
