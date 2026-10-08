# SPEC-0008 — Owned x64 terminal exception state

Finalized v1, 2026-10-08; implementation lead self-review, human provenance review
pending. Owner SRC-0042; Intel public interfaces SRC-0043; ADR-0012 / IMPL-0012 /
TEST-0014. Original native-PC contract; no historical Macintosh behavior.

## Profile and ownership

One CPU, CPL0, ordinary 64-bit IDT delivery, CET/shadow stacks and FRED disabled.
The named firmware/machine must establish this profile before execution. Retain
identity paging and reserve all non-owned memory, including its page tables.
Our image code must be executable; image data, stack and tables must be writable.
Address validation is not a check of actual page permissions or accessibility.
No paging changes, reclamation, interrupt enabling, recovery, scheduler or devices.

The existing 512-KiB loader bundle has unused tail storage. Reserve its first
8 KiB starting at offset 495616 for CPU state. Split the existing 16-KiB fault
area into four disjoint 4-KiB stacks: general fault, NMI, double fault and machine
check. The 64-KiB ordinary kernel stack remains separate. Bundle ledger unchanged.

## Byte builder

`cs_x64_tables_init(storage, capacity, layout)` receives live caller-owned byte
storage of at least 8192 bytes and disjoint live layout input. Logical addresses
need not equal hosted pointers; native use must provide their truthful identity.
Layout supplies table base, vector block base, ordinary stack top and four IST
tops. All ranges lie strictly below 2^47, nonzero and pairwise disjoint. Table
base is 16-byte aligned, vector base 32-byte aligned, stack tops page aligned.
Table and vector blocks occupy 8192 bytes each, ordinary stack 65536 bytes, each
IST stack 4096 bytes. Range-end equal to 2^47 is accepted; no overflow arithmetic.

Null storage/layout returns ARGUMENT; short capacity or invalid address/alignment
returns LIMIT; overlapping logical ranges returns OVERLAP. Failures leave every
storage byte unchanged. Success zeros exactly 8192 bytes; excess capacity is
unchanged. No heap, library, packed structure, native dereference of logical
addresses or global state in the builder. Byte fields are explicitly little endian.

| State offset | Bytes | Contents |
| --- | --- | --- |
| 0 | 40 | GDT: null; selector 8 accessed/readable long code, L=1/D=0; selector 16 accessed/writable flat data; selector 24 available 64-bit TSS descriptor |
| 40 | 10 | GDTR limit 39 and table base |
| 50 | 10 | IDTR limit 4095 and table base + 256 |
| 64 | 104 | TSS; RSP0 ordinary stack, IST1–4 supplied tops, I/O-map offset 104 exceeds limit 103; other bytes zero |
| 176 | 64 | Fault record: u32 state, u32 vector, u64 error/RIP/CS/RFLAGS/old-RSP/old-SS/CR2 |
| 240 | 4 | Descriptor installation complete marker, initially zero |
| 256 | 4096 | 256 present DPL0 interrupt gates, selector 8, vector base + vector * 32 |

IST1 is the default; vector 2 selects IST2, 8 selects IST3, 18 selects IST4.
Unused/reserved bytes zero. GDT access bytes 0x9B/0x93, flags 0xAF/0xCF;
TSS type/access 0x89, limit 103 with byte granularity; IDT access 0x8E.

## Native assembly and terminal policy

Original EFI native stop builds tables from its validated owned handoff before
any device access, then calls original `cs_x64_install`. Its one aligned 8-byte
image-data slot stores the active state pointer before installing descriptors.
The helper keeps ordinary interrupts masked, loads GDTR, reloads CS through a
64-bit far return, loads DS/ES/SS, clears LDTR, loads TR and IDTR, then marks ready.
FS/GS bases are unused. It returns on the ordinary stack; native stop then halts.
No helper is linked into hosted tests or executed there. Failure to build halts.

This is not an atomic descriptor change. NMI/machine-check/fault delivery before
LIDT can still encounter inherited state. Known live mapped tables/code/stacks
and the named single-CPU startup profile are prerequisites. No immunity to this
transition window, nested same-IST faults, broken mappings, SMI or triple faults.

256 original 32-byte stubs normalize a seven-qword frame. CPU error-code vectors
8,10,11,12,13,14,17,21 keep the CPU error; others push zero. Every stub pushes a
32-bit immediate vector with zero extension and jumps to one common terminal
handler. Software INT to an error-code vector is outside this profile because
the CPU does not push an error for INT. AMD-specific vectors 29/30 unsupported.

The handler CLI/CLD, snapshots CR2, claims the record with locked cmpxchg state
0 -> 1, writes vector and six frame fields plus CR2, then publishes state 2 and
halts/re-halts without calls, device access, firmware or return. State 1 means an
incomplete capture. A nested handler seeing nonzero state halts without modifying
the record. The first capture may remain incomplete if interrupted/faulted. CR2
is diagnostic only and is the fault address only for a page fault. No GPR snapshot,
UART/pixel fault report or recovery claim. The bounded record persists until reset.

## Acceptance and limitations

Hosted independent decoders check whole-byte GDT/TSS/register/gate fields, all
256 vectors, reserved bytes, unaligned host storage, guards, rejection stability,
address boundaries/overlap, multiple layouts and independent buffers. Both
compilers and actual i686 exercise only serialization. Sanitizers cover the C
builder. Freestanding object has no undefined functions or mutable globals.

Fresh own EFI O0/O2 twins retain this C and native assembly without imports or
default libraries. Image audit checks writable non-executable pointer slot, exact
installation/record/stub bytes and relative destinations for every vector, and
mutation/omitted-object rejects. Images remain unloaded until reviewed firmware
and the native profile are established. Hosted serialization and image inspection
do not prove LGDT/LTR/LIDT, exception delivery, runtime stack safety or B2.

Real boot fault injection, visible diagnostics, framebuffer/timer/keyboard loop,
formatted media and firmware eligibility remain required. Mac/mini vMac and all
physical-edition gates remain independent and unverified.
