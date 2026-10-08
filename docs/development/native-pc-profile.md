# First native-PC profile and ownership contract

Date 2026-10-08. ADR-0010 / SPEC-0006. This is a proposed reproducible test profile
and an implementation contract; no VM, UEFI image or native boot exists yet.
The tested subset is preboot data/exit-model validation, IMPL-0010 / TEST-0012.

| Item | Proposed first configuration / required evidence |
| --- | --- |
| Platform | Owner-installed VirtualBox 7.2.16r174877, isolated project VM with retained configuration; existing owner VMs untouched |
| CPU/RAM | x86-64, one virtual CPU, 512 MiB; this is a PC development budget, unrelated to Macintosh 128K feasibility |
| Firmware | EFI64 candidate only; exact firmware artifact hash, selected source/dependency/notice closure and clean-room eligibility required before launch |
| Board | PIIX3, ACPI enabled; one polling CPU, hardware interrupts disabled after successful handoff until owned exception state is installed |
| Display | VBoxVGA, 16 MiB VRAM, 3D off; query current GOP data, choose supported RGB/BGR-reserved mode before final capture; prefer 1024x768 without assuming availability |
| Input | Standard virtual PS/2 keyboard; no USB, Guest Additions, shared folders, clipboard or drag/drop dependency |
| Timer | Optional ACPI PM timer must be discovered/validated and sampled directly after exit; no firmware timer/Stall/Windows callbacks in native loop |
| Diagnostics | Polling 16550A COM1 to local bounded capture; UART wait budgets must not block input/timer sampling |
| Other devices | Network/audio/USB disabled, no extension-pack requirement, no unrelated mounted disk or external boot image |
| Boot medium | Independently generated FAT/UEFI medium carrying only our PE image; packaging/filename/partition recipe will be specified and audited with the loader |

## Before final memory-map capture

The future loader verifies x64 UEFI table signatures/revisions/lengths, EFIAPI
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

## Native device obligations before implementation

- PS/2: separately reviewed register/reset/command/ACK/error/scan-set specification;
  drain stale firmware bytes, negotiate the declared set, maintain press/release
  state, normalize Space/Escape to SPEC-0004, count overflow and bound all waits.
  No assumption that firmware left the controller configured correctly.
- Timer: ACPI PM timer's published 24/32-bit, 3,579,545 Hz counter is a candidate;
  reject absent/unsupported hardware, validate FADT and access mode before reading.
  Polling modular extension cannot recover an arbitrary multi-wrap pause. Bound
  loop/driver work, document sampling assumptions and compare the 60-second B2
  observation with an independent wall clock; no precision or paused-VM guarantee.
- Diagnostics: separately reviewed 16550 register contract, bounded TX polling,
  fixed-size trace ring, lost-record count and original text only. No blocking
  stdio/firmware logger. Interrupt/CPU fault handling needs primary ISA contracts.

These are device ownership/acceptance requirements, not completed register-level
drivers. Exact ISA/i8042/UART/ACPI-table parser leaf specifications and tests are
required before implementation. Unknown device capability fails visibly; no hidden
firmware service fallback. This profile establishes no generic modern-PC/USB support.

## Provenance and current availability

SRC-0035 records installed tool/manual hashes and official v7.2.16 tag
`4cf0b89546257f5044534a7b94cde2dac1e8c175`. Public license section 18.2.3.12
includes Apple-attributed OVMF/Bhyve notices. That is an eligibility concern, not
proof of Apple ROM code or of inclusion in VirtualBox's x64 firmware. Metadata
inventory/ARM manifest/packaging entries do not establish the complete x64 binary
closure. Firmware stays needs-review; no external EFI binary has been adopted.
The observed VBoxDD2.dll hash is an installed file fingerprint, not a firmware hash.

VBoxManage's version query works. Its modifyvm help request failed local COM setup
with E_ACCESSDENIED before displaying help; no VM/configuration was examined or
changed. Resolve service availability in the isolated project environment before
future configuration. This is a local tool limitation, not an auto-review rejection.

Next: resolve the exact firmware eligibility/identity, finalize the loader/UEFI ABI
and packaging, implement native entry and the reviewed device leaves, then run B2
with repeated cold-start traces and original-scene capture. Macintosh replacement
firmware/mini vMac and physical hardware remain independent targets; all three
OS edition boots and historical parity remain open.
