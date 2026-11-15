# TEST-0191 - Disposable-disk copy and boot in BootHush

2026-11-14; SPEC-0108 / IMPL-0135. An original guest wrote and read back a
45-sector diagnostic prefix on a newly generated blank disk. The entire saved
disk matched the intended image and booted to the shared-window initial state.
Injected write and read failures reached distinct stops; the former preserved
the blank modeled disk and the latter retained the preceding completed write.
A changed input produced a different disk that the host identity check
rejected. This is model evidence, not production or physical qualification.
[Evidence](../../docs/development/mac-pc286-shared-parity-20261114.md).

2026-11-15: three track-bounded writes and three track-bounded reads replaced
the test guest's single cross-track operation. Each operation was separately
failed and the entire resulting disk compared against the expected partial
state. A separate guest-side readback mutation reached the mismatch stop. The
successful generated disk still matched the same identity and booted.
