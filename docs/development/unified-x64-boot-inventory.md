# Unified x86-64 boot dependency inventory

Source revision `fdc6d3e983b55f7a3c3bb38da345ab71e34b7944`. SRC-0343..0345. Lead-reviewed engineering inventory; proposed semantics are not a frozen runtime contract. These inputs contain a UEFI image path and no BIOS loader, media layout, or new boot evidence. Items in sections 2–4 are proposals.

## 1. Source-pinned current facts

The admitted PC product is one EFI-linked `BOOTX64.EFI` (`scripts/Verify-UEFIImage.ps1`) built from the source list at `scripts/Verify-UEFIImage.ps1`. The roadmap says the current PC release is UEFI-only and that BIOS support is unimplemented (`docs/development/unified-x64-pc-plan.md`). `platform/uefi/abi.h` requires an x64 pointer width. `cs_uefi_loader_run` fills a handoff and does not transfer (`platform/uefi/loader.h`).

| # | Fact | Binding | Locator |
| --- | --- | --- | --- |
| 1 | One EFI link named `BOOTX64.EFI` | Firmware image | `scripts/Verify-UEFIImage.ps1` |
| 2 | x64 pointer width and `CS_EFIAPI` | UEFI call profile | `platform/uefi/abi.h` |
| 3 | Loaded-image, boot services, and GOP required before allocation | UEFI protocols | `platform/uefi/loader.c` |
| 4 | 512 KiB bundle and handoff version 2, capped to one page | Record filled by the UEFI loader; offsets reused after exit | `platform/uefi/loader.h`, `platform/uefi/loader.c` |
| 5 | Exit snapshot, observe, and retry; success is status 0 | UEFI exit path | `platform/uefi/contract.h`, `platform/uefi/loader.c` |
| 6 | `cs_x64_enter` clears interrupts, aligns the stack, calls `cs_native_stop` | Native transition glue; transfer is not itself proof of exit success | `platform/uefi/transition.S`, `platform/uefi/entry.c` |
| 7 | Descriptor tables use bundle CPU, stack, and fault offsets | x64 table code; native use needs live identity mapping | `platform/pc/x64/state.h`, `platform/uefi/entry.c` |
| 8 | Present runs only if exited, status 0, and ready equals 1 | UEFI stage gate, then `cs_fb_*` | `platform/uefi/native.c`, `platform/pc/framebuffer.h` |
| 9 | Reads use map kinds 0, 6, 9, 10 inside a declared window | UEFI map plus entry window | `platform/uefi/entry.c`, `platform/uefi/native.c` |
| 10 | Timer requires RSDP/map validation; keyboard loop additionally requires ACPI 8042 presence | Post-exit capability gates | `platform/uefi/native.c`, `platform/uefi/keyboard.c` |

Firmware-bound today: boot-service table checks and the allocate/get-map/exit/open/watchdog/locate set (`platform/uefi/loader.c`); loaded-image code type, data type, and base (`platform/uefi/loader.c`); GOP mode and pixel geometry (`platform/uefi/loader.c`); configuration-table RSDP capture (`platform/uefi/loader.h`, `platform/uefi/loader.c`); vendor bytes and firmware revision (`platform/uefi/loader.h`, `platform/uefi/loader.c`); map key and exit attempts (`platform/uefi/loader.c`).

Called from that path, and still hosted on its record: `cs_fb_init` / `cs_fb_present`, whose comments say validation cannot prove mapping or caching (`platform/pc/framebuffer.h`); `cs_x64_tables_init`, which requires truthful live identity-mapped addresses (`platform/pc/x64/state.h`); arena and boot-scene calls inside `cs_native_present` (`platform/uefi/native.c`). Port access is indirect (`platform/uefi/native.h`, `platform/uefi/entry.c`). `cs_native_stop` still takes `cs_uefi_handoff` (`platform/uefi/entry.c`). Unsupplied ACPI, PS/2, timer, UART, and app bodies are link members only (`scripts/Verify-UEFIImage.ps1`); their names do not prove behavior. The 512×342 monochrome canvas is a release rule (`docs/development/unified-x64-pc-plan.md`).

