# SPEC-0007 — Original x64 UEFI loader and owned transition scaffold

Status finalized v1, 2026-10-08, lead self-review; human provenance review pending.
Owner SRC-0037; UEFI SRC-0038; Intel SRC-0039; metadata SRC-0040; PE SRC-0041.
ADR-0011 / IMPL-0011 / TEST-0013. This is a native-PC loader contract, not
Macintosh behavior. SPEC-0006 remains unchanged and supplies preboot validation.

The same orchestration code is linked into a PE32+ EFI application and exercised
against original hosted callbacks. Only x64 declarations/calls are supported.
It does not provide a complete kernel, native devices, B2 or any edition boot.
No external UEFI headers/library/example implementation or firmware is adopted.

## Entry and supported interfaces

Firmware supplies truthful live aligned table/protocol objects and valid callable
x64 EFIAPI function pointers, and owns their pre-exit lifetime. Metadata checks
cannot establish arbitrary pointer accessibility, honest firmware or permissions.
Our own declarations have static size/offset assertions for the published x64 ABI.
EFIAPI uses the Microsoft x64 convention; entry receives image handle/system table.

Accept only UEFI major 2 tables with exact supported sizes: system 120 bytes,
boot services 376. Check signatures, reserved header zero and independent CRC32
with the checksum field treated as zero. Future extensions are unsupported v1,
not malformed universal UEFI. Required allocation/free/map/open/locate/watchdog/
exit pointers must be nonnull. Do not inspect console/runtime implementations.
Nonzero service statuses, including warnings, are failures in this strict profile.

Disable the watchdog with timeout/code/size zero and null data; accept success
or UNSUPPORTED (no watchdog). Open LoadedImage using GET_PROTOCOL, agent=image
handle/controller=null; no CloseProtocol required by that mode. Require revision
>=0x1000, matching system table, page-aligned nonzero image base, positive size
<=16 MiB, rounded page span below 2^47, code type LoaderCode/data type LoaderData.
Locate the first GOP instance; validate current mode/index/info version zero,
size 36..256, then SPEC-0006 framebuffer. V1 keeps the current mode and rejects
unsupported RGB/BGR data; it performs no mode change or firmware drawing.

## Owned bundle and final capture

AllocateMaxAddress below 2^47, 128 pages of LoaderData (512 KiB). Successful return
must be page aligned/nonzero/in range, disjoint from the loaded image/framebuffer
before any write. Allocated pages are suitable live native storage on the named
identity-mapped x64 environment. Failure/corruption never licenses a wild write.
Zero the bundle with an original bounded byte loop; retain no firmware pointer
in the kernel handoff. Header area 4 KiB; map 256 KiB; stack 64 KiB; reserved
fault stack 16 KiB; arena 128 KiB; trace 16 KiB; remainder reserved. Stack top
is 16-byte aligned. Fault stack/arena/trace are reserved but not initialized as
kernel services in v1. No page reclamation or framebuffer access.
Malformed allocation return metadata is neither written nor freed: its ownership
cannot be safely inferred. This bounded leak remains firmware-owned until reset.

GetMemoryMap always receives the preallocated 256-KiB buffer. Buffer-too-small,
zero/oversized output, unsupported descriptors or ownership failure reject;
no growing buffer. Validate map before deriving up to seven loaded-image spans
from its descriptor intersections. Require complete image page coverage, each
type LoaderCode/LoaderData without runtime bit. Add the one owned bundle span,
then SPEC-0006's eight-span validation. Preserve map bytes/stride/version/key and
owned ledger in the handoff; no protocol/service/configuration-table pointers.

Cache required callback pointers before attempts; do not reread protocol/service
tables after the first exit attempt. Only owned handoff/map data crosses the boundary.

No allocation, logging, protocol or other firmware call occurs between capture
and exit. On stale key (EFI_INVALID_PARAMETER) recapture/revalidate only, at most
three attempts. Record the actual outcome with SPEC-0006. Before any exit attempt,
rejecting the map returns a failure after a single FreePages attempt. After any
exit attempt, never free/return to firmware, even when subsequent capture fails.
Preserve bounded reason/status/attempts in owned storage. Free failure is reported,
not silently treated as successful cleanup. No indefinite retries or fallback I/O.

## Actual entry wrapper and transition

Hosted orchestration returns a disposition for inspecting results; the EFI entry
wrapper enforces the boundary. Before an exit attempt it may return EFI failure.
After attempted exit, success or failure takes an original nonreturning x64 path:
CLI, CLD, switch to the reserved aligned stack, provide 32-byte call home space,
call the native stop scaffold, then halt/re-halt. No boot/runtime callback follows.
The scaffold records native entry in owned memory. It does not draw or touch devices.

CLI masks ordinary interrupts; it does not mask exceptions/NMI. V1 retains firmware
descriptor/paging state and reserves all non-owned memory. Own GDT/IDT/TSS, fault
handling, page mappings, timer/input/framebuffer/diagnostic drivers and event loop
are still required before a B2 run. The terminal scaffold is an inspected development
image, never launched against unreviewed firmware or advertised as a working OS.

## Image/package and acceptance

Link our own C/assembly at x64 O0/O2 without CRT/startup/default libraries. EFI
subsystem 10, exact cs_uefi_entry RVA, zero imports/TLS/IAT/delay/runtime libraries,
zero timestamp, executable non-writable entry, bounded sections and valid DIR64
base-relocation records. Preserve an intentional original address anchor so relocation
handling is audited. Fresh twins match per optimization; no cross-profile identity claim.
Prepare an EFI/BOOT/BOOTX64.EFI directory containing only the audited own image.
This is a payload tree, not a formatted FAT disk or an observed boot medium.

Hosted tests check CRC/layout, success/stale retries, bounded terminal failures,
service order/key/buffer arguments, map/image/bundle ownership, output stability,
cleanup and independent executions. Native image inspection verifies structure/link
closure and stack/halt bytes; privileged entry is not executed in hosted tests.
Firmware eligibility remains open under SRC-0040; real firmware/handoff/stack,
exceptions/devices/B2/Macintosh/mini vMac and physical edition gates remain unverified.
