# SPEC-0040 PC PS/2 mouse controller profile

Accepted2026-10-16; SRC-0233..0237 / ADR-0046 / IMPL-0044 / TEST-0048.
Original strict engineering profile; no universal controller or historical Mac claim.

## Preconditions (caller)
Live disjoint state/queue/callbacks, one CPU, interrupts masked, ACPI 8042 present, no USB legacy/SMM or other consumer, standard PS/2 mouse, buttons/keys released, no unsolicited startup motion.

## `cs_ps2_receive`
Pure READY-only routing of one byte already read: no I/O, no time. Null or OBF=0 -> ARGUMENT; keyboard not phase 34/READY -> STATE; both without effects. Otherwise sets `last_byte`; status&C0 -> errors++, held/prefix cleared, READY; AUX -> auxiliary++; else the existing scan. `cs_ps2_poll` READY routes through it with no semantic change.

## `cs_ps2_bus`
`begin`: initialize keyboard/decoder/left through their APIs, zero own counters/time/report, result ACTIVE and synced 1. `poll`: null args -> ARGUMENT without effects; phase>24, result>9 or synced>1 -> sticky STATE before I/O; terminal results return unchanged without I/O. An ACTIVE/READY phase/result mismatch and clock regression also refuse STATE before I/O.

Phase 0 delegates exactly one `cs_ps2_poll`; keyboard failures become terminal; keyboard READY -> phase 1, return ACTIVE. Phase numbers below are decimal; commands/replies/masks are hexadecimal:

| phase | action | phase | action |
|---|---|---|---|
| 1 | AD | 13 | wait AUX BAT AA |
| 2 | A7 | 14 | wait AUX ID 00 |
| 3 | drain <=64 | 15 | D4 |
| 4 | A9 | 16 | F6 |
| 5 | wait controller 00 | 17 | wait AUX FA |
| 6 | A8 | 18 | 20 |
| 7 | D4 | 19 | wait config, (b&73)==10 |
| 8 | F5 | 20 | D4 |
| 9 | wait AUX FA | 21 | F4 |
| 10 | D4 | 22 | wait AUX FA |
| 11 | FF | 23 | AE |
| 12 | wait AUX FA | 24 | READY |

Deadlines are checked before I/O: 357955 ticks (phase 13: 3579545) or 1000000 polls -> TIMEOUT. One status read per turn; with OBF set, at most one data read and no write. Writes need IBF=0. AUX phases never write D0/D1/AA or pulse commands; phase0 retains the qualified keyboard self-test/output-port checks.

Every non-drain AUX byte is counted, including ACK/BAT/ID and corrupted AUX bytes. Startup status&C0 -> errors++, CONTROLLER, no cleanup. Phases5/19 take only non-AUX; AUX waits take only AUX, routing other bytes to the keyboard. Unexpected AUX is dropped; unrelated traffic never restarts a deadline. In ACK waits FE retries the same byte with D4 (9->7,12->10,17->15,22->20), twice at most, then DEVICE. ACK-wait FC/FD/00/FF -> DEVICE, other bytes -> PROTOCOL; bad BAT/ID -> DEVICE; A9 nonzero or configuration-mask mismatch -> CONTROLLER. Failures are sticky; counters saturate independently of the64-byte drain budget.

READY: no writes. Non-AUX bytes go through `cs_ps2_receive` (QUEUE propagates). AUX bytes: auxiliary++; on an error status: errors++, decoder fed corrupt, left cancelled, synced=0. DROPPED: errors++, cancel, synced=0. PACKET: report stored, packets++. When unsynced, left.down is rebaselined with no press credited; otherwise it is sampled. FA/FE/AA count as payload. Limit: sync after unreported loss cannot be proven. Mouse data never enters the keyboard FIFO; there is no cursor UI.

## Native profile `CLASSICK_PC_MOUSE=1` (keyboard.c only)
Default0 retains the legacy contract/tested behavior and v1 112-byte trace. Profile1 waits for complete bus READY before drawing or timing. Its trace stays112 bytes, version2:12/16 bus result/phase;36 saturated Space+left presses (serial `space=` is a compatibility label for combined input);52/56/60/64 keyboard+bus sums of errors/AUX/drained/resends;104 last byte;108 last verified configuration before final AE, not current hardware state. Queue-pop faults also set bus.result=QUEUE. Qualification remainsv1/keyboard-only. Mouse changes the count without changing Space's offset. Variants never link together. The native UEFI ABI remains x64-only; portable bus tests cover both widths/byte orders.
