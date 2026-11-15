# TEST-0195 - Window cursor art validation

2026-11-15; IMPL-0137. An independent per-bit oracle checked 196,608
single-bit combinations, dimensions, hotspots and mixed valid patterns;
a deliberately incorrect mask was rejected. Hosted scene reference and
sanitizer checks passed. The rebuilt Mac and 286 diagnostics passed eight
modeled state/pixel comparisons and replay at the documented slower input
setting; the fast Mac case remains failing. Details, identities and limits are
in the [canonical evidence](../../docs/development/mac-pc286-shared-parity-20261114.md#cursor-art-validation-optimization).
