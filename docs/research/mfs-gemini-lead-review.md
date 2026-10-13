# Gemini MFS research intake and lead review - 2026-10-12

SRC-0105; research review only, not an MFS specification or implementation.
The owner supplied three Markdown documents in an archive after commissioning
Gemini's read-only MFS research. Their proposed actions, C declarations and
"formal specification" labels are research content, not adopted instructions.
Original files are retained unchanged outside Git. This review publishes the
lead's findings, not the supplied algorithms, quotations or unsupported claims.

## Source and intake boundaries

Archive SHA-256:
`cb18273156ac8d6f1e6f6620aae858135d8dd09bb5c3eaf99b02b0fcd683c043`.

| Supplied file | Bytes | SHA-256 |
| --- | --- | --- |
| mfs-readonly-draft.md | 23012 | 8d2ad228f01d1ac7faba3ee47fa5368fa553d7f46b1960d804182f9518571d7d |
| mfs-readonly-test-plan.md | 12499 | 8e8ce2f2b2765baa762e0c64f4e6ed9edec213482dd52739c2ab713d81c67ec2 |
| research-response-summary.md | 5436 | 5a1386363cb77f017027fa518facd9355da9369f1a7fd64a526185c4f7febf73 |

Gemini attests to published-manual-only research and no prohibited implementation
inputs. That statement is not an independently established exposure history.
No prohibited input is identified in the supplied text. Its source claims and
proposed algorithms are not implementation authority.

The lead independently checked SRC-0071, the owner's published 1985 Volumes
I-III: original SHA-256
`b65fe8efe782799a333a0b59b225e2bbaf9e2e150dc949dd555571971b331d97`,
1,284 pages. Printed II-81/82/84/119-123 were read; II-81/120-123 were visually
checked, including Figures 6-8. For this identified copy, PDF page = printed
Volume II page + 568. Claimed III-108-112 citation openings and the following
II-124 page were also inspected to delimit format material from in-memory
summaries. Ancillary public API/type/equate declarations were encountered;
no sample, equate text, in-memory queue/FCB/VCB layout or internal algorithm is
adopted. Reference pages remain outside Git.

This review does not independently verify the Promotional Edition citations,
prove absence of a packing explanation throughout either edition, or establish
January 1984 behavior. The published Promotional Edition is distinct from the
excluded confidential/prepublication drafts; Gemini's loose description of it
as prepublication text must not change SRC-0067/0068/0070 eligibility decisions.

## Corrections required before finalization

