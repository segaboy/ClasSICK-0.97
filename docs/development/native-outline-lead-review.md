# Native outline contract lead review -2026-10-18

SRC-0280/0282/0284; SPEC-0048 / ADR-0054. Owner "Now, on to the next task"
continues the previously proposed paired drawing-primitive contract after the
SPEC-0047 boot observations. Claude supplied114 spec lines and44 prospective
matrix rows in68s from reviewed public headers/contracts, no tools or writes/tests.
Codex alone reviews, edits and allocates IDs. No new historical source fact.

| Proposal | Disposition |
| --- | --- |
| OL1-OL7 | Adopt: separate helper, explicit0..32767 pen dimensions, existing view validity, complete preflight, signed BE Rect/translation, empty/inverted/zero-size success |
| OL8 | Adopt with correction: intermediate subtract minimum is -98303, not -65536; upper bound98301; both safely int32 |
| OL9-OL12 | Adopt: coverage before clip, no invented clip edge, black OR/set, exactly one saturating paint, fixed effect order, no external calls/scratch/global/allocation |
| OL13 | Adopt with changes: complete port readable/writable; view/port/rect pairwise disjoint; no invalid native-pointer safety promise |
| OL14-OL15 | Adopt as deferred: selected-view demonstration and legacy ABI need later scopes; current modes/build inputs unchanged |
| P1-P5 | Adopt with changes: lead finalizes this contract; no invented extra owner approval for documentation. Runtime implementation remains a later authorized slice; named profile/layout audits replace unspecified all-hashes claim |

The matrix has44 rows:38 prospective native cases/audits,3 future compiler/
regression audits,3 deferred/non-coverage rows. None executed; no TEST or IMPL ID
is allocated for outlines. Remove the poisoned-pointer suggestion because it
violates the truthful native-storage premises; null/corrupt-state checks suffice.
Include offset -32768 as well as32767 to exercise the stated arithmetic minimum.

H1-H5 remain open: historical pen placement/zero/negative/oversized behavior,
pattern/mode/hidden-pen effects, region/port clipping, early trap ABI and recording
side effects. This original outline profile does not answer them or identify
historical FrameRect. No manual or untracked draft transferred to any worker.

Next implementation prerequisites: finalize the header with explicit current pixel
polarity/bit mapping from reviewed project code; preserve existing structs/APIs;
give Claude one bounded C artifact and Copilot the frozen-API pixel oracle; audit
32/64/endian/native68000 closure and unchanged candidate inputs. Selected demo,
guest trap, target trial and release remain separate. Physical/full128K/B2/
edition/native-external-app/both System1 gates persist. No OneNote/wiki.
