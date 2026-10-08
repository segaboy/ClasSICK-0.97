# IMPL-0017 — Bounded polling UART diagnostics

2026-10-08. SRC-0053/0054, reused SRC-0046/0052; SPEC-0013 / ADR-0017 /
TEST-0019. OpenAI Codex (GPT-6) fresh implementation agent assisted with original
C, synthetic tests, CMake, PowerShell and documentation. Implementation-agent
self-review; human provenance review pending. GPL-3.0-or-later. No independent
staffed or legally certified clean-room claim.

Primary TI interface was directly reviewed and selected tables visually checked;
SPEC-0013 recorded its scoped facts and original contract before implementation.
The source catalog retains eligible scope and adjacent exposure. No
search engine, external driver/header/code sample, Apple executable/ROM/source,
uncertain firmware body or protected fixture/asset input was used. Existing owned
contracts/source/build patterns and approved byte-port instructions/timer reused.

Paths: platform/pc/uart.h/.c; platform/uefi/uart.h/.c, keyboard.h/.c observer and
entry.c; tests/uart.c, uart-fixture.h, native-uart.c; CMake, UART wrapper, EFI
audits and workflow. Original caller-owned ring/phase transport makes bounded
startup accesses, atomic records, saturated loss counts, sticky failures and
THRE/TEMT state visible. Small original ASCII formatter needs no runtime.
Native observer retains the existing keyboard function and behavior; diagnostics
service one poll per timer turn. UART errors continue keyboard work, and final
drain is finite. Trace 256..895 fits the existing bundle; previous records persist.

Target assumptions are explicit: compatible TI semantics, scratch/FIFO/readback,
0x3F8 decode, 1.8432-MHz clock, exclusive ownership and bounded access latency.
No hardware identity, VM model, native delivery, asynchronous serial fault output,
firmware eligibility, media, B2 or edition claim. Macintosh/mini vMac remain
independent required goals. Source/provenance review covers every staged byte;
compiled/test/script revision `3b44cb9510a400397a33cd1cc0802f45c36c8110`
passed CI 37827697279 and all 53 local/CI fingerprints match. The exposed lead's
follow-up records aggregate results only; no implementation/specification/test
changes follow the fresh agent's reviewed checkpoint. Human review stays pending.
