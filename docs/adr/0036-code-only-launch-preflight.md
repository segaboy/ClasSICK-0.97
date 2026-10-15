# ADR-0036 - Read-only CODE launch preflight first

Status: accepted engineering design, 2026-10-14. SRC-0163/0165/0166;
SPEC-0032. No implementation or historical confirmation.

The reader, bounded heap/session/Toolbox services and resource cache exist.
They do not yet establish a safe legacy application launch: dynamic continuation
and executable-resource ownership remain absent, and existing reset's full-zone
stack restriction cannot be silently weakened. Startup must validate CODE and
world/stack arithmetic before a future irreversible population transaction.

Adopt a read-only, host-supplied CODE-only inspection/layout profile with64 entries
and16 segment IDs, explicit literal resource-start offsets, conservative total32K
segments and typed no-effect refusals. CODE0's four long fields, initial entry
shape, segment headers/groups and first-CODE1 rule are checked without executing
or loading data. Candidate memory layout has explicit B/S/H_min and honors the
current lifecycle/map constraints. Output is metadata, never execution authority.
All selected interpretation limits are engineering policy; AL-2/LQ and1984
identity questions remain open. No manual sample or private heap algorithm is used.

Alternatives deferred: adding reset/LoadSeg mechanics before entry lifetime is
reviewed; making cache handles executable by assumption; silently admitting
full-zone stack overlap; native variant selection; treating a format test as
boot/application compatibility. This step introduces none of those behaviors.

The two owner-required System1/ROM boot directions remain untested acceptance
gates. Preflight helps a future loader and proves neither direction. Next
implementation can test this original data-only subset separately; actual
execution/population/continuation needs further contracts and source/owner gates.