Present, timer, and keyboard all stop unless `stage` is `CS_UEFI_EXITED` and `firmware_status` is 0 (`platform/uefi/native.c`, `platform/uefi/keyboard.c`). A zero RSDP is a timer and keyboard failure (`platform/uefi/native.c`, `platform/uefi/keyboard.c`). After allocation, a failed map/ownership check before the first exit attempt takes the bundle-release branch (`platform/uefi/loader.c`).

## 2. Proposed handoff boundary

Proposal only. This note selects no binary layout, ABI, CPU baseline, media format, or C type. The UEFI loader remains the only producer shown here.

Proposed categories (semantic review input, not fields with assigned offsets):

- Identity and bounds: version/length and extensions, one payload identity, loaded ranges and entry bounds. Existing image base/size describe the complete EFI image, not an independently linked payload. A later build/load contract must define identity checks and relocation; matching address/size alone proves no payload identity.
- Memory: normalized classified physical ranges, access/retention constraints, owned/reserved code/data and explicit device exclusions. Copy/validate or retain all descriptor backing storage for the declared lifetime. UEFI descriptor encoding/stride/version, map key and raw exit diagnostics belong in the UEFI adapter or an optional tagged diagnostic extension, not mandatory common fields. Unknown categories remain unavailable. No reinterpretation of BIOS records as UEFI descriptors.
- Mappings: explicit accessible physical/virtual windows, overflow-safe translations, permissions and mapping ownership/lifetime. The current 0..2^47 identity-window declaration is an access bound, not installation or proof of page tables. Identify inherited mappings and any transition before touching payload/record/stack/device addresses.
- Display: live framebuffer base/size, dimensions, pitch units, accepted pixel format and access/cache constraints. Preserve existing bounds/overflow/disjointness checks; ordinary RAM composition remains separate from volatile device presentation. Keep the monochrome512x342 canvas centered and unscaled.
- CPU/stack/exceptions: declare established entry state, stack/storage reservations and what the common native entry must establish before devices. Existing x64 table setup uses bundle offsets and truthful identity mapping; its ready marker is produced by native setup, not made trustworthy by copying a firmware flag. Page-table ownership, interrupt state and fault paths require an explicit contract, not an assumed CPU baseline or current ready bit.
- Capabilities and lifetime: bounded validated discovery data (optional RSDP), device availability/unsupported reasons, ownership transfer and terminal failure policy. Vendor/revision strings remain optional self-report diagnostics, not capability evidence. Missing RSDP/PM timer or 8042 still fails current required-profile gates; absent alternatives are not implemented drivers.

Proposed readiness invariant: both producers must establish live/disjoint owned
resources, valid mappings and the chosen CPU entry conditions, and shared native
code must validate the common record before relying on them. UEFI must still
complete and verify its real exit protocol. BIOS must satisfy its own reviewed
handoff conditions; it must never synthesize `CS_UEFI_EXITED` or a zero EFI status
to pass current consumers. Keep UEFI exit key/retry/status logic in its adapter.
Native CPU setup has its own later readiness gate. Failure ownership/retention
must be explicit, including faults after ownership transfer. These are proposed
safety obligations, not claims the common validator or adapter already exists.

## 3. Proposed migration order

