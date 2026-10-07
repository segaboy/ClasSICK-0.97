# Bounded arena verification snapshot

Date: 2026-10-07. SPEC-0003 / IMPL-0004 / TEST-0006.
Executed code/test/build inputs are frozen at
`360c79d216b4262ff61310b35c72852603176777`. The local matrix used identical
implementation, tests, scripts and build definitions before that commit.
Subsequent source-pointer/CI text updates do not change those inputs.
Remote [Windows CI run 37699177939 passed](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37699177939)
at evidence revision `09907fb471484d2feafaea3fad269a53a5af748b`. The guard,
pinned setup, bootstrap twins, full five-configuration matrix, sanitizer detection
controls and ARM64 endian import checks pass. CI artifact hashes below match local
fresh-twin hashes. No implementation or build inputs changed after 360c79d.

Local invocation: Verify-Arenas.ps1 -BuildRoot <fresh-output-directory> -Sanitizers.
Five configurations pass 16/16 CTest checks each; x64 detection controls and ARM64
little/big-endian compile/import checks pass. See [TEST-0006](../../provenance/records/TEST-0006-arenas.md).

| Fresh x64 Debug executable | Matching SHA-256 |
| --- | --- |
| classick_arena_tests.exe | 15a485fffd312f9221cf939cfc7dbaf0351a0a78ca7678f1e21ce3219f7ca6b5 |
| classick_surface_demo.exe | de2958593ad1a1e766b2d6812ceee0bd381a48ffc45054c1a522e40c2d583019 |
| classick_surface_tests.exe | 6c44fa17d22f384f7a9e1837f887f39658df5adf30948fdc23a33b7ce4bcc667 |
| classick_presentation_tests.exe | 4cde2257f71a4ceec66fb6871a36ca5a5d452e342292a6c567eee8e06d35f01d |

Only the allocator-test and viewer artifacts are new. Old surface and adapter-test
hashes are retained. This checks named fresh executables with one pinned compiler;
it does not establish archive/OS-image/general cross-machine reproducibility.
B1 remains partial; every boot edition and historical/mini vMac compatibility
remains unverified. No reference binaries, copied assets or release binary added.
