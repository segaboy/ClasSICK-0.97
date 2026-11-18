<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0125 - Explicit modeled Macintosh sector writes

2026-11-18; IMPL-0155 / TEST-0216 / SRC-0619. Original engineering contract
extending the existing SPEC-0114 reader against project-owned BootHush's
explicit writable-copy model. No physical floppy timing or Apple implementation
is an input. Ordinary read-only media behavior remains unchanged.

`cs_mac_iwm_write_one` borrows a bound reader, immutable512-byte replacement
and512-byte scratch. All controls/buffers are truthful, live, stable, disjoint
native objects under exclusive controller ownership with interrupts masked.
It validates the logical block range, acquires the current sector through the
existing checksum/address-validated reader, preserves its12tag bytes, encodes
the replacement and transmits one complete record. No allocation or retained
new pointer is involved. The initial read places the drive at the required
track; no intervening controller owner may change that state.

The independently authored encoder reverses the published project model's
existing three-byte rolling checksum/GCR wire profile. It writes the706-byte
sector body: sector symbol,699payload symbols,4checksum symbols and DE AA.
The final two-byte group retains the implicit third zero byte's XOR-derived
high bits even though its low symbol is omitted. The read decoder alone does
not check those otherwise redundant bits; the writable model requires them.
Its input tags/block and output must be disjoint. Null inputs and sector>=12
refuse before output publication. Zone validation belongs to the caller and
the controller. This helper consumes no historical payload or code.

Native assembly preserves D2-D4/A2-A3 and emits the three-byte D5 AA AD
prologue followed by the body. Q6/Q7-high admits the gate; Q7-high/Q6-low polls
the handshake. Bit6 clear refuses; bit7 clear waits. Each accepted byte uses an
odd-address register13 strobe. A shared2,000,000-poll bound prevents an endless
wait. After the final acknowledgement or a failure it closes Q7. The model
commits a sector only after complete geometry, symbol, checksum and epilogue
validation. No multi-sector atomicity is promised.

Writer results distinguish invalid arguments, failed initial read, bounded
timeout and modeled rejection. Input bytes remain unchanged. Provider state
and scratch may change. A failure after sending the final byte can have an
uncertain commit; callers must re-read rather than assume rollback. The API
does not equate a guest acknowledgement with persistent host storage.

BootHush must separately enable a private copy using `--floppy-out NEW_PATH`.
Successful export and a fresh guest reading that file are necessary persistence
evidence. The original input remains immutable. Protected default media,
malformed records and failed export are not successful saves. Original tests
cover all five zones, backward seeks, final sector, preserved tags, full output
comparison and fresh-start readback. No MFS mutation, File Manager operation,
selected owner-media write, System/Finder startup or physical claim follows.
