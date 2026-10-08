# Timed progress loop verification

Date 2026-10-08. SPEC-0011 / ADR-0015 / IMPL-0015 / TEST-0017. Hosted tests and an
unloaded EFI image only.

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Verify-ProgressLoop.ps1 -BuildRoot C:\ClasSICK\progress-loop-local -Sanitizers
```

Compiled/test/script source: `c82abe79a31da20e1b325bb17e0f4fbd999122ba`.
[Windows CI 37812572045 passed](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37812572045):
Clang x64 81 checks per configuration, i686 65; GCC x64 66, i686 50.

| Artifact/profile | SHA-256 (matching twins) |
| --- | --- |
| Clang loop tests Debug | `dc1be5ed689dca40ef6c3f68b582f0ac1bc44a9cef953b49d52da941114e00c7` |
| GCC loop tests Debug | `54029266a3ce8ad22d94f4d1295d6467a1a29dd866161f911d6c084db9676881` |
| Clang native-timer tests Debug | `5cafd356d624eeb5bdfb1612f68eaace8a8c3c43551051f1b186eb1a5b49cea3` |
| GCC native-timer tests Debug | `a974477722f4c7cbe9483274994d846149827cbda8a051c674dc154b491fcb01` |
| Clang native-gate tests Debug | `82ed9fccd80b2e7d3ea6e6b4cb43d7558b8f218f111005752109f6b7b49f2cc1` |
| GCC native-gate tests Debug | `12d19d2b43ddea77ae7d0a27a6064ae01a051faefc34f1ca598d7fda93889b23` |
| Clang x64 EFI O0 | `41e4c094e30037c5bc97b89f58ceeddbd0cb416286d7591678260d59135f616d` |
| Clang x64 EFI O2 | `c4068287a79d1cbd5dcf9e2bf9e82596dcdfda3385de53afd97c794fc0f29c4d` |

All other TEST-0016 fingerprints are unchanged, including the loader, ACPI, PM
timer, framebuffer and scene tests. The EFI image now runs present, probe, then
a 60-second loop, then halts; it has never been executed.
