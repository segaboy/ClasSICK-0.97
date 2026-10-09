# SPEC-0016 — Native qualification diagnostics

Version 1, finalized 2026-10-09, implementation-lead self-review (Claude); human
provenance review pending. Owner delegation SRC-0064; interfaces SRC-0049 (SMSC
output port and D0), SRC-0043 (A20M#), SRC-0038 (UEFI 2.11 §4.3.1 system-table
identity, Table 7.10) and SRC-0034 (§2.3.4 identity mapping, §7.2.3 memory
attributes). ADR-0020 / IMPL-0020 / TEST-0024. Amends SPEC-0007 (handoff
version 2), SPEC-0012 (startup v2) and SPEC-0013 (serial v2). Original native-PC
contract; no Macintosh behavior. Hosted/unloaded evidence only until a trial.

## Purpose and boundary

TEST-0023 left two measurement gaps: SPEC-0012's precondition that controller
self-test preserve A20 and required machine state, and the firmware's own
identity. This contract adds the measurements to the normal native image, so one
payload is both the B2 candidate and its qualification probe. The SPEC-0015 trial
remains evidence for payload `8e25d694…ad831f2` only; the new payload needs its
own trial, including operator keyboard input. Criterion 6's firmware *hash* is not
addressed: a self-reported vendor/revision is not a hash, and B2 stays partial on
that criterion unless the owner approves an amendment (SRC-0064).

Reads beyond our own memory are limited to the UEFI-defined system-table identity
fields before exit and one 8-byte partner location after exit, which is compared
and never written, stored or exported. No firmware image, module, variable store
or memory dump is read. Nothing here changes the SRC-0059 black-box boundary.

## Firmware self-report (SPEC-0007 amendment, handoff version 2)

Before any exit attempt, and after the bundle is zeroed, the loader copies the
CRC-validated system table's `FirmwareRevision` and at most 31 UCS-2 code units
of `FirmwareVendor` into the handoff. Units are read bytewise as little endian.
0x20..0x7E copy as ASCII; any other unit becomes `?`. NUL ends the copy. State:
COMPLETE=0 (NUL within 32 units), TRUNCATED=1 (unit 31 nonzero), ABSENT=2 (null
pointer, nothing read). The 32-byte field is NUL-filled; byte 31 is always zero.
No firmware pointer is retained. Handoff version becomes 2: fields
`firmware_revision`, `firmware_vendor_state` (u32) and `firmware_vendor[32]`
follow `rsdp`; the header page bound is unchanged. UEFI §4.3 says the vendor and
revision fields remain valid after exit; we copy before exit regardless.

## Controller output port (SPEC-0012 v2)

Startup gains two Read Output Port commands, each followed by one controller
reply phase under the existing OBF/IBF, auxiliary, error and 100-ms rules:

1. AD, A7, drain; 20 and reply; 60 and data (unchanged phases 0–6).
2. **D0 and reply (phases 7–8): record `port_before`.**
3. AA and 55 (9–10), AD, A7, drain (11–13).
4. **D0 and reply (14–15): record `port_after`; if bits 0 (System Reset) or 1
   (Gate A20) differ from `port_before`, stop with MACHINE=9.**
5. 60, data, 20, readback check (16–19); AB/00 (20–21); AE (22); FF, ACK, BAT
   (23–25); F5, F0, 02, F4 with ACKs (26–33); READY is phase 34.

Self-test (phase 10) keeps the 500-ms bound and BAT (phase 25) the 1-s bound.
Bits 2–7 (auxiliary and keyboard lines, buffer-full interrupt outputs) are
recorded but not compared. The driver never writes D1 or pulses F0–FF. Begin sets
both port values to PORT_UNREAD=0x100. The D0 replies are diagnostic: if one does
not arrive within its 100-ms/1,000,000-call bound, that value stays PORT_UNREAD
and startup continues without I/O on that turn; the comparison runs only when
both values were read. An unread value makes the qualification incomplete. SMSC
names the command but these pages do not describe its reply, so receiving it as
an ordinary controller byte is this profile's assumption.

## Bit-20 alias probe

The owned cell is the 8 bytes at bundle trace offset 1152, physical
P = trace_base + 1152. Its partner is Q = P XOR 2^20. Intel's A20M# masks
physical bit 20 (SRC-0043); while masked, P and Q reach the same location.

