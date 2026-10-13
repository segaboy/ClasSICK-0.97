# ADR-0029 - Bind built-in memory calls before guest callbacks

Accepted 2026-10-13, SRC-0110/0111 / SPEC-0025.

The register Memory Manager works independently of CPU execution. Connect it
through a personality-owned, signature-checked logical identity table and a
trusted internal CPU adapter, preserving the old strict interpreter entry.
Guest logical identities never become native callable pointers. Keep arbitrary
guest execution and application callback signatures for Claude's separately
reviewed catalog and a later contract.

Opt-in data-address normalization permits dereferencing documented flagged
master pointers without discarding flags from registers. Control flow and
implicit stack accesses stay strict. Protect exception scratch from heap and
BlockMove writes before executing the service. This permits transactional step
failure without copying the whole heap. Original tests, compiled object audits
and retained evidence validate this engineering slice; no boot/VM or historical
application compatibility follows from it.
