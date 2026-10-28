# SPEC-0079 - PC second-report checkpoint

2026-10-28; original diagnostic contract, not a decoder or hardware correction.
The existing raw-motion-later checkpoint ignores zero dx/dy, so a missing trigger
does not establish absence of a second report. Owner release work (SRC-0521)
authorizes a bounded diagnostic to resolve that distinction.

CLASSICK_PC_INPUT_PROBE_REPORT defaults0 and accepts only0/1. At1 it requires
RAW_PACKET_PROBE=1, MOTION=1 and SKIP_FIRST_PACKET=1. It observes only PACKET
deliveries with total decoded packet counter>=2, including zero motion. Null,
NONE, LOST and first-packet inputs do not trigger it. No controller, mouse
decoder, timing, loss, presentation or normal-session behavior changes.

The native observer copies eight64-bit words into r8..r15 using the existing
original audited nonreturning leaf, then spins. Word0 is the distinct CSPRPT01
magic4353505250543031. Words1..7 retain the raw-record packing: packet/error
counts, raw header/X/Y from delivery (not reset parser state), signed dx/dy bits,
buttons/down/suppress, pointerXY, windowXY, input/drag state. Signed values use
explicit two's-complement32-bit encodings. It is a trusted live-session observer,
not a validator of arbitrary memory or guest pointers. No memory/device writes
or inspection of firmware internals are performed by the register readout.

Default, loss and earlier motion modes remain unchanged. The builder and package
must explicitly identify raw-report-later. New-format readers must verify this
tag and the exact own-code spin/source identity before interpreting registers;
old tags are not silently reclassified. This diagnostic intentionally freezes
and is not a continuous release candidate.

TEST-0100 verifies boundaries, zero/nonzero report capture, literal register words,
const caller bytes, independent raw snapshots and compile-time refusals, with
legacy observer tests retained. IMPL-0087 records source and actual verification.
Native twins, corruption/omission controls, default byte identity, media twins
and source identity precede any isolated trial. Planned cases are not passes.

No raw/synthetic result becomes GUI or human usability evidence. Visible movement,
drag/release/cancel/restoration on exact Mac and PC candidates still gate the
paired release. Historical/source/role/physical/full128K/native-app/edition/B2
and System1 interoperability gates persist; publication remains separate.
