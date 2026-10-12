# ADR-0023 - Share a bounded trap-address service directory

Accepted 2026-10-12, SRC-0088-0090 / SPEC-0020. The first common OS service
boundary must distinguish 32-bit guest addresses from host pointers and retain
explicit unsupported results. SPEC-0017 still has historical and CPU/stack ABI
questions; published manuals warn of calling-convention exceptions.

Implement a native value directory and two checked address services in portable
C. Both boot demonstrators link the same original probe. Symbolic service tokens
select intrinsic operations; arbitrary patch targets request guest execution,
which this milestone does not provide. Decode Toolbox words without guessing
their stack handling. Adopt only the stated later-manual rules as an engineering
profile; preserve the historical draft and its questions.

This makes a reusable manager boundary while retaining the native PC architecture
and the original Macintosh goal. It does not introduce an emulator core, Apple
ROM fallback, hidden unsupported success or a historical compatibility claim.
