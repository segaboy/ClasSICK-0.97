# SPEC-0022 - Checked guest memory and data/control instructions

Finalized v1, 2026-10-12, lead self-review; human provenance review pending.
SRC-0096-0098 / ADR-0025 / IMPL-0026 / TEST-0030.
Profile `classick-memory-execution-v1`, extending SPEC-0021.

## Published instruction facts

SRC-0097: Motorola, 1992 M68000 Family Programmer's Reference Manual,
1.7.1/2 (1-25/27), 2.2.4/2.2.5 (2-6/7), 2.2.17/18 (2-18/19),
2.6.1 (2-28), Table 3-19 (3-19), MOVE
(4-116/118), MOVEA (4-119/120), Bcc (4-25/26), BRA (4-55/56),
BSR (4-59/60), and the previously reviewed MOVEQ/NOP/RTS sections.
Encoding tables and prose only, no sample code. MOVE changes N/Z, clears V/C,
preserves X; MOVEA preserves CCR and sign-extends a word. Byte/word Dn writes
preserve the other bits. Address-register byte operands are invalid. Increment
or decrement is operand size, except A7 byte uses two. Branch displacement is
relative to opcode PC+2, including word displacement; BSR pushes the address
after the entire instruction. This profile is 68000: FF is an eight-bit negative
displacement, not the later CPUs' long-displacement extension.

## Independent memory contract

At most eight caller-owned, nonempty, even-base/even-size, nonoverlapping logical
regions within 24 bits. Each borrows a truthful native read buffer; WRITE requires
the same writable buffer; WRITE/EXECUTE require READ. No heap allocation, global
state, MMIO, address masking, or conversion of guest addresses into native pointers.
Caller keeps descriptors/buffers live and synchronized, and disjoint from frames,
reports, gateways and outputs. Native aliases are the caller's responsibility.

Validate the whole map before each operation. Read/write widths are 1/2/4;
word and long use even alignment. Values are big-endian. A complete access must
fit one region; even adjacent regions do not jointly satisfy it. The bounded
window API resolves an arbitrary nonempty byte span with explicit permissions;
it does not itself impose instruction alignment. Failed reads preserve output;
failed writes preserve buffers. Report argument/map/width/address/permission
failures separately. This is mapped storage, not Macintosh allocation policy,
handles, pointers, compaction, heap zones, or historical RAM layout.

## Independent CPU profile

New `cs_m68k_bus_step` leaves the SPEC-0021 API unchanged. Supervisor S=1/T=0
only. Fetch requires READ+EXECUTE, data read READ, store WRITE. PC and transfer
targets are checked 24-bit even addresses, without wrapping. MOVE.B/W/L and
MOVEA.W/L support Dn, An (word/long source), (An), (An)+, -(An), absolute long,
and immediate source. Other addressing modes explicitly stop. Source evaluation
and its auto-address update precede destination evaluation. Register arithmetic
is unsigned 32-bit; a subsequent memory use still has to satisfy the 24-bit map.

BRA/BSR and all fourteen conditional branches support byte/word displacements;
all preserve SR. Validate taken target as an executable word before committing.
BSR checks a four-byte stack write and pushes the full return PC; RTS validates
the popped executable target before changing A7. No JSR/JMP, arbitrary exceptions,
user stack switching, cycle/prefetch accuracy, or general historical gateway.
MOVEQ/NOP/RTS/Axxx retain the prior profile. Axxx preflights six writable bytes
below A7, uses the shared handler, and leaves completed SR/PC below unchanged A7.
All failures preserve registers, gateway and mapped bytes. Decode/check into a
temporary value frame; at most one successful memory store commits. Reports are
diagnostics and may change on failure. Borrowed map synchronization makes the
preflight-to-commit interval a caller precondition.

## Independently authored program and verification

A 36-byte program moves 11223344 into own data at logical 1800, reads with
postincrement, overwrites its lower word with A55A using predecrement, tests an
untaken BNE, calls a subroutine with BSR, writes byte 7F, reads the new word/long,
and returns. Twelve steps, one BSR, final data 11227F5A, D1=11223344,
D2=7F5A/D3=11227F5A/A0=1800/SR=2710. Buffers and addresses are engineering
fixtures, not a Macintosh heap or legacy application. A NOP provides a bounded
caller continuation for the interpreter. Budget is fixed; no unbounded run loop.

Independent truth tables test every branch/CCR combination; operand widths,
registers, addressing side effects, endian/alignment/span/permission boundaries,
failed-step atomicity, stack/target errors and original program guard bytes are
tested separately. Clang/GCC widths, sanitizers and little/big-endian standalone
objects provide scoped portability evidence. Native linkage/build and actual
CPU execution must be identified separately; no new boot claim follows from
hosted results. Existing native image/platform admissions stay revision-specific.
Claude's allocation research remains a draft and supplies no implementation here.
All full CPU/OS/physical/edition gates and B2 criterion 6 remain unchanged.
