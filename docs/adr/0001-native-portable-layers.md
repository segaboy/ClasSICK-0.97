# ADR-0001 — Native portable layers

- Date: 2026-10-07
- Status: accepted
- Owner: Project HQ under founding requirements SRC-0001

## Context

Macintosh 128K is one target; x86-64/ARM64 must execute native ClasSICK code.
Legacy application instructions can require translation. No Apple code is permitted.

## Decision

Separate portable core, System 1 personality, architecture primitives, platform
devices, and optional 68000 application execution. Trap entry converges on the
same personality implementation for native and translated applications. Keep
room for future personalities without implementing them now.

## Alternatives

Whole-Macintosh emulation as the OS foundation violates native portability.
A hardware-specific core makes portability untestable. A single universal
hardware API hiding guest behavior would mix compatibility with device policy.

## Consequences and verification

Explicit contracts and adapters cost design effort. Build dependency/import audits
must prove core isolation; same contract tests run on multiple native architectures
and hosted/bare-metal adapters. Limited hardware may omit optional services but
must report them honestly. Native 68000 does not guarantee application isolation.

## Review conditions

Refine interfaces when tests or size measurements demand it. Native OS execution,
zero Apple code, and clean provenance remain foundational constraints.
