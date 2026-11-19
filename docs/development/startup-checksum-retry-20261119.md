<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Macintosh startup recovers from a transient sector checksum error

2026-11-19. SPEC-0114 / SPEC-0122 / IMPL-0144 / TEST-0203. The original
native Macintosh startup-files ROM abandoned startup-file preparation when a
single bad checksum arrived while it was opening the second resource map. The
startup reader now captures the same sector a second time after a decoded
checksum mismatch. In BootHush, a one-shot error recovered and both maps
opened and reopened in that same boot. Persistent corruption exhausted the
one retry, returned the original classified failure and preserved the
destination session and its guards.

| Generated disk condition at track 2, sector 3 (LBA 27) | Previous startup ROM | Revised startup ROM | Revised guest retry count | Altered nibbles, revised ROM |
| --- | --- | --- | ---: | ---: |
| Healthy | Both maps reopened | Both maps reopened | 0 | 0 |
| First delivered checksum altered | Second map refused | Both maps reopened | 1 | 1 |
| Every matching checksum altered | Second map refused | Second map refused | 1 | 2 |

Each column used a single ROM image across all three runs. The same 400 KiB
project-generated raw disk supplied every run. A controller can deliver a
corrupt nibble while guest code is searching for sector headers. The verifier
therefore checks a counter in guest RAM at 0x3084, the complete 16-word
resource report and all seven diagnostic words. The healthy and recovered
reports agree on two prepared and reopened sources, six types and eight
references. Both persistent failures identify resource-map position 66048.
The revised guest reached those outcomes in 100,617,286, 103,003,040 and
36,238,996 modeled cycles respectively.

`platform/mac128k/startup-floppy.[ch]` binds the actual IWM provider under a
startup-specific callback. A retry requires a fresh checksum mismatch from
that invocation. Each capture has its own bounded poll budget and publishes
only after checksum validation. Timeouts and unrelated failures return
directly. The underlying single-attempt primitive is also used by the
sector writer. The startup policy introduces no implicit write retry.
This is an original engineering decision based on the project provider
contract and BootHush [fault interface](https://github.com/segaboy/boothush/blob/cc182c2/docs/mac-floppy.md).
No third-party implementation, owner disk or original system code was used.

| Item | SHA-256 |
| --- | --- |
| BootHush `cc182c2` executable | `5fa45b7542409300b7604606a78c81d4ee95e888a57c592a83d0cec43ce1056e` |
| Prior independently built startup ROM | `94e95ba320b51d14ffd6fec646403b2135310fc94d9dfbe6946928169dea1fb1` |
| Revised independently built startup ROM | `9c0e660d4a033cbde00f084298a2a977d96bb9db50a1dc1511bcfa58e5be26be` |
| Generated startup disk | `79aa17dd27c2ab48d384f9bf5ee542a2ab3e9e477232ac6eefe204a1cd9099d8` |

Build the candidate with the usual `scripts/Test-StartupFiles.py` on generated
media. The final normal paired run passed three native Mac and three 286
checks using its pinned emulator, two hosted builds with 2,634 assertions each,
three rejected coordinator mutants and seven container controls. The revised
Mac payload uses 10,792 bytes before ROM padding (164 additional bytes) and
retains its declared RAM/stack limits. The build report includes the hashes
of the new provider source and header. Then run
`scripts/Test-StartupChecksumRetry.py --emulator EXE --single-attempt-rom OLD_ROM --candidate-dir BUILD --output NEW_DIR`.
The standalone verifier generates the media and verifies the exact control
ROM, emulator, fixture identity, candidate and diagnostic addresses.

The final external reports under `C:/Repos/.task-temp/` are
`startup-retry-build-20261119-b/report.json`
(SHA-256 `60c7e2d6cdda4e254a8cfbfb7e1acdd94ea04816a1d6dd1372d0f4f45509adef`)
and `startup-checksum-retry-20261119-b/report.json`
(SHA-256 `ab0535232886f435e28a191766ff5343f5a7d5835338f1a130a54462f62bb75b`).
Both reports record their inputs, commands and guest observations. This
qualifies the modeled read path through startup-file preparation. Physical
disk behavior, original System launch and R5 disk compatibility remain open.
