# TEST-0191 - Disposable-disk copy and boot in BootHush

2026-11-15 follow-up: an opt-in generated-media prefix preflight, failure
cases and an out-of-prefix limitation control are recorded in the
[canonical evidence](../../docs/development/mac-pc286-shared-parity-20261114.md#disposable-copy-prefix-preflight).
The test does not qualify an installer or physical disk safety.
The later four-track follow-up tested 52- and 58-sector generated prefixes,
guarded and unguarded, and fourth-track faults; short-profile bytes were
preserved. See the same evidence record for exact commands and limitations.
The later write-retry experiment exercised transient and persistent modeled
write failures on each track plus a no-retry control. Its reset-failure and
physical-media paths remain untested. [Evidence](../../docs/development/mac-pc286-shared-parity-20261114.md#disposable-copy-write-retry).

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
The same copy and fault controls later passed with the new CRC-checked
diagnostic; the source and copied disk identities are in the linked evidence.
The same controls subsequently passed with the optional model-geometry check
and CRC both enabled; this remains a generated-disk experiment.
The copy and fault controls were repeated successfully on the rebuilt
active-profile BootHush twin, as recorded in the same evidence.
The copy and fault controls also passed with the retry-enabled diagnostic.
They passed again for the version that saves the retry counter across modeled
firmware calls; the linked evidence identifies the image and commands.
