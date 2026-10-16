# ADR-0026 - Shared application memory and 68000-first foundations

2026-10-16 scope correction: [ADR-0043](0043-coupled-native-mac-pc-parity.md)
supersedes this record's blanket wait for complete68k experience before native
PC application/test/loader work where it blocks parity. Shared memory/interface
foundations and transparency remain. The text below records the2026-10-12 decision.

Accepted 2026-10-12 for the allowed 68000 foundations; SRC-0099.
Native-application implementations remain gated on the owner's explicit
confirmation that the 68k experience is complete. This is an architecture
decision, not a new runtime implementation or a declaration of completeness.

Adopt the research's D1, D2 and D4 only within the permitted scope:

- One application-visible memory model: fixed-width logical guest addresses,
  explicit big-endian records and a versioned address-width policy. The current
  SPEC-0022 storage remains strict; 24-bit data-bus normalization and flag-byte
  handling need their own reviewed contract and tests. Shared native side tables
  must fit the actual Macintosh RAM budget.
- One original semantic implementation per routine. A reviewed machine-readable
  registry describes guest parameters, results, preserved registers, record
  fields and applicable profiles. Generate or check the 68000 front ends and
  record parsing from it. No native application headers or SDK are generated.
- Variant selection is a distinct stage above format loading. Its implementation
  knows only the 68000 CODE path now and records the selection/failure reason.
  No native descriptor, image format, native entry, ISA-tag registry or x86
  application loading is implemented before the gate opens.

The registry separates first documentary evidence from the release that
introduced an interface; later manuals cannot silently populate January1984
availability. Procedures have signatures, including stack/register conventions,
because guest callbacks and patches cannot use guessed C types. Dispatch entries
will identify built-ins or guest code explicitly while preserving documented
Get/Set address behavior. Keep extension room unspecified, with no native kind
instantiation or marker. A bounded call-into-guest service preserves A5/A7 and
has per-call budgets, checked arguments/return sentinel and a bounded depth.

Future D3/D5-D7 and native image/ABI/marker/container decisions remain proposed.
The owner's transparency requirements apply when variants become authorized;
do not introduce a separate user mode or reopen the licence question implicitly.
An initial ordinary third-party application still requires the named legally
supplied local-test protocol; no Microsoft or Apple code is a design input.
See the [runtime plan](../development/m68k-runtime-plan.md).
