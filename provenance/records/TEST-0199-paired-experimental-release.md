# TEST-0199 - Paired experimental release qualification

2026-11-17; SRC-0611; existing SPEC-0096/0109 and TEST-0197/0198.
The original `scripts/Test-MacReleaseInputProfile.py` checks decoded motion,
loss/omission/fault counters and FIFO completion, including the retained failing
2048-cycle trace as a negative control. 46 checks pass at their stated expected
outcomes. Fresh 286 native/media qualification and eight-state pixel/replay parity
pass with the exact current BootHush executable. Final packaging verifies the
unchanged reviewed images, corresponding committed source and GPL notices.
The [canonical receipt](../../docs/development/paired-release-20261117.md)
states identities, retained failures, packaging controls and remaining limits.
No physical or human response pass is inferred; no guest semantics changed.
