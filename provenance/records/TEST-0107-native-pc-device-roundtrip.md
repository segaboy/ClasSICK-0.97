# TEST-0107 - Native PC device-control roundtrip

2026-10-29,SRC-0548,SPEC-0062. Twelve admitted public-device API events on one verified normal PC candidate. Exact raw framebuffer checks:separate X/Y response,held20/12drag,release with no pixel change,unheld8X movement changes only expected cursor rectangle,return equals the complete initial frame except a frozen original3x5counterglyph0002. Both initial0000/final0002glyphs checked pixel by pixel. The first naive full-frame equality assertion failed on the expected counter change;comparison was corrected explicitly,without retouching captures.

Captured host GUI route is separately refused before events at the physical-containment guard. No E3/E4 pass is claimed. [Canonical identities,events,actual outcomes and cleanup](../../docs/development/release-input-repair.json).
