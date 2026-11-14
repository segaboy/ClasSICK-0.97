# TEST-0185 - Four-track diagnostic loader bounds

2026-11-14; SPEC-0105 / IMPL-0132. Generated payloads exercise old and new
track boundaries and the maximum length; an oversized payload is rejected and
an intentionally shortened last read fails the byte comparison. The existing
286 interactive fixture still passes in BootHush. This is not physical or
release qualification. [Evidence](../../docs/development/mac-pc286-shared-parity-20261114.md).
