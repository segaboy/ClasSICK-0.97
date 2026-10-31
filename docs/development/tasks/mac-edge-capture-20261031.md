<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Original Mac edge-capture worker brief

Baseline 40690d0a660e94277bde07c9339a3d253828a953. Frozen public original inputs below, each through EOF. Provide a bounded implementable patch and focused tests for an original 68000-compatible acquisition strategy, or explain a concrete blocker. Do not read other paths, use tools, commands, web, emulator source, manuals, binaries, private material or protected implementations. Do not write or run anything. Preserve SCC delays, exclusive ownership, ordering and loss detection; never invent the direction of an ambiguous transition. Default build and existing contracts stay unchanged unless equivalence is proven. The delivered patch must be a real exact artifact, not pseudocode. Distinguish engineering hypotheses from observed facts and tests_executed=[] from proposals. At most 450 lines.

Context: the paired release is blocked by real Mac motion. Timer divisor 4 idles without losses but with movement observed losses include Gray phase jump and unstable pair; divisor 8/16 still overrun with optimized sampling, even with less frequent trace copying. Human reported center jitter. The first loss is not a complete history; host injection can create large deltas. Hardware interface facts are limited to the included reviewed SPEC-0072 text. Need an acquisition change, not just more diagnostics, and a falsifiable bounded verification.

## Frozen inputs
- `platform/mac128k/motion.c`: 336 lines, SHA-256 `b2975851fddf2cf3df2cd6fd7edc013dc9a1ed50cb14c434afc1e45c9d067833`
- `platform/mac128k/motion.h`: 98 lines, SHA-256 `a618ed959e1c475924b18d21536ff2a8799d23719f11bb778252969caa782cba`
- `platform/mac128k/timer-poll.c`: 331 lines, SHA-256 `512b65a8ed3c7e5065be0e9e12158775fb99c3a39d4241ff41ac8695f5121fa3`
- `platform/mac128k/timer-poll.h`: 75 lines, SHA-256 `ffedac47dbd85ffadc7eae4776445dd60d1ba59dd3f48fcf5d48afa12c0915a5`
- `platform/mac128k/polled-phase.c`: 90 lines, SHA-256 `537d6d62bf3177e16153c86ba4d42fee90a90192bf17e103fc635f5bf1518766`
- `platform/mac128k/polled-phase.h`: 32 lines, SHA-256 `889aa6a1314e32114722f8ef822de14b100f197927fc77b9768561ac1566428f`
- `apps/window-scene/inbox.c`: 253 lines, SHA-256 `24939209ed957f9c6c796b60eeee518cfacdd7d0c9ed1d43cfba216fc60db8f4`
- `apps/window-scene/inbox.h`: 59 lines, SHA-256 `2bebf4901506814c270c4612b9170ad0dce2da58a037fa4e187f8a748bc3fa90`
- `tests/mac-timer-poll.c`: 288 lines, SHA-256 `a20825530e118e9e7a09fd8182f870ec4a39d72fa95b3b3d0bf8097abd473e75`
- `docs/specifications/SPEC-0072-mac-window-hardware-binding.md`: 221 lines, SHA-256 `0f70a79c790994205caccd0e07447ead82072d46913ae4ed928d059ae1dc0863`
