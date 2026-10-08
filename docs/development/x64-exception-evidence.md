# Owned x64 exception scaffold verification

Date 2026-10-08. SPEC-0008 / ADR-0012 / IMPL-0012 / TEST-0014.
Original descriptor serialization and native terminal assembly. No native execution.

Repeat with pinned tools and a fresh output directory:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Verify-X64Exceptions.ps1 -BuildRoot C:\ClasSICK\x64-exception-local -Sanitizers
```

The wrapper retains complete prior hosted/core/debugger/sanitizer/link/endian
verification, adds serializer checks and Clang/GCC hosted twin hashes, and audits
four own EFI images including descriptor installation and every vector/fault path.
The first development checks pass; complete immutable-source verification pending.

The image is an unloaded terminal scaffold. Real descriptor installation/fault
delivery, native devices/event loop, firmware eligibility, formatted media and B2
remain unverified. The original Macintosh/mini vMac and all physical edition gates
remain independent. No OneNote publication or external firmware adoption occurs.
