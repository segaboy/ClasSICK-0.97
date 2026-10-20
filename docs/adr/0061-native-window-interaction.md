# ADR-0061 - Original native fixed window interaction

Accepted2026-10-20;SRC-0323..0325;SPEC-0053.
Use one caller-owned44-byte state with configurable fixed geometry,half-open
title/content hit areas,and persistent grab offsets while clamped. Apply final
drag movement before release;cancel never moves. Toggle once per accepted content
PRESS. Refuse invalid/coherence inputs atomically,without upstream rollback.
No raw-input resampling,rendering,allocation,callback or guest ABI is introduced.
The later compositor/owner must provide dirty repair and coherent distribution.
[Contract](../specifications/SPEC-0053-native-window-interaction.md).
