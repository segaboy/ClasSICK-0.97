# Hosted debugger verification snapshot

Date 2026-10-07. IMPL-0007 / TEST-0009, existing SPEC-0002–0005.
Tested source pin follows the implementation commit. Remote CI pending.

Local matrix: five configurations, 34/34 checks each; validated sanitizer controls,
optimized core imports/global-data and ARM64 LE/BE compile-only audits pass. Six
valid x64 LLDB sessions and three invalid-argument rejections pass against fresh
Debug twins. Local interactive core launch/state/step/resume also passed. Original
raw viewer delimiter failure was fixed; optional WOW64 inspection failure remains
outside the verified scope. Existing failure records are retained in TEST-0009.

| Fresh x64 Debug executable | Matching SHA-256 |
| --- | --- |
| classick_surface_demo.exe | 7461531a1ef27ee343d2f79ca726bcb24629671f96739a4ab09bf2f81e34f4ac |
| classick_clock_tests.exe | 03c4e9abf6a75459113d28f255d97f48e0ab2b0d42ac90ca7f36e2e3d8224fc2 |
| classick_windows_clock_tests.exe | 2aa6fd9400a490975bc34ad9c70116a093dfdf06c5f32c83a1f3a8c4c82e0a95 |
| classick_input_tests.exe | 2e87cd26b4a79bf7900203c485e8071dab2494e0750a967e42acc22c2ab450b8 |
| classick_windows_input_tests.exe | ff1328d879cb16ad279756162123fdf28637ee7cfccc814be798a742c6879c26 |
| classick_arena_tests.exe | 15a485fffd312f9221cf939cfc7dbaf0351a0a78ca7678f1e21ce3219f7ca6b5 |
| classick_surface_tests.exe | 6c44fa17d22f384f7a9e1837f887f39658df5adf30948fdc23a33b7ce4bcc667 |
| classick_presentation_tests.exe | 4cde2257f71a4ceec66fb6871a36ca5a5d452e342292a6c567eee8e06d35f01d |

These named executables/pinned tools only. No general tool certification, static
archive/OS-image reproducibility, historical compatibility, B1 or boot claim.
Git wiki plans remain queued until requested; no live OneNote work for this milestone.
