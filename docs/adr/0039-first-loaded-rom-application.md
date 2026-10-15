# ADR-0039 - Integrate one loaded application into a boot ROM

2026-10-15; SPEC-0035 / SRC-0187..0190. Accepted engineering decision.

The owner accepted a visible integration milestone after reviewing the imbalance
between isolated infrastructure and demonstrated behavior. Group the smallest
execution/ownership/retirement dependencies into one original CODE application
whose observed value/pattern are rendered by a new boot ROM profile.

Use a dedicated single-root session with one entry/segment, the image's explicit
embedded exclusive participant, actual guarded READ-backed fetch and balanced
RTS interception. Keep existing legacy session/loader/external gates intact;
do not admit Toolbox services or broaden native application work. The larger
SPEC34 general profile remains incomplete. Its native-protocol safety principles
continue to apply; no parallel decoder or guessed Apple internals are introduced.

A separate reset/entry/linker/build profile reserves its own static workspace
and conservatively bounded native stack within128KiB; preserve the prior ROM
profile. Show returned guest results through the existing original renderer,
clearly distinguish kernel mouse input from application event support, and
separate hosted preview/build evidence from any owner-authorized actual boot.

Batch worker assignments, checks and public/HQ/wiki evidence at this milestone.
No general desktop, historical compatibility, reference trial or native SDK
follows. All existing source, physical, edition and human-review gates persist.
