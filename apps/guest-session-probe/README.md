# Original bounded session fixture

SPEC-0028 / TEST-0036. The independently authored 18-byte OS/core probe uses an
immediate-word push, pop, BSR and RTS. Hosted conformance observes seven steps,
the two-byte stack effect, stopped registers and complete caller restoration.
Standalone original-68000 assembly verifies bytes without running them.
No application file, CODE resource, native application or historical offset probe.
