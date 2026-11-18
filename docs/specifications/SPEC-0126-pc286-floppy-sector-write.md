<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0126 - Modeled native286 single-sector floppy writer

2026-11-18. Original provider contract, SRC-0620, sharing the admitted
SPEC-0113 geometry and native buffer checks. The only operation writes one
512-byte sector on BootHush built-in firmware drive0. No generic multi-sector
write transaction, MFS mutation, physical BIOS claim or retry/reset is added.

`cs_pc286_floppy_write_bind(writer, first, blocks)` validates a nonempty range
inside2400 sectors, current native DS and the writer's512-byte bounce address,
then publishes only its first/count/segment controls. Failure preserves every
writer byte; success leaves bounce unchanged. It performs no media I/O and
does not enable writes or discover media. The writer is caller-owned small-model
native near data. Source, writer, all other live controls and borrowed storage
are stable/exclusive/disjoint; each native object fits one segment. Arbitrary
overlap or physical alias detection is not promised. The source is a truthful
readable512-byte RAM object; it need not itself meet BIOS DMA placement rules.

`cs_pc286_floppy_write_one(writer, lba, source)` first refuses nulls and exact
source/bounce identity as ARGUMENT, malformed controls/stored DS or unsafe bounce
as STATE, and an out-of-range logical LBA as RANGE. These pre-I/O refusals leave
source, controls, bounce and media unchanged. Complete object validity and a
stable actual DS remain preconditions; the stored segment check cannot repair
an invalid native pointer after a caller changes DS.

Bounce admission reuses `cs_pc286_floppy_buffer_fits`: near offset<=FE00,
physical DS*16+offset<=9FE00 and low16 physical bits<=FE00. After validation,
copy all512 source bytes to bounce, then issue exactly one INT13 AH03/AL01
using cylinder=p/30, head=(p/15)%2 and sector=(p%15)+1, p=first+lba.
The source and writer controls stay unchanged on every result. Bounce may
change after admission, including write-protection or another BIOS refusal.
OK means CF clear and AH0; PROTECTED means CF set and AH3; all other firmware
responses are IO. On the admitted model each rejected operation preserves the
entire private disk image. A successful operation commits exactly its one
sector. A later operation's failure does not undo an earlier successful write.

The original inline wrapper saves FLAGS, DS, ES, SI, DI and BP, sets ES=DS,
and invokes the BIOS with explicit CX/DX/BX arguments. It retains AH while
converting returned CF into AL using MOV then ADC before restoring FLAGS;
AX/BX/CX/DX are declared clobbered. Caller IF is restored and DF is cleared
for C. CS/SS and balanced stack behavior remain properties of the admitted
model, not arbitrary firmware guarantees. No runtime allocation or helper
library is allowed in the native provider closure.

Original qualification writes patterned sectors at first/last range positions,
head and cylinder boundaries, checks same-guest readback, and exports only to
explicit new copies. Whole-image comparison must show exactly those six
sectors changed. A fresh boot from that exported copy verifies all writes
before any write is attempted. Default read-only refusal and invalid controls
preserve the complete original image/source/canaries. Numeric segment/DMA/RAM
boundary tests do not fabricate pointers. Wrong-head, truncated-copy and
ignored-protection mutations must fail original checks. Exact source/tool/media
identities and loaded payload verification belong to TEST-0217. No owner media,
filesystem write, System boot or physical persistence is qualified by this work.
