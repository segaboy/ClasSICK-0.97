# CODE-only preflight contract review - 2026-10-14

SRC-0163/0165/0166 / SPEC-0032 / ADR-0036. Engineering contract only,
unimplemented. Human provenance review pending; no TEST/IMPL/build or fixture.

Resource residency is completed first: compiled b94e2ff, public06e7751 and
HQ6e6043c, with1803 initial local/90 final affected checks and2229 publication
CI checks. Its exact source/artifact/evidence remains. Owner then directs the
next task and two boot compatibility directions; the latter are documented
separately at84c19f4 / HQb288bd1, SRC-0164. Neither is a new trial or proof.

## Inputs and original choices

Use the existing reviewed SRC-0130 LK review, source reader, heap/session/
Toolbox/resource contracts and original session push encoding. No raw draft,
new manual/source/sample, executable/resource/asset or emulator source is read.
The complete copyable Grok prompt precedes one frozen-packet Read-only review;
no HQ/manual/raw original/credential transfer. Its actual runtime reports4.6-build,
not the earlier review's4.7-build; identities/usage in SRC-0166 reflect actual
metadata. No second review of the lead's amendments is claimed.

| Choice | Adopted engineering boundary |
| --- | --- |
| CP-1 | Data-only snapshot/layout, no launch authority or executable entry; CODE-only selection and typed source refusals. |
| CP-2 |64 entries/16 segment IDs, within explicit source/profile/scratch bounds; not historical limits, RAM fit or cache reservation. |
| CP-3 | Four unsigned BE32 CODE0 fields, nonempty eight-byte table, even sizes/offset, signed-word-safe offset32..7FFE; preserve admitted slack/trailing/noncanonical warnings. |
| CP-4 | Original verified0x3F3C push shape and reviewed0xA9F0 word, positive segment IDs, first CODE1; inspect only, never execute. |
| CP-5 | Per-segment first/count bounds and complete contiguous groups; table-relative header offset and literal routine offsets are explicit interpretations. AL-2/LQ stay open. |
| CP-6 | Total segment resource <=32768 including header is a conservative subset, not proof of historical size counting; raw larger resource support remains distinct. |
| CP-7 | Explicit B/S/H_min, checked A5/global/stack/table/limit arithmetic; no implicit BufPtr/default8K/minimum budget and no full128K claim. |
| CP-8 | Honor current full-zone stack restriction; reject unsupported layout before pretending reset can populate it. Native backing-window aliases also refuse. |
| CP-9 |128-byte bands from original manager32*4, H_min0 allowed, no zero-byte views, separate format versus source/layout applicability and no allocation reservation. |
| CP-10 | Keep resource CLOSED-only execution/reset and deferred dynamic continuation/executable ownership; preflight proves neither boot direction. |

## Bounded review dispositions

| Grok finding | Lead disposition |
| --- | --- |
| G1, layout failure subclasses unordered | Adopt with changes. Integrity/state precedes parameters/checked derivations and strict24 scalar checks; then full-zone stack refusal, other inequalities/aliases and actual map windows. Do not move integrity after arithmetic. |
| G2, reject H_min0 or empty heap | Reject that ban. Explicit Z+128<=limit makes the gross heap nonempty. H_min0 is a valid chosen requirement, not proof of payload capacity; a zero remainder gets no window. |
| G3, implicit cell-band formula | Adopt explicit32*4 and checked Z+128/Z+H_min/system-band bounds. This is our allocator contract, not Apple's heap. |
| G4, owned metadata can sound like a lease | Adopt caller-held snapshot wording and source_revalidated_for_layout=false. Structural inspection has no runtime epoch lease; layout is separately tied to map/epoch/context, and later loader must revalidate. Do not adopt CLOSED-only 'load' wording: the gate is for execution/reset. |
| G5, local-storage/warning ambiguity | Adopt warning clarification. Correct its proposed definition: native source/backing disjointness belongs to the earlier envelope; local-storage eligibility means reader local payload plus explicit application attribute policy. No bit7 offset is followed. |

No proposed cases execute. Future original verification obligations are enumerated
in SPEC-0032, including full boundary/alias/count/layout cases and success with
execution still unauthorized. No registry import, CODE fixture, heap reset,
loading, guest binding, app/boot/VM/reference run or historical question closure.

Next propose implementing this bounded read-only checker, then separately address
the layout/executable-owner/continuation/population gaps before actual launch.
The user's boot requirements and independent Apple-ROM-free distribution remain
required; small profiles do not narrow those release goals. Preserve all24
originals/copies, existing build/edition/interface/boot/assets/runtime pointers,
native owner gate, source/UART boundaries and human provenance review. Git wiki
queue only; no extra agent/chat/automation/OneNote.
