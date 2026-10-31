# TEST-0125 - Automated PC GUI first-byte observation

2026-10-31; SRC-0577 / SPEC-0085. A source-matched three-byte recorder captured 2A FF 01 after automated capture and a one-pixel SKY drag; the offline model yields dx +255, dy -255 and button 2 if aligned. Initial and first-capture snapshots did not reach the stop. Capture and gesture share the interval; neither exact attribution nor packet alignment is proven. The own VM, prior inventory and media were preserved. [Exact observation and limits](../../docs/development/pc-sequence-evidence.json). No human, physical, dragging or release pass.

A second isolated short-recorder run confirmed capture but the existing guarded helper refused with the Windows-reported cursor outside the owned client; it attempted zero events. Its recorder stop was not reached and no byte count is inferred. Inventory and media were preserved. See the same canonical receipt.
