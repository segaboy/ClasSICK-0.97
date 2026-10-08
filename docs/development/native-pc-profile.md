# First native-PC profile and ownership contract

Date 2026-10-08. ADR-0010 / SPEC-0006; loader ADR-0011 / SPEC-0007. This is a
proposed native test profile. Original preboot validation and hosted loader calls
pass; our own EFI entry/stack/stop image is linked and inspected, never loaded.
SPEC-0008 adds owned descriptor/terminal-fault scaffolding with hosted byte tests
and unloaded image audits. No VM/native boot exists; actual descriptor/fault
execution, profile/mapping qualification and device work remain required.

[The 2026-10-08 readiness audit](boot-readiness-audit.md) confirms current
artifact integrity, including the payload copy. It records NO-GO for launch:
the payload remains a directory tree, firmware eligibility remains needs-review,
and this configuration/devices are unqualified. No native boot is observed.

| Item | Proposed first configuration / required evidence |
| --- | --- |
| Platform | Owner-installed VirtualBox 7.2.16r174877, isolated project VM with retained configuration; existing owner VMs untouched |
| CPU/RAM | x86-64, one virtual CPU, 512 MiB; this is a PC development budget, unrelated to Macintosh 128K feasibility |
| Firmware | EFI64 candidate only; exact firmware artifact hash, selected source/dependency/notice closure and clean-room eligibility required before launch |
| Board | PIIX3, ACPI enabled; one polling CPU, hardware interrupts disabled after successful handoff until owned exception state is installed |
| Display | VBoxVGA, 16 MiB VRAM, 3D off; SPEC-0007 reads current GOP RGB/BGR-reserved mode, rejects unsupported data and makes no mode change. SPEC-0009 presents the original scene with direct post-exit byte stores after the exit/ready gates; hosted-verified only. Later mode-selection contract is separate |
| Input | Standard virtual PS/2 keyboard; no USB, Guest Additions, shared folders, clipboard or drag/drop dependency |
| Timer | ACPI PM timer: SPEC-0010 captures the RSDP pre-exit, validates XSDT/FADT and samples the I/O port directly after exit (hosted-verified only); no firmware timer/Stall/Windows callbacks in native loop |
| Diagnostics | SPEC-0013 qualified TI-compatible byte-register UART at proposed 0x3F8, assumed 1.8432-MHz clock, divisor 12/9600 baud/8N1. Mapping/model/clock/capture require qualification; fixed queue, nonblocking service and bounded final drain are hosted-verified only |
| Other devices | Network/audio/USB disabled, no extension-pack requirement, no unrelated mounted disk or external boot image |
| Boot medium | Independently generated FAT/UEFI medium carrying only our PE image; packaging/filename/partition recipe will be specified and audited with the loader |

## Before final memory-map capture

The SPEC-0007 loader verifies x64 UEFI table signatures/revisions/lengths, EFIAPI
calling convention and required function pointers. PE subsystem must be UEFI
application, with exact entry/relocation/import audit independent of the earlier
native-subsystem link fixtures. No CRT, unreviewed compiler helpers or third-party
loader library. Disable firmware watchdog before the critical transaction.

Allocate and record the loaded image, at least 64 KiB kernel stack, bounded fault
stack, map buffer (up to 256 KiB), handoff record, diagnostic ring and core arena.
Native typed allocations use aligned live storage under SPEC-0003; firmware pool
alignment alone is not treated as sufficient. Eight reserved spans is the v1 limit;
combine related buffers into a truthful preallocated data region when appropriate.
Round page extents with checked arithmetic; record actual ownership, not just a
memory-type inference. Validate framebuffer and immutable map through SPEC-0006.

Copy required GOP mode, allocation ledger, map stride/version and device metadata
into owned storage. Do not retain a GOP/console/device protocol pointer in the
kernel contract. Validate firmware-derived ACPI tables/lengths/checksums and timer
address access separately before using them. No firmware callbacks remain active
in our post-handoff loop; our code uses no UEFI runtime services either.

## Capture, exit and kernel entry

Capture into the already allocated buffer, validate all bytes and owned spans,
record the opaque key, then call ExitBootServices with no intervening allocation,
console, protocol or mode operation. A stale key triggers recapture/revalidation
only; after the first exit attempt no returning to firmware or ordinary boot-service
operation. Failed map capture, unsupported data or three exit failures produce a
bounded owned diagnostic/halt path. Fake state-machine success does not prove the
real firmware call happened.

