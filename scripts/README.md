# Bootstrap scripts

All scripts target built-in Windows PowerShell 5.1 and keep generated tools/output
out of public commits.

- `Inventory-Windows.ps1`: sanitized PATH/common-location capability inventory.
- `Setup-Windows.ps1`: prepare SHA-256-pinned portable tools; optional `-Offline`.
- `Enter-DevEnvironment.ps1`: activate prepared tools in the current process only.
- `Verify-Bootstrap.ps1`: two new native hosted builds, CTest and executable hashes.
- `Verify-Surfaces.ps1 -BuildRoot <fresh-directory> -Sanitizers`: x64 debug twins,
  release, x86 execution, ARM64 endian compile/import checks, validated x64 ASan/UBSan.
- `Check-Freestanding.cmake`: reject core object undefined symbols/global data.
- `Verify-Presentation.ps1`: established matrix plus viewer/presentation hash checks.
- `Verify-Arenas.ps1`: full matrix, arena executable hash and ARM64 endian arena
  compile/import checks. Sixteen CTest checks per Windows configuration.
- `Test-Repository.ps1`: audit indexed text/source, required docs, local Markdown
  links, JSON, common credentials and forbidden payload directories/extensions.
- `toolchain-lock.json`: exact package URLs, digests, versions and purposes.

Run [the Windows procedure](../docs/development/windows.md). No script establishes
clean-room originality, lawful reference use, or native boot by itself.
