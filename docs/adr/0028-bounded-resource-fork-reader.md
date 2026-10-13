# ADR-0028 - Bounded read-only resource-fork profile

Accepted engineering contract, 2026-10-13; implementation not started.
SRC-0106; [SPEC-0024](../specifications/SPEC-0024-resource-fork-reader.md).

Application loading needs a format reader independent of MFS, heap management
and the contents of any resource. Claude supplied researched format/test drafts;
the lead checked both published editions and preserves their disagreements.

Choose a borrowing, constant-state reader over immutable bytes. Validate all
structural spans before exposing metadata; separate tolerant bounded reading
from a later strict audit. Select explicit unsigned/checked uint32 offsets and
most-significant-byte-first decoding as an engineering profile. Reject ambiguous
requested keys and expose bit-7 references without mistaking them for local data.
Return raw metadata without recreating Apple's in-memory Resource Manager.

Strict rejection of every layout deviation could exclude safely bounded files;
a naive bit-7 decoder could expose another resource's payload. Accepting a
candidate format does not prove January 1984 compatibility. Memory loading,
system-reference resolution and optional audits need separate contracts.

No new implementation/test-execution/boot claim. Guest-trap integration remains
the current runtime dependency. Revisit with eligible source or approved owner
observation, keeping existing native-app, VM, physical and edition gates.
