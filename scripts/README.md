# Bootstrap scripts

All scripts target built-in Windows PowerShell 5.1 and keep generated tools/output
out of public commits.

- `Inventory-Windows.ps1`: sanitized PATH/common-location capability inventory.
- `Setup-Windows.ps1`: prepare SHA-256-pinned portable tools; optional `-Offline`.
- `Enter-DevEnvironment.ps1`: activate prepared tools in the current process only.
- `Verify-Bootstrap.ps1`: two new native hosted builds, CTest and executable hashes.
- `Test-Repository.ps1`: audit indexed text/source, required docs, local Markdown
  links, JSON, common credentials and forbidden payload directories/extensions.
- `toolchain-lock.json`: exact package URLs, digests, versions and purposes.

Run [the Windows procedure](../docs/development/windows.md). No script establishes
clean-room originality, lawful reference use, or native boot by itself.
