# ADR-0031 - Separate typed patch execution from atomic built-in dispatch

Accepted for bounded development verification, 2026-10-13; SPEC-0027.

A guest patch can change RAM or allocate before encountering a CPU fault.
Putting it inside SPEC-0025's atomic dispatch callback would promise a rollback
that the bounded call service cannot supply. Use a separate personality-owned
executor and trusted step callback with explicit entry/failure diagnostics.
Keep the legacy path and its failure contract intact.

Built-in identities and guest code are distinct typed entries, with an explicit
register signature. Saved-original memory veneers execute only under the active
matching trap word. Preflight their complete return before invoking the manager.
This preserves allocation flags without guessing a direct-call convention from
an arbitrary pointer. General direct guest calls, other callback shapes and
Get/Set veneers remain future work. No third native application entry kind.

Retain completed guest effects on late failure and restore caller controls/frame
as specified. Whole-runtime snapshots would add unbounded cost and obscure guest
side effects; pretending all failures atomic would be incorrect. Existing gates
and historical uncertainties are unchanged.
