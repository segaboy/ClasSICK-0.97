# TEST-0190 - 286 whole-disk diagnostic model checks

2026-11-14; SPEC-0107 / IMPL-0134. Original generated payloads exercise track
boundaries, BSS clearing, corrupt-payload refusal, an invalid fourth read, and
a documented additive-checksum collision. The actual shared window diagnostic
also boots from the generated model disk and matches hosted and Mac state/pixel
checkpoints. BootHush observations do not establish physical disk or BIOS
behavior, installation, production boot or release qualification.
[Evidence](../../docs/development/mac-pc286-shared-parity-20261114.md).

The later BootHush built-in BIOS injector failed each of the four generated
maximum-payload reads and each of the actual session image's three reads.
The expected failure stop, prior-byte retention and untouched failing byte
were observed with unchanged media; this remains modeled fault evidence.

2026-11-15: 29 loader cases and a full eight-state CRC disk session passed;
the equal-and-opposite modification missed by the additive sum was rejected.
The initial input schedule preceded CRC startup and was corrected after a
separate readiness observation. [Evidence](../../docs/development/mac-pc286-shared-parity-20261114.md).
