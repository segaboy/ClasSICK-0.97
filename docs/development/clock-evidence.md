# Portable clock verification snapshot

Date 2026-10-07. SPEC-0005 / IMPL-0006 / TEST-0008. Tested code/test/build inputs:
`958e4794381e6dae25556a5c2796b1ad8c461e04`. This subsequent text-only record pins
that immutable implementation. Evidence/CI source:
`f814fd48991e14a2fd8ea0860471aa11d6c3dac1`.
[Windows CI 37722447834 passed](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37722447834)
at that source. Observed guard/setup/bootstrap twins and full five-configuration
34-check matrix pass, including sanitizer controls and endian core audits. All
eight named executable hashes below match local and remote results. No code/test/
build input changed after the implementation commit; this result is text-only.

Verify-Clocks.ps1 -BuildRoot <fresh-output-directory> -Sanitizers: five
configurations pass 34/34 checks each. Independent 5,184-pair arithmetic oracle,
thirteen conversion fixtures, injectable provider faults, live QPC and fake/live
hidden viewer deadlines/rendering/timers pass. Sanitizer detection controls and
optimized imports/data audits pass; ARM64 endian checks are compile-only.

| Fresh x64 Debug executable | Matching SHA-256 |
| --- | --- |
| classick_clock_tests.exe | 03c4e9abf6a75459113d28f255d97f48e0ab2b0d42ac90ca7f36e2e3d8224fc2 |
| classick_windows_clock_tests.exe | 2aa6fd9400a490975bc34ad9c70116a093dfdf06c5f32c83a1f3a8c4c82e0a95 |
| classick_surface_demo.exe | cbc9bcd688af47e0c8007a0cc9e41bae7369fe721514f1c67a8bd42a704e219e |
| classick_input_tests.exe | 2e87cd26b4a79bf7900203c485e8071dab2494e0750a967e42acc22c2ab450b8 |
| classick_windows_input_tests.exe | ff1328d879cb16ad279756162123fdf28637ee7cfccc814be798a742c6879c26 |
| classick_arena_tests.exe | 15a485fffd312f9221cf939cfc7dbaf0351a0a78ca7678f1e21ce3219f7ca6b5 |
| classick_surface_tests.exe | 6c44fa17d22f384f7a9e1837f887f39658df5adf30948fdc23a33b7ce4bcc667 |
| classick_presentation_tests.exe | 4cde2257f71a4ceec66fb6871a36ca5a5d452e342292a6c567eee8e06d35f01d |

One pinned compiler's named executables only; COFF archives/OS images/general
cross-machine reproducibility remain unproven. No clock accuracy, historical
compatibility, debugger/B1 or boot certification. VirtualBox version availability
was observed; no VM was created, altered, inspected or started for this milestone.