Q is eligible only if one descriptor of the validated final map contains
[Q, Q+8) with type EfiLoaderData, EfiBootServicesData or EfiConventionalMemory
(2, 4, 7), attribute EFI_MEMORY_WB and no EFI_MEMORY_RUNTIME, and Q lies inside
the native identity window. UEFI Table 7.10 gives the exiting loader those types;
code types 1 and 3 are excluded so no code bytes are read. Otherwise the result
is UNAVAILABLE and Q is not accessed. The descriptor type and attribute found
(or type 0xFFFFFFFF) are recorded either way.

Procedure: save the cell; write 8 pattern bytes `(0x5A + 37i) mod 256` to the cell,
read Q's 8 bytes; write the complements, read Q again; restore the cell. All
accesses are volatile bytes. ALIAS=1 when both reads equal what was just written
to the cell, otherwise PASS=0. A constant Q cannot match both patterns.

The probe runs twice: once on the first timer turn after all SPEC-0012 gates and
before the first controller access (`before`), and once when the keyboard first
reports READY, before the first frame (`after`). Either ALIAS stops with loop
result MACHINE=14 before any further keyboard access or drawing; the serial lines
and termination drain still run. A PS/2 MACHINE
result also maps to loop MACHINE=14 (keyboard result 9 tells them apart).
NOT_RUN=3 marks a probe that did not run. A null qualification pointer, as in
`cs_native_keyboard_loop`, disables both probes; behavior is otherwise identical.

## Qualification record

128 bytes at bundle trace offset 1024, rewritten with every keyboard trace record
(same non-atomic rule), followed by the cell at 1152. Little-endian fields: magic
0x31515043 at 0, version 1 at 4, loop result 8, keyboard result 12, port_before
16, port_after 20, alias before 24, alias after 28, partner type 32, firmware
revision 36; u64 cell P 40, partner Q 48, partner attribute 56, image base 64,
bundle base 72, stack top 80; vendor state 88, zero 92, vendor bytes 96..127. A
null keyboard trace writes nothing; other rejected gates write an initialized
record (probes NOT_RUN, ports 0x100). With a null handoff, identity fields are
zero, the vendor state is ABSENT and P/Q are zero.

## Serial lines (SPEC-0013 v2)

The banner becomes `ClasSICK 0.97 UART v2`. Immediately after it:
`fw rev=<8> state=<8> vendor=<ASCII>` (the UART re-filters to printable ASCII,
at most 31 bytes) and `own img=<16> bun=<16> stk=<16>`. After `keyboard ready`,
or at termination if READY never arrived: `qual port=<before>/<after>
a20=<before>/<after>` and `alias q=<16> type=<8>`. Fields are uppercase hex; 16
digits for u64. Existing frame and result lines are unchanged. The native UART
loop now requires a nonnull qualification pointer.

The termination drain bound becomes 3,579,545 ticks (1 s) and 4,000,000 turns,
enough to empty a full 512-byte ring at 9600 baud; the transport's per-byte
no-progress bounds are unchanged. With these lines an early failure enqueues
about 270 bytes, which the former 100-ms drain could not send.

## Interpretation

Qualification of a machine for SPEC-0012 requires, in every run: alias PASS
before and after, port bits 0–1 equal, and no MACHINE result. PASS says owned
addresses differing only in bit 20 did not alias in that run; the port values are
controller-reported state, which may not be how the platform gates A20. The
physical probe is the decisive measurement. UNAVAILABLE is incomplete, not a
pass. ALIAS or a port change fails closed. Firmware vendor/revision identify the
firmware as it reports itself; they are not a hash and do not prove provenance.

## Acceptance

Hosted tests: the PS/2 transcript with both D0 commands, renumbered reply/ACK/
timeout phases, every port-bit mismatch, an ignored bit-2–7 change, a controller
that never answers D0, no D1/F0–FF writes; loader vendor copy for empty, 5, 30, 31, 32 and 39 units, non-printable
and non-ASCII replacement, no NUL, null pointer and capture before exit (the mock
overwrites the string at exit); probe PASS for types 2/4/7 with the partner
unchanged and the cell restored; UNAVAILABLE for types 1/3/6/9, missing WB, the
runtime bit, an unmapped partner and a partner outside the window; ALIAS before
startup with no controller access; ALIAS appearing at self-test, failing before
drawing; record contents and guards; exact serial transcripts including failure
paths; drain cap and stopped-clock bounds. Both compilers, x64 sanitizers,
object import audits and unloaded O0/O2 EFI twins apply. No native claim.
