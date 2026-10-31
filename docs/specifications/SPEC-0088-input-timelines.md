<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0088 - Bounded native input timelines

Original diagnostic-only engineering profile. Normal Mac and PC builds omit these observers. No hardware, emulator implementation, historical behavior, host-input acceptance or release claim follows. The existing controller, sample-admission and cancellation contracts remain unchanged.

## Mac interrupt profile

`CLASSICK_MAC_MOTION_TIMELINE=1`, selected by `--window-drawing --window-motion-timeline`, is exclusive of the timer and other diagnostic profiles. The six-entry MTL1 record is embedded in the original motion object and identified through the diagnostic's own exported address word. It records service count, RR3, three already-read PB/RR0A/RR0B snapshots, retained prior DCD/button state, inbox phase and the returned service result. It records only a nonzero pending indication, unstable triple or changed retained endpoint. A full record counts later eligible observations in a saturating overflow field; it does not retain them.

Each record's result begins as `UINT32_MAX` and is committed by the serialized interrupt caller after service returns. An omitted commit remains unknown. Record count is advanced only after the raw fields are written. Read only from our own paused diagnostic, verify the exact ROM and symbol-derived address, and reject a bad magic, count, pending or incomplete result. Pausing may interrupt a 32-bit update; a valid-looking read is not an atomicity or causality guarantee. Service counts are not timestamps. The recorder performs no extra MMIO, but its added interrupt work can perturb timing. It cannot reveal unobserved hardware transitions or attribute a record to a specific host action.

`Verify-SnowWorkspace.py` is a read-only preflight that requires the workspace's ROM path to equal the expected absolute path, verifies the expected 64 KiB file and SHA-256, and refuses nonempty floppy/SCSI media, auxiliary images, sharing or an unexpected model/mouse/audio profile. It refuses duplicate JSON keys. This checks the configuration and intended file; it does not prove the emulator actually loaded that file.

## PC timeline profile

The explicit window input probe mode `timeline` uses a caller-owned CSPTIM01 recorder and the existing original register-halt veneer. The observer runs after each successful native session poll and, in this diagnostic mode only, once after a failed poll. It never writes the session or performs hardware I/O. Consecutive bus READY observations allow an auxiliary-counter increase to record the last byte, decoder-used before/after, delivery kind, whether the aggregate error counter changed, and the session and owner phase/stage. A counter jump records only the last byte and increments a saturating gap count; counter regression or saturation marks the record unreliable. A new READY baseline does not claim startup bytes. A saturated error counter also marks it unreliable.

Stop after six entries, a post-READY drained keyboard record, or an observed session fault; full takes precedence over fault, which takes precedence over a marker. A marker is any drained record, not proof of a particular physical key. The existing halt veneer writes magic `CSPTIM01` into r8. r9 contains count bits 0..7, gaps 8..15, unreliable bit 16, READY-baseline bit 17, stop reason in bits 24..31 (1 full, 2 marker, 3 fault), final X bits 32..47 and Y bits 48..63. An out-of-range coordinate is `FFFF`. r10..r15 hold up to six entries as eight low-to-high bytes: raw byte, decoder used before, used after, delivery kind, error-counter change, session phase, owner phase, owner stage. Unused words are zero. A field too large for a byte is `FF` and marks the observation unreliable.

The reader validates the format only. Before interpreting data the operator must separately verify the exact own image, halted RIP in its own audited spin and magic. A zero-entry marker means no qualifying AUX increment was observed in the recorded interval; it is not proof that the physical device sent nothing. An unreported byte loss, a saturated counter, a failed poll that does not mark the session faulted, and events before READY remain outside the record. Neither these bytes nor their decoder states prove stream alignment or host origin.

## Validation

Hosted tests cover initialization, marker and full stops, loss/fault fields, startup boundary, byte order, gaps/saturation, incomplete results, bounded overflow, session immutability and output formatting. Exact native builds, unchanged default binaries and isolated target observations require their own recorded execution. Proposed cases are not execution evidence.
