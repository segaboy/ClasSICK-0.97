# Normalized keyboard verification snapshot

Date: 2026-10-07. SPEC-0004 / IMPL-0005 / TEST-0007.
Implementation revision and remote result are pinned after committing the tested
source; pending publication at this draft stage. No remote success claimed yet.

Local invocation: Verify-Input.ps1 -BuildRoot <fresh-output-directory> -Sanitizers.
Five configurations pass 24/24 checks each; independent queue oracle explores
327,680 eight-operation traces. Sanitizer detection controls and optimized core
audits pass; ARM64 LE/BE compile-only checks pass. See [TEST-0007](../../provenance/records/TEST-0007-input.md).

Fresh Debug twins match the two input tests and updated viewer. The surface,
presentation-test and arena-test hashes retain their prior values. Hashes are
listed after the final complete local run; no archive/OS-image reproducibility,
historical compatibility or boot result is asserted. Timing and debugger remain
next work; B1/M0.1 and all three boot editions remain open.

| Fresh x64 Debug executable | Matching SHA-256 |
| --- | --- |
| classick_input_tests.exe | 2e87cd26b4a79bf7900203c485e8071dab2494e0750a967e42acc22c2ab450b8 |
| classick_windows_input_tests.exe | ff1328d879cb16ad279756162123fdf28637ee7cfccc814be798a742c6879c26 |
| classick_surface_demo.exe | 372aa50d3d8fb0cc5567e259b722e06d55b72ea1ac966a155059bafa19ba37f9 |
| classick_arena_tests.exe | 15a485fffd312f9221cf939cfc7dbaf0351a0a78ca7678f1e21ce3219f7ca6b5 |
| classick_surface_tests.exe | 6c44fa17d22f384f7a9e1837f887f39658df5adf30948fdc23a33b7ce4bcc667 |
| classick_presentation_tests.exe | 4cde2257f71a4ceec66fb6871a36ca5a5d452e342292a6c567eee8e06d35f01d |
