# ADR-0058 - Proposed first interactive native window release

Recorded2026-10-19,SRC-0307..0309. Status:recommended release goal;implementation not authorized by this planning record.

Target paired-native-preview-2 with original moving pointer/cursor,one draggable fixed-size native window and one content-click action. Shared portable C must compile natively for68000 and x86-64,retain logical512x342 and legacy regressions,and preserve released modes/assets. Propose ROM0.4.0-alpha.1/PC0.2.0-alpha.1 for this new visible capability;published version pointers stay0.3/0.1 until future publication.

Choose this over another static primitive because it delivers direct manipulation and reusable input/composition/window foundations. Cursor-only is a smaller fallback requiring a new owner decision;disk/System boot remains the long-term dependency goal. No general Window/Event/QuickDraw ABI,menus,files,native SDK or historical artwork is implied.

First propose the pointer/input contract and source-sufficiency inventory,then original core/cursor/window/redraw/adapters/qualification slices. Mac motion input needs an eligible published-source review beyond the existing button contract. Keep PC hosted dispatch separate from VM capture and unclassified GUI startup failure. Normalize logical pixels/state across formats/endian;do not infer human qualification from injection or automation. Record actual ROM/RAM costs and preserve64KiBROM target without claiming full128K System feasibility.

All cases are unexecuted. Future code,exact-image trials and publication retain applicable owner gates. [Plan](../../releases/paired-native-preview-2.md).
