# TEST-0169 - 286 diagnostic loader failure branches

2026-11-14; SRC-0607 and the original diagnostic loader in
`scripts/Test-BootHushPC286BankPlan.py`. BootHush injected failures separately
on both track reads of a 7,169-byte payload; the guest reached `EE`, expected
RAM contents matched, and floppy images remained unchanged. Four normal payload
sizes and four invalid input controls also passed. This is not an installed 286
OS or physical-hardware test. [Receipt](../../docs/development/boothush-paired-20261114.md).