| Finding | Evidence and required correction |
| --- | --- |
| R1: swapped volume lock bits | Draft section 4.1 reverses the two meanings. Published II-121 gives bit 7 hardware lock and bit 15 software lock. Correct the draft and give separate single-bit tests. |
| R2: invalid base fixture | II-120/Figure 6 reserves logical blocks 2 and 3 for the master directory. FIX-V01 starts its directory at block 3, overlapping that region. Its dense-packing proposal gives map span [1088,1676) and directory [1536,3584), overlapping by 140 bytes; the two-block master-region overlap is independently 512 bytes. All fixtures inheriting this base need new geometry. The allocation-region end itself, 407040, fits its 409600-byte buffer; that does not repair the overlap. |
| R3: rejecting a documented directory placement | II-122/123 permits directory blocks within the allocation area, with their allocation entries marked FFF. FIX-M06 cannot classify every such overlap as malformed. Distinguish reserved directory ownership from file payload ownership. FFF is not an alternate file-chain EOF marker. |
| R4: incomplete directory boundary checks | II-122 requires each directory entry to stay within one 512-byte logical block. Checking only the entire directory extent is insufficient. Define padding and inactive-entry traversal; do not terminate the whole directory merely because one sector's remainder is zero. Tests need later active records following holes or an empty sector. |
| R5: an allocated empty fork is discarded | II-81 explicitly describes zero logical bytes with 1024 physical bytes. Draft section 7.1's rule that every zero-length fork starts at 0 or 1 is unsupported. II-123 gives start 0 for an absent fork, which is a separate condition. Distinguish absence, zero logical bytes with allocation, and inconsistent metadata; zero returned bytes do not justify skipping chain validation when promised. |
| R6: filename restrictions are overstated | II-81 describes 1-255 printing characters excluding colon, case-insensitive comparison retaining diacritic distinctions, and version numbers. It separately recommends fewer than 64 characters for Finder use. FIX-V07's 31-character maximum is not this published File Manager limit. ASCII-only comparison, encoding, version selection and disk-entry rejection rules need explicit scope rather than a January 1984 claim. |
| R7: unsupported field/padding assertions | Selected pages do not establish the draft's copy-protection bit 6, mandatory zero padding, ASCII-only names, or a four-block historical directory default. Figure 8 confirms bits 7 and 0; II-122 gives a 12-block 400K example. Preserve other flag bits and padding as uninterpreted until a reviewed contract says otherwise. |
| R8: chain indexing and termination are ambiguous | II-122 makes the first stored map entry correspond to allocation block 2. A dense decoder must explicitly map block number to entry index; draft VBM[block] notation is insufficient. Its pre-visit comparison uses count > expected rather than excluding an extra block when count equals expected. A one-block physical fork followed by a second terminal block can pass that proposed procedure. Add a two-block counterexample, not just FIX-M09's three-block chain. Exact physical-chain consistency remains a contract decision. |
| R9: the byte-packing gate remains open | Neither word endianness nor the author's confidence selects a 12-bit packing order. Selected II-122 prose establishes entry meanings, not a nibble diagram. No proposed decoder or byte-encoded fixture is accepted as MFS-compatible here. Synthetic self-consistency would test an engineering encoding only. |
| R10: the proposed API lacks a bounded failure contract | Specify caller-owned scratch/output capacities, input lifetime, alias rules, descriptor provenance and behavior of output bytes/reports after failure. The proposed traversal writes output before discovering later corruption. Choose preflight/transactional behavior or explicitly documented partial output; do not leave it implicit. Avoid a volume-sized automatic array without a checked bound and a 128K budget. |
| R11: validation promises and coverage differ | Cross-link detection is optional in the draft while an error code is declared, but there is no cross-link fixture. Define whether validation covers the selected fork or every live file/fork. Add same-file data/resource cross-links, cross-file sharing and reserved-directory references under that stated scope. FIX-M12's value 9999 cannot be represented in 12 bits; use an encodable out-of-range value. Several advertised cases are only matrix rows, not specified fixtures; none has executed. |

The raw header and directory offsets broadly agree with Figures 7/8. That
agreement does not establish the draft's extra validation rules or make its
invalid fixtures usable. Printed-page references must identify each actual
format table rather than rely on a broad summary-page range.

## What can be carried forward

The research provides useful coverage categories: truncation, checked address
arithmetic, noncontiguous forks, cycles, free/out-of-range links, premature EOF,
logical-versus-physical lengths and directory boundaries. Keep those ideas as
proposals and rebuild fixtures from a reviewed contract with independent
interval/content oracles. No parser, API spelling, status enum, placement
procedure or candidate bit decoder is adopted by this review.

The next MFS contract should define a flat sequence of 512-byte logical data
blocks separately from physical/tagged/floppy formats, bounded caller-owned
state and explicit assurance scope. It can finalize header/directory facts and
engineering safety choices before a justified packing decoder is available;
complete fork extraction cannot claim interoperability until that gap closes.
No stock image, Apple boot block, executable or manual example becomes a fixture.

2026-10-13: the requested [filename follow-up review](mfs-filenames-lead-review.md)
(SRC-0109) corrects case-flag polarity, retains version-sensitive identity and
the published accent distinction, and limits interim ASCII comparison to an
engineering mode. Its bounded-reader directions do not finalize an MFS API or
close the packing/historical gates. Original intake findings above remain.

Suggested follow-up research: filename encoding/comparison, including
case, diacritics, lengths, version numbers and duplicate names; separate creation
rules from validation of existing disk entries. Claude's resource-fork draft and
the proposed Copilot Memory Manager review remain separate tasks.

SPEC-0023, its compiled source and previous test/native evidence are unchanged.
No MFS specification/build/test ID or implementation is created. No emulator/VM,
x86 application, UART-body inspection, delegation or OneNote operation occurred.
Native-app confirmation, future VM go-ahead, historical/physical/edition/parity
and budget gates remain unchanged. Lead self-review; human provenance review
pending. The [68000 runtime plan](../development/m68k-runtime-plan.md) remains
the dependency order; this intake does not displace guest-trap integration.
