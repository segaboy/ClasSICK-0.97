# IMPL-0031 - Original bounded resource-fork reader

2026-10-13; SPEC-0024 / ADR-0028 / TEST-0032 / SRC-0106/0126/0127.
Codex-assisted original code from the finalized contract and its prior published
format review. No new manual, Apple/Microsoft implementation, executable file,
resource, disk image, font, asset or emulator source was consulted. Original
algorithms and fixtures only; lead self-review, human provenance review pending.

formats/resourcefork/reader.c/.h is a borrowing internal OS/core service over
immutable caller bytes. It checks uint32 arithmetic and file regions before
publishing a view, then validates all ordered reference lists before entry names
and local length/payload spans. Exact type/signed-ID lookup rejects requested-key
ambiguity while preserving unrelated lookups. Bit7 returns opaque reference
metadata without reading its ignored local-data position. Bytewise big-endian
decoding imposes no native input alignment or packed-struct assumptions.

Only constructor-produced immutable views are admitted; native objects and
storage are truthful and disjoint. Refusal preserves view/entry/input, argument
refusal preserves report, and successful outputs publish complete fields once.
Explicit field assignments avoid compiler runtime helpers. No allocation, host
I/O, mutable global state, callback, data-content scan or native-pointer decoding.
Warnings and work counters are bounded diagnostics, not a full strict audit.

tests/resourcefork.c supplies an independently written declarative encoder,
fourteen small variants/truncation sweeps, malformed/error-precedence cases,
duplicates/retention/unaligned input and generated large/dense files. The delivered
scripts/Test-ResourceFork.py supplies a separate arbitrary-precision model and
encoder;113664 mutations compare open diagnostics and lookup metadata, not merely
absence of crashes. Test outputs are generated outside Git. No draft checker or
manual examples are copied; Claude's research originals remain unchanged.

CMake and compiler/CI verifiers check hosted portability, sanitizers, object
imports, deterministic twins and the standalone original68000 closure/storage.
This header is internal core plumbing, not an application SDK. Resource Manager
loading/search chains, optional strict audit, MFS, CODE launch, callbacks, boot
binding, native applications and new VM/emulator/reference trials remain open.

Accepted exact revisions, outcomes and limits:
[reader evidence](../../docs/development/resource-reader-evidence.md).
