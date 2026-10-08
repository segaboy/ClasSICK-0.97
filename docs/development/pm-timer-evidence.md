# ACPI PM timer verification

Date 2026-10-08. SPEC-0010 / ADR-0014 / IMPL-0014 / TEST-0016. Hosted tests and an
unloaded EFI image only; no port, table or firmware access was executed.

Repeat with pinned tools and a fresh output directory:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Verify-PMTimer.ps1 -BuildRoot C:\ClasSICK\pm-timer-local -Sanitizers
```

Compiled/test/script source: `222a837e` (full revision in Git history).
[Windows CI 37810609408 passed](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37810609408).
The preceding [run 37810231122](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37810231122)
failed on a test-only LLP64 conversion and is retained as history. Clang x64 78
checks per Debug twin/Release/ASan+UBSan, i686 65; GCC x64 63, i686 50. 21 image
rejections and four omitted-object controls pass.

| Artifact/profile | SHA-256 (matching twins) |
| --- | --- |
| Clang ACPI tests Debug | `cda9978bc08796c37ef35ad984ce78edc5f91f7d582d4b81d260c9189c4d1926` |
| GCC ACPI tests Debug | `f341c5870710c272f2591e4b87acbe9ab958e73615b83ecb0b831475b2e99bbb` |
| Clang PM timer tests Debug | `3e1ed3ee87f463ba1411d29a05bee3c78708c6290ac02b07fd210f3981f50f77` |
| GCC PM timer tests Debug | `cce0b6bd46e747f98a35b5ce7158b5e1c1ab039acd66dea9512a5bc981d44969` |
| Clang native-timer tests Debug | `2499b3bd1124cc53284a03a639caab9cd6f5646706d35c44955c7727e92d1016` |
| GCC native-timer tests Debug | `4d5007b7009875f2cb446e94e215723f88b12282430dd7868508954a4937a1e4` |
| Clang loader tests Debug (new handoff field) | `8798991a975cbe7b4d795b2387e61fa41ad663986683f7d8ac79536d72abcbe8` |
| GCC loader tests Debug (new handoff field) | `9d8dca71e250ad926eb367ac572c86eeaefa92c6e134a419636c6a98e134c596` |
| Clang boot-scene tests Debug | `bcae81c41b9fcacc3e59da2fcf67ac84aecd7dde9d37a09aa8b6f1f2665b1709` |
| GCC boot-scene tests Debug | `3358842f5435741b98d820fae6d512725a2f3e5c432e6af6f4ce50170c73ab5d` |
| Clang native-gate tests Debug | `eeec99d3c5d23bc38e7b0c38c70104db655c04e6b2c1b102e0aa7bdab34cbb53` |
| GCC native-gate tests Debug | `f7a88237ff08bafe24e13ea68ad275f18988e93d85806019019178818bc2695d` |
| Clang x64 EFI O0 | `e856814182f8e2d4fe5bab92ef34328144a1c292d3f2aca09d0015bde42781ee` |
| Clang x64 EFI O2 | `040baf3975ee6944c07a1196b1f1818846d4692ad9335094f9b4e81ee1710baa` |

The 25 TEST-0014 fingerprints outside the loader and the two SPEC-0009
framebuffer-test fingerprints are unchanged. The EFI image is an unloaded payload.
No native timer reading, ACPI table observation, 60-second loop or B2 exists.