On success invalidate firmware service pointers, disable hardware interrupts,
clear direction flag and switch to our aligned stack through original x64 entry
code. Install owned exception/descriptor state before device access. The initial
kernel may retain firmware identity paging only while all non-owned memory remains
reserved; no boot-service, runtime, ACPI or unidentified page-table memory is
reclaimed. Own page tables/reclamation need a later reviewed contract. This does
not grant execute/write permission; actual mappings/capabilities must be checked.

Initialize the arena, core surfaces/queue/clock and original scene from owned
storage. Convert RGBA8 to GOP RGB/BGR with reserved byte explicitly zero; use
bounded direct framebuffer writes and preserve scanline padding. Never call GOP Blt
or SimpleTextInput/Output after exit. No visible pre-handoff picture counts as B2.
SPEC-0009 implements this presentation step for the native stop path: whole-width
bands of at most 64 KiB from the 128-KiB arena, a 32-byte owned trace record and
no device access after a failed exit. Native mapping/caching/visibility unverified.

## Native device obligations before implementation

SPEC-0012 implements the scoped two-key polling contract with eligible
manufacturer documentation. Require FADT revision >=3 with the 8042 bit set,
PS/2-mode semantics, no translation/interrupts or USB/SMM interference, released
keys during startup, and controller self-test that preserves A20/machine state.
These are unverified machine qualification prerequisites. The interactive loop
uses an eight-record arena FIFO and 112-byte trace at offset 128; only READY
starts its 60-second duration. Escape is an early stop, not acceptance.

- PS/2: separately reviewed register/reset/command/ACK/error/scan-set specification;
  drain stale firmware bytes, negotiate the declared set, maintain press/release
  state, normalize Space/Escape to SPEC-0004, count overflow and bound all waits.
  No assumption that firmware left the controller configured correctly.
- Timer: ACPI PM timer's published 24/32-bit, 3,579,545 Hz counter is a candidate;
  reject absent/unsupported hardware, validate FADT and access mode before reading.
  Polling modular extension cannot recover an arbitrary multi-wrap pause. Bound
  loop/driver work, document sampling assumptions and compare the 60-second B2
  observation with an independent wall clock; no precision or paused-VM guarantee.
- Diagnostics: SPEC-0013 uses the scoped TI register interface, one operation per
  startup poll and one LSR plus at most one THR write per timer turn. A 512-byte
  fixed queue counts dropped records; exactly 640 trace bytes at offset 256 retain
  queue/transport/termination evidence without enlarging the bundle. UART failure
  leaves keyboard progress intact. Final drain has 100-ms/100000-call limits and
  requires TEMT. THR acceptance is not remote delivery. No stdio/firmware logger,
  UART RX console or asynchronous fault output. Existing first-fault RAM remains.

These are device ownership/acceptance requirements; scoped drivers are implemented
and hosted-verified, while native behavior and machine qualification remain open.
Unknown device capability fails visibly; no hidden
firmware service fallback. This profile establishes no generic modern-PC/USB support.

## Provenance and current availability

SRC-0035 records installed tool/manual hashes and official v7.2.16 tag
`4cf0b89546257f5044534a7b94cde2dac1e8c175`. Public license section 18.2.3.12
includes Apple-attributed OVMF/Bhyve notices. That is an eligibility concern, not
proof of Apple ROM code or of inclusion in VirtualBox's x64 firmware. Metadata
inventory/ARM manifest/packaging entries did not establish the x64 producer.
SRC-0040 now locates OvmfPkgX64.dsc/fdf and selected VBOX modules from build/license
declarations; [the follow-up](firmware-eligibility.md) retains binary correspondence
and transitive derivation/notice closure as needs-review. No external EFI binary adopted.
The observed VBoxDD2.dll hash is an installed file fingerprint, not a firmware hash.

VBoxManage's version query works. Its modifyvm help request failed local COM setup
with E_ACCESSDENIED before displaying help; no VM/configuration was examined or
changed. Resolve service availability in the isolated project environment before
future configuration. This is a local tool limitation, not an auto-review rejection.

Next: resolve the exact firmware eligibility/identity, format reviewed boot media,
validate owned exception execution and implement reviewed device leaves, then run B2
with repeated cold-start traces and original-scene capture. Macintosh replacement
firmware/mini vMac and physical hardware remain independent targets; all three
OS edition boots and historical parity remain open.
