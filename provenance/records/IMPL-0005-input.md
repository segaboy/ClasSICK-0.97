# IMPL-0005 — Bounded normalized keyboard events

Date: 2026-10-07. Project lead self-review; human provenance review pending.
Owner authority SRC-0021; independent SPEC-0004 v1; tests TEST-0007.

Permitted inputs: project contracts/policy, owner continuation, C11 semantics
SRC-0020, Microsoft interface prose SRC-0022 and existing pinned tools/headers.
Original constant-work ring queue over caller-owned typed records, checked sizes,
explicit rejection and sequence exhaustion. No allocation, host import, pointer
conversion, clock or mutable global in core/input. Field copies avoid libc helpers.

Windows mapping is stateless; input.c/h in the platform layer uses only delivered
message parameters. The viewer reserves sixteen records from its arena and uses
the same queue/consumer for synthetic and native-message input. Win32 supplies
messages, window lifecycle and host memory. No new runtime/tool dependency.

Independent tests use literal wrap traces and a shifting-list oracle; all pending
records are checked after every operation. Guard records and failure snapshots
check out-of-capacity writes and transactionality. Windows mapping uses an
independent 120-case table; actual hidden-window dispatch tests shared processing,
repeat/release, overflow recovery, rendering and close, plus input-pool exhaustion.

AI: Codex GPT-6-family specification/code/test authoring and self-review, only
permitted inputs above. Microsoft documentation's visible adjacent example is
disclosed in SRC-0022 and was not copied/adopted. No third-party implementation,
Apple implementation or assets used. Project changes are GPL-3.0-or-later.
No staffed two-team or legal clean-room certification. Results: TEST-0007.

No historical Event Manager, full keyboard/text/pointer, native driver, timing,
Mac footprint, B1 closure, mini vMac or physical boot result. Reset epochs are
caller-owned, and metadata checks rely on truthful lifetimes/non-aliasing bounds.