1. Retain this inventory and the current UEFI link evidence. No BIOS code. Released UEFI behavior stays the current PC behavior (`docs/development/unified-x64-pc-plan.md`).
2. Review and freeze a semantic handoff contract from section 2, including normalized memory and independent readiness. Payload format, CPU minimum, and media layout stay unchosen (`docs/development/unified-x64-pc-plan.md`).
3. Define the independently linked payload/build boundary and UEFI-to-common adapter, classifying current fields as loader-private, common invariants or optional diagnostics. Future implementation must preserve the existing UEFI behavior/refusals and prove that payload bytes can be reused; this planning step needs no BIOS recipe.
4. Wait for an in-repo review of published BIOS, media, and CPU interfaces. That review is a prerequisite and is not in these inputs.
5. Add a second entry only after that review. It must load the same payload bytes and either complete a real firmware handoff or stop with a defined failure.
6. Treat further display, USB, storage, and timing paths as later coverage (`docs/development/unified-x64-pc-plan.md`).

Paired-native-preview-2 remains the immediate checkpoint. BIOS work is a longer-term PC goal and is not a new requirement for that pair (`docs/development/unified-x64-pc-plan.md`).

## 4. Unresolved decisions and prerequisites

- In-repo BIOS, media, and CPU-interface review is missing. Boot-sector layout, interrupt numbers, and register recipes are out of scope until that source exists.
- Firmware bitness combinations are unaudited (`docs/development/unified-x64-pc-plan.md`). The checked ABI profile is x64-only (`platform/uefi/abi.h`).
- Display acquisition in these inputs is GOP mode data (`platform/uefi/loader.c`). Qualified profiles require a framebuffer (`docs/development/unified-x64-pc-plan.md`). No second display producer is shown.
- Timing and keyboard input, as called here, need a copied RSDP, a checked map, port I/O, a PM timer, and ACPI 8042 presence (`platform/uefi/keyboard.c`, `platform/uefi/entry.c`). The plan lists USB and other timing as later coverage (`docs/development/unified-x64-pc-plan.md`).
- The native window is physical 0, length 2^47, base 0 (`platform/uefi/entry.c`). The declaration does not create or verify mappings. A BIOS entry has no admitted equivalent. Table init still requires live identity-mapped addresses (`platform/pc/x64/state.h`).
- Who loads and relocates the payload, and which container is used, is undecided (`docs/development/unified-x64-pc-plan.md`). Today the loaded-image protocol supplies base and size (`platform/uefi/loader.c`) and the bundle is allocated below 2^47 (`platform/uefi/loader.c`).
- Reader kinds 0, 6, 9, and 10 are tied to the UEFI map walk (`platform/uefi/native.c`). A later normalized map needs reviewed category translation and ownership rules for each producer.
- Post-exit bundle ownership and the meaning of `native_entered` (`platform/uefi/entry.c`) are unspecified for any second firmware.
- This draft claims no BIOS completion, no new runtime, and no new test result.

## Lead disposition and next slice

Adopt the current dependency inventory with corrections: timer and keyboard
requirements differ, and transition is not exit proof. `cs_uefi_entry` transfers
after any recorded exit attempt, so the consumer failure gates remain essential
(lead check of `platform/uefi/entry.c`, `cs_uefi_entry`). Adopt common identity,
display, ownership and capability categories as proposals. Change the memory,
mapping and readiness categories as above; defer their binary ABI and all BIOS,
media, CPU-baseline and source recipes. Existing framebuffer/table helpers take
their own interfaces; their current callers use the UEFI record.

The immediate next release task remains the PC window boot-binding contract,
including RAM composition/volatile presentation and explicit packet/loss/fault
ownership, alongside the separate Mac M1-M7 hardware-source prerequisites. Design
that binding without adding UEFI dependencies to shared scene/input code. The
common payload contract is the first later unified-loader contract; no loader,
runtime header, build input, trial or release is changed by this inventory.

One Grok primary supplied the64-line/1090-word inventory; Codex integrated it,
corrected the categories and verified cited original code. No second provider,
retry, new qualification or worker tool/write/test occurred. Proposed tests are
not executed tests. Only documentation/preservation checks apply to this slice.
[Input identities and receipt](unified-x64-boot-inventory.json),
[worker brief](tasks/unified-x64-boot-inventory-grok.md),
[accepted goal](unified-x64-pc-plan.md).
