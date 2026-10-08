# Native framebuffer presentation verification

Date 2026-10-08. SPEC-0009 / ADR-0013 / IMPL-0013 / TEST-0015. Hosted tests and an
unloaded EFI image only; nothing was executed natively.

Repeat with pinned tools and a fresh output directory:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Verify-NativeFramebuffer.ps1 -BuildRoot C:\ClasSICK\native-framebuffer-local -Sanitizers
```

Compiled/test/script source: `c6289e1a5c404494b6b4d39e20ea23bae2f45887`.
[Windows CI 37806764756 passed](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37806764756)
(workflow dispatch on that exact commit before `main` advanced). Clang x64 66
checks per Debug twin/Release/ASan+UBSan, actual i686 57; scoped GCC x64 51 per
Debug twin/Release, actual i686 42. Debugger, sanitizer, core, endian and link
controls and all TEST-0014 image controls pass, plus an omitted-presenter link
rejection. 27 earlier named fingerprints are byte-identical to the TEST-0014 local
evidence.

| Artifact/profile | SHA-256 (matching twins) |
| --- | --- |
| Clang framebuffer tests Debug | `d7512df5106e1ff97b42c70d63c8fc057d4288e2f12cab9166b777edf8152b55` |
| GCC framebuffer tests Debug | `9152124daea5151e2048b122492ff60e2c39b0b2e33e1a869b2ddcdf7234f243` |
| Clang boot-scene tests Debug | `fcb810d10e181b7d1ad3d8e4a5c12bbf0573943efddc08774ee129aa5a9da2f1` |
| GCC boot-scene tests Debug | `5ad6a8d9728bee01c36418d59cb1c52a675acf48c31f356e6f0471534ebb9647` |
| Clang native-gate tests Debug | `086c8fe3716670dfea6ab4afd1238d0945360042b86e46a81f2185d2ac062a20` |
| GCC native-gate tests Debug | `f9ee96f19552a2ca184af47cc4a38c673399cb70855821510364d22ab8ba23cd` |
| Clang x64 EFI O0 | `5b934e1158e888546b58355af304a5c8cc361b9c596ea5dd749c6cdc79e24865` |
| Clang x64 EFI O2 | `aba8e1cdfdd43db84f06a91fb5c83ffa96f8ec68d9c3b24eb3e311277ba960b2` |

The EFI image keeps three DIR64 relocations and exactly one 8-byte writable slot.
It links 34 named original symbols with zero imports or runtime libraries. It is
an unloaded payload, not formatted media or a boot. The local owner-workstation
replay is optional. No native framebuffer write, timing, keyboard, firmware, VM,
B2 or edition result exists.
