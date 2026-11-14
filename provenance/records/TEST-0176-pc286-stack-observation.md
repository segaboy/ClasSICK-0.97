# TEST-0176 - Modeled 286 window stack observation

2026-11-14; IMPL-0126 / IMPL-0125. Two stack fill patterns and full gap
inspection show a 1,056-byte touched span during one modeled eleven-key window
sequence, with matching final pixels. This is a lower-bound observation, not
a worst-case stack budget or physical qualification.
[Evidence](../../docs/development/mac-pc286-shared-parity-20261114.md).
