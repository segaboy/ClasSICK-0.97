# Bootstrap scripts

All scripts target built-in Windows PowerShell 5.1 and keep generated tools/output
out of public commits.

- `Inventory-Windows.ps1`: sanitized PATH/common-location capability inventory.
- `Setup-Windows.ps1`: prepare SHA-256-pinned portable tools; optional `-Offline`.
- `Setup-SecondCompiler.ps1`: prepare optional pinned GCC/GNU ld without system
  changes; supports `-Offline`. Package and executable digests are in
  `gcc-toolchain-lock.json`.
- `Verify-CoreLink.ps1 -BuildRoot <fresh-directory> -Sanitizers`: full Clang
  matrix/debugger replay, GCC core-only matrix and complete standalone link twins
  at two widths/optimizations. Includes unresolved-helper and image rejection controls.
- `Test-LinkImage.ps1`: bounded original PE/map auditor for unloaded link fixtures;
  checks the actual entry address, retained core symbols and import/library boundary.
- `Verify-UEFIContracts.ps1 -BuildRoot <fresh-directory> -Sanitizers`: complete
  Clang/GCC/link/debugger matrix plus preboot data/model behavior, executable twins
  and ARM64 endian compile/import audits. Never launches firmware or a VM.
- `Verify-UEFILoader.ps1 -BuildRoot <fresh-directory> -Sanitizers`: full prior
  protocol plus x64 loader callback behavior, twin executables and EFI image audit.
- `Verify-X64Exceptions.ps1 -BuildRoot <fresh-directory> -Sanitizers`: full loader
  protocol plus descriptor-byte suites at both widths and new Clang/GCC twins;
  current EFI audits cover owned installation/256 vectors/terminal fault capture.
- `Verify-NativeFramebuffer.ps1 -BuildRoot <fresh-directory> -Sanitizers`: the
  complete x64 exception wrapper, plus SPEC-0009 hosted presenter/scene/gate twin
  hashes and AArch64 LE/BE compile/import audits. The EFI image now links the
  presenter and must reject an omitted presenter object. No image is executed.
- `Verify-PMTimer.ps1 -BuildRoot <fresh-directory> -Sanitizers`: the complete
  native framebuffer wrapper, plus SPEC-0010 ACPI/PM timer/native-probe/loader twin
  hashes and AArch64 audits. The image audit checks the exact port-read bytes and
  rejects an omitted ACPI object.
- `Check-ObjectImports.cmake`: freestanding object audit with an exact list of
  permitted original project imports and no mutable data.
- `Verify-UEFIImage.ps1 -BuildRoot <fresh-directory>`: inspection-only Clang x64
  O0/O2 EFI twins, corruption/missing-transition controls and original payload tree.
- `Test-UEFIImage.ps1`: bounded original EFI section/map/relocation/entry/stack/halt
  audit; `Check-LoaderObject.cmake` permits only our preboot helpers and no mutable
  globals. Neither script loads firmware or executes the generated EFI code.
- `Enter-DevEnvironment.ps1`: activate prepared tools in the current process only.
- `Verify-Bootstrap.ps1`: two new native hosted builds, CTest and executable hashes.
- `Verify-Surfaces.ps1 -BuildRoot <fresh-directory> -Sanitizers`: x64 debug twins,
  release, x86 execution, ARM64 endian compile/import checks, validated x64 ASan/UBSan.
- `Check-Freestanding.cmake`: reject core object undefined symbols/global data.
- `Check-Hosted-EarlyExit.cmake`: require early Escape to reject incomplete
  acceptance despite clean shutdown. Normal combined startup is `hosted.start`;
  operator validation and gate scope are in [the B1 audit](../docs/development/hosted-start-audit.md).
- `Verify-Presentation.ps1`: established matrix plus viewer/presentation hash checks.
- `Verify-Arenas.ps1`: full matrix, arena executable hash and ARM64 endian arena
  compile/import checks. Sixteen CTest checks per Windows configuration.
- `Test-Repository.ps1`: audit indexed text/source, required docs, local Markdown
  links, JSON, common credentials and forbidden payload directories/extensions.
- `toolchain-lock.json`: exact package URLs, digests, versions and purposes.

Run [the Windows procedure](../docs/development/windows.md). No script establishes
clean-room originality, lawful reference use, or native boot by itself.
