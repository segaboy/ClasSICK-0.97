# Gemini MFS filename review - 2026-10-13

SRC-0109; source review and carry-forward design choices, not a finalized MFS
API, implementation or historical compatibility result. This follows the
[first MFS intake](mfs-gemini-lead-review.md). The owner's supplied dossier is
research: its instructions, C declarations and claims of readiness do not
authorize or establish implementation behavior.

## Intake and independently checked sources

Supplied `classick_mfs_filenames_research.md`: 16,463 bytes; SHA-256
`151aef6c8790e82a52a80e97b48ebb8989676d3fa725d0219f1043c37d315628`.
An unchanged copy and private reference extracts are retained outside Git.
Gemini attests to published-document-only inputs. The lead cannot independently
establish that exposure history; no prohibited input is identified in the text.
The dossier's bibliography does not identify the actual PDF copies or verified
page mappings. Only the lead's independently checked facts support this review.

The owner's eligible SRC-0071 copy was rechecked: 19,245,372 bytes, 1,284 pages,
SHA-256 `b65fe8efe782799a333a0b59b225e2bbaf9e2e150dc949dd555571971b331d97`.
Read I-246/247; II-79-85, 371/377/378; III-110/185/239/241/258-260.
III-237-260 glossary headings were searched to locate the actual definitions.
I-247's character-code chart, II-377's API flag table and III-110's directory
summary were visually checked. Prior SRC-0105 checks of II-122/123 and Figure 8
remain the evidence for record offsets and logical-block boundaries. PDF page
equals printed page plus 10/568/1003 for Volumes I/II/III in this identified copy.

Scope is published API prose, numeric character codes and on-disk format facts.
Ancillary API/type/in-memory declarations on mixed pages were encountered; no
sample code, equate text, in-memory queue/VCB implementation or glyph artwork is
adopted or republished. Numeric encoding facts are separate from fonts/assets.
No new Promotional Edition verification occurred. Its published status remains
distinct from the excluded confidential drafts. These 1985 books do not establish
the January 1984 routine set, character repertoire or actual duplicate behavior.

## Corrections and retained facts

| Item | Published evidence and review outcome |
| --- | --- |
| F1: length and Finder scope | II-81 supports file names of 1-255 printing characters excluding colon. It recommends fewer than 64 for Finder use and describes an error for longer names. This does not establish an exact Finder 1.0 `Str63` boundary or behavior at 64. The report's 31-character/HFS history is outside the checked scope. |
| F2: accents are already specified | II-81 says File Manager name comparison ignores case and retains diacritic distinctions. II-79 says the same for volume names. The report's question about whether file lookup ignores accents is answered for this published edition; the exact internal call/flag sequence is neither needed nor inferred. January 1984 confirmation and the complete high-byte equivalence relation remain open. |
| F3: reversed case flag | II-377 specifies that setting trap bit 10 selects case sensitivity, not case insensitivity. Setting bit 9 suppresses diacritic distinctions. The dossier gets bit 10 backwards. These are published interface facts, not adoption or implementation of EqualString/UprString. |
| F4: version is part of identity | II-81 identifies a file by name and version, with version 0-255 distinguishing same-name files. It recommends version 0 because several higher-level managers do not handle nonzero versions and Finder ignores versions. It does not say File Manager ignores the field or that the field is unsupported. A lookup key must retain the version; a launch policy choosing 0 must say so explicitly. |
| F5: character chart and period | I-246/247 are in the Toolbox Event Manager, not a QuickDraw/Font Manager appendix. I-246 describes extended ASCII; I-247 Figure 1 supplies the chart and identifies control codes 00-1F and 7F. The chart supports the report's selected 1985 high-byte examples, but not its January 1984 dating or a complete case-fold table. Its proposed 87-to-83 example changes the base letter and is not a valid case pair. Blank chart cells do not establish historical introduction dates. |
| F6: reading and creation differ | A counted on-disk name can be represented as raw bytes without a C terminator. Preserving an embedded zero in a bounded parser is an engineering decision, not proof that the historical File Manager accepted such a created name. The report's uniform `bdNamErr` behavior for every empty/control/colon input needs routine-specific evidence. Its control-byte rule also omits 7F. |
| F7: no evidenced duplicate winner | No checked passage establishes first-active-match precedence on duplicate identities. The report's lenient mode cannot be labeled System 1.0 fidelity. Same name with versions 0 and 1 is not a duplicate File Manager identity. Future corruption reporting needs a defined comparison mode, version and search/validation scope. |
| F8: corrected locators | Volume-name definition is II-79, directory summary III-110, file-name glossary III-241, version glossary III-258, volume-name glossary III-259. II-82 concerns EOF/mark behavior, not the alleged pathname construction; II-371 concerns parameter RAM. III-185 does summarize the string APIs. Only II-377 among the cited utility pages supplies the reviewed comparison contract. |

