<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0124 - Opt-in banked trap-address query service

2026-11-18; IMPL-0154 / TEST-0215. Original engineering integration using the
already admitted SPEC-0020 trap-address interface, SPEC-0025 collapsed service
completion, SPEC-0120 guarded bank execution and SPEC-0123 atomic transfers.
No new historical claim, protected executable or third-party source is used.

## Entry and scope

`cs_s1_banked_step` in `personality/system1/banked-services.[ch]` takes a trusted
transfer backend, current native trap directory, frame/report, stack limits
and guard. Its memory boundary is backend-neutral and reusable for later
prepared native services. This increment admits only GetTrapAddress words
A146 and A046. Existing bus/transfer entry points still refuse all A-lines.
The opt-in wrapper refuses every other A-line, including SetTrapAddress.
No manager, allocation, Toolbox, guest-patch execution or native callback SDK
is exposed. The native directory is not a guest-visible historical table.

The wrapper first performs a fully guarded opcode peek. Non-A-line words
delegate to the existing transfer engine. Consequently an ordinary instruction
has an additional OPCODE guard/read before the engine's normal fetch. Backing
is stable ordinary RAM, and the immutable guard is synchronous, pure and
non-reentrant; repeated reads have no device effect. Guard callbacks must not
depend on invocation counts or retain event/frame pointers. Only the service
route emits its own INSTRUCTION, WRITE and COMMIT events. No callback comes
from an address supplied by guest code.
The routing OPCODE event has word zero and both before/proposed pointers equal
to the unchanged input frame. Subsequent service/engine events have a separate
staged proposed frame. This permits pure policies to distinguish routing from
execution fetches without mutable counters or retained pointers.

## Native and logical admission

Before publishing the report, verify representable disjoint native extents for
transfer descriptor, native directory, frame, report, limits, guard, complete
backend context and optional complete guard context. Typed objects use admitted
ABI alignment checks. The backend's own admission must exclude every source,
code/control/output object and stack from guest backing; the concrete native286
provider excludes complete CS/DS/SS envelopes. All objects already have truthful
live extents and valid alignment. All callbacks, directory and bank lifetime
remain stable under exclusive ownership throughout the call. No native pointer
validity or arbitrary hidden alias discovery is claimed.

Use strict24, even PC at most FFFFFC, S=1/T=0/I=7, a positive even stack interval
inside one readable/writable logical region and even A7 within its bounds.
Backend/lifetime admission occurs before opcode fetch. Service scratch is the
six-byte range [A7-6,A7), must lie in that stack interval, begin at or above
0x400 and have WRITE permission. The source scratch bytes are native staging,
not a guest view. No memory manager/heap is active in this service profile;
future manager binding must separately exclude its zones and operation outputs.

## Read-only service preparation and atomic completion

Only A146/A046 reach preparation. Decode the current word, resolve native
directory entry 0x46, and require its current target to equal get_token.
An opcode whitelist alone is insufficient: the directory could remap that
slot to set_token or guest code. Remapped/unsupported targets refuse without
calling a different intrinsic. The caller-requested number is D0's low word;
`cs_trap_get` validates 0..511 and resolves that current entry.

Compute results in a proposed frame. A146 returns the logical token in A0;
A046 preserves A0. Other registers and A7 remain unchanged. Apply SPEC-0020's
successful mediated CCR rule: N/Z test low D0, V/C clear, X and other SR bits
preserved. Advance proposed PC by two. No directory mutation occurs, including
failure cases. Numeric tokens are never native pointers or executed stubs.

Preflight scratch, validate continuation and proposed frame, and ask COMMIT
permission before publication. Stage completed SR followed by completed PC in
six big-endian bytes. Invoke exactly one `write_bytes` callback for all six
bytes. That callback validates the whole span before its first store and either
commits all bytes or none. Only successful completion publishes the CPU frame.
Two separately fallible scalar writes are not an implementation of this rule.

Unsafe native-envelope refusal returns ARGUMENT without changing the report.
Other failures may update it but preserve frame, native directory and RAM.
Backend failures use ADDRESS plus guest-memory status. Guard/refused-A-line or
remapped-signature failures use POLICY; query failures use SERVICE and the
underlying trap status. Service-called is set only after the named intrinsic
and directory signature are admitted. A successful query is reported as called
even if a later guard or atomic scratch write refuses; no visible effect has
yet committed. This is collapsed completion, not a general exception engine.

## Qualification and remaining contracts

Original hosted/native fixtures must exercise both accepted words and CCR/A0
behavior, every directory index including invalid low-word values, remapped
slot46 to Set/unsupported/guest identities, cross-bank six-byte scratch,
late-bank failure, continuation/stack/permission/mode/lifetime refusal and
guard denial before publication. Deliberately broken early-frame, split-write
and omitted-signature controls must be distinguished from baseline.
Ordinary instructions through the wrapper and all old A-line refusals remain
covered. No selected disk is executed or inspected.
The harness hashes sources/specs/helpers and pinned tools before compilation
and checks unchanged identities after qualification. Full native payload
comparison is split over fresh identical-state replays when it exceeds the
CLI's 16 KiB inspection budget; every replay also performs the original
loader's complete payload checksum before execution. This is not one
simultaneous full-memory snapshot or a cryptographic authenticity check.

SetTrapAddress needs a prepared native-directory delta and infallible
publication after final RAM commit. Memory traps need prepared arbitrary
copy/fill/master-cell/native-metadata effects and overlap/compaction/CLEAR
rules, rather than wiring a flat manager into this interface. Actual System
startup additionally lacks reviewed boot handoff, initial memory/ROM state,
streamed executable loading and general runtime ownership contracts. This
slice establishes one real native service path only; it does not identify
which service the selected System needs first or establish physical behavior.