II-123/Figure 8 and III-110 agree that the directory's version byte is distinct
from Finder file type metadata. The name length is at offset 50 and characters
begin at 51. Record extent is 51 plus character count, rounded to even length;
it must stay within its 512-byte logical block. No filename character limit can
replace that boundary check.

## Carry-forward design choices

These choices constrain a future reviewed reader contract; no C API, status
enum, decoder or implementation is finalized here.

1. **Preserve representation.** Return counted raw byte spans from a checked
   immutable input buffer. No NUL termination, Unicode normalization, trimming
   or lossy encoding conversion. A zero-length active name is retained as
   anomalous metadata for the future reader policy to diagnose; it cannot
   silently terminate the directory or become permission to create an empty
   name. Borrowed spans require the caller to retain the unchanged input buffer.
   Validate lifetime, aliasing, descriptor provenance and failed-output behavior
   in the eventual API contract, following SPEC-0024's explicit borrowing rules.
2. **Limit provisional comparison.** Exact bytes and a named ASCII-only mode
   are useful diagnostic/engineering operations. The latter folds only 41-5A
   against 61-7A, leaves higher bytes literal and preserves all other bytes and
   lengths. It is incomplete for historical accented upper/lowercase pairs and
   must not silently satisfy a request for full File Manager comparison.
   A historical mode stays unsupported until its equivalences are justified.
3. **Keep the version in the key.** Preserve the full unsigned byte and require
   an explicit requested version. Same name with different versions remains
   distinguishable. A future CODE-only launch service can request version 0
   under its own reviewed contract; a general reader must not discard versions.
4. **Report ambiguous identities.** Under the explicitly selected comparison
   mode and requested version, multiple active matches produce an ambiguity
   result with no selected winner. This is ClasSICK's engineering safety policy,
   not a historical observation. Define complete search bounds and the relevant
   identity scope before promising duplicate detection; a first-hit early return
   cannot provide that promise.
5. **Separate extraction from creation.** Bounds-safe extraction retains raw
   bytes and unknown flags/padding. Future creation/name-validation services
   separately specify accepted characters, routine errors and Finder limits.
   Byte slices alone are not historical validation of those names.

No public filename helper, File Manager trap or full comparison API is introduced
by these decisions. Full high-byte mapping/case equivalence, January 1984 evidence,
malformed-entry behavior, duplicate observations and the MFS 12-bit packing gate
remain open. Guest-trap integration remains the next implementation dependency.

## Test-plan review

The dossier has 14 creation rows, 13 comparison rows and 11 directory rows.
They are proposed cases, not generated fixtures or executed tests.

- The positive directory-size arithmetic in DISK-01..06 is correct, but each
  fixture also needs a chosen sector offset and a check against that sector's
  end. Include exact-fit, one-byte-overrun and later-sector cases.
- CREAT-10..12 cannot claim an exact Finder boundary. The parser API shown does
  not implement file creation; creation errors require a separate contract.
  Add 7F and keep high-byte repertoire validity separate from raw ingestion.
- CMP-01..13 can test the explicitly limited modes. The accent distinction in
  CMP-11 is documented for the published File Manager, while CMP-12 still needs
  a supported accented case pair for full historical comparison. Treat supplied
  high bytes as bytes in engineering fixtures, not evidence of 1984 acceptance.
- DISK-07 needs the active flag, name extent and sector placement defined. It
  cannot simply skip an unspecified zero-length record. Do not confuse an unused
  slot or sector tail with a valid active empty name or end of the whole directory.
- DISK-09's different versions are separate identities, so its duplicate warning
  is wrong. DISK-10 must give both versions explicitly and test each comparison
  mode. DISK-11 needs inactive-slot traversal and later-sector coverage.
- The proposed lookup lacks a target version and an output/failure/lifetime
  contract. Several expected warning/error names are absent from its status
  enum. Resolve those inconsistencies before allocating executable test IDs.

Original source copies and Claude drafts remain unchanged. No parser/test code,
new VM/emulator run, native application, UART-body inspection, delegation,
asset adoption or OneNote operation occurred. Existing Memory Manager/build/boot
evidence and owner/native/historical/physical/edition/parity/budget gates persist.
Lead self-review; human provenance review pending.
