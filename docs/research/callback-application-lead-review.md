# Claude runtime and baseline research: lead review - 2026-10-13

SRC-0113 (callbacks), SRC-0114 (first application), SRC-0115 (baseline audit).
The owner requested intake of all three packages. This is a source review and
planning decision record, not a new implementation authorization, callback
specification, registry expansion or historical compatibility result.

## Intake and review scope

Both repositories were checked and synchronized with fast-forward-only pulls.
The reported checkout was stale: commits 799df5d and 5aa75c2 were already
preserved in ecc561e. No research commit was replaced. Original callback and
resource-fork drafts remain unchanged and untracked; unchanged intake copies,
manual extracts and page renders are kept outside Git. Only this independent
review and its planning/provenance records are added.

| Input | Bytes | SHA-256 |
| --- | ---: | --- |
| callback-catalog-draft.md | 29328 | `156bcf56f0c962045ffac30fbd5621ecae81a5a417a9970519102fd434764aee` |
| callback-catalog-draft.json | 29606 | `952715b94ae45822878ad7297d022b28b130e3de27762b51541cc3f537bc75fd` |
| first-application-interfaces.md | 24054 | `a6aa1d309ed6c00b1fdada7a6a668e5786a8d782afe613552d69b87d3fa888fe` |
| first-application-interfaces.json | 25817 | `b74cf5f544c2a6d74aed4d6f68a94b5b714219667b05f186235d0959da729285` |
| jan1984-baseline-gemini-review.md | 17148 | `d005acf87998bff32847a4fed4e53c72930c184f40933ec2d21646007dfdde17` |
| Gemini original baseline report | 34883 | `97974f8d8ec2763621072c108fa60a4043608203d4a56f8b4b885d98eb72d234` |

All three Markdown records and both JSON structures were reviewed. Metadata
checks confirm 41 distinct application routines (tiers 23/18), thirty positive
memory-movement flags, matching numeric trap fields, and 41 unique grouped
callback entries. These checks do not verify every source locator, calling
convention, omitted routine or claim of catalog completeness.

Independent primary-source checks used the already eligible SRC-0071 and
SRC-0068 copies. Their full hashes and page counts were rechecked against the
existing records. SRC-0071 printed I-91/93/95/97, I-162/163/209, I-282/288/324,
I-379/380, II-20/99/349, I-58/453/454, II-358/372 were read for selected
interface/numeric/prose facts. Visual checks: I-91 parameter table, I-163 global
placement, I-324 action signatures, I-58 Edit menu and II-358 alert/return rule.
SRC-0068 AL-10/14/18 (PDF108/112/116), QD-34 (PDF288) and WM-20 (PDF428)
were checked for corresponding prose; AL-18's OCR header is unreliable, so its
page assignment follows the existing verified mapping rather than that OCR.
No new independent confirmation of the catalog's remaining edition comparisons
or all forty-one trap words is claimed. Review each adopted executable
signature and trap record separately before implementation.

Only published interface facts are carried forward. Manual instruction
examples, ancillary equates, private implementation layouts and pictured
artwork are not replacement-code or asset inputs. Research exposure
attestations are attributed claims, not independently proven provenance.
Neither published edition establishes the January 1984 trap/callback set.

## Callback catalog: corrections and K-decisions

The call-site model is supported by I-324: indicator actions have no parameters;
other control actions receive a control handle and part code. The manual also
warns that the application must supply a procedure appropriate to that case.
A selector chooses a calling shape; it cannot make one arbitrary procedure
compatible with both. DeskHook's two uses are documented with D0=0 for painting
(I-282) and D0=-1 plus A0 pointing to an event for clicks (I-288).

The draft's statement that four rules govern *every* callback is too broad.
I-93/95/97 give general stack/Pascal/A5 guidance, but I-379/380 explicitly
document register/condition-code TextEdit entries, and II-99/349 give different
interrupt preservation sets and require the callback to establish A5 when
using application globals. The specific call-site contract takes precedence.
An OS patch retains its documented register interface; a generic Pascal
parameter cleanup rule cannot override it. A5 guarantees outside ordinary
application Toolbox calls remain CQ-7, not an inferred universal promise.

| Decision | Lead choice for future contracts |
| --- | --- |
| K-1 | Adopt call-site signatures with explicit selectors, entry/result/condition-code fields, preservation, A5/context, cleanup and return kind. A pointer alone supplies no signature. |
| K-2 | Accept the proposed default-identity order when their managers are supported: DABeeper, standard filter, word-break and click-loop defaults; drivers and interrupt vectors later. This allocates no addresses or registry records now. A built-in identity is an owned guest token, never a native pointer derived from guest data. |
| K-3 | Modify the recommendation: register-preservation diagnostics may be optional, but checked control targets, stack bounds/alignment, owned return sentinel, signature-specific completion and execution/depth limits are mandatory in the bounded executor. A malformed return must become an explicit service failure, not permission to resume an unsafe host frame. Define restoration and guest side-effect policy in the next contract; no blanket rollback is promised. |
| K-4 | Retain both edition readings. Adopt a differing reading only under an explicit engineering profile or later eligible historical confirmation; no January 1984 availability inference. |
| K-5 | Defer MBarHook and TextEdit highlight/caret execution until parameter cleanup is evidenced. No guessing from sample instructions. |
| K-6 | Defer interrupt callbacks until interrupt points, context, preservation and memory restrictions have a reviewed contract. |
| K-7 | JSON remains review input. Only independently checked signatures become stable registry records. |

CQ-1 through CQ-8 remain open. The next bounded service should start with a
fully specified ordinary returning procedure and original probes. Nested traps
and callbacks need owned per-call state and bounded depth. Nonreturning resume
entries, interrupt callbacks and unsupported resource classes are not silently
accepted as RTS callbacks. No callback execution exists as a result of this review.

## First application: FA-decisions

The [41-routine research set](first-application-interfaces.md) is accepted as
a candidate application workload, not forty-one implemented or approved
dispatch records. The documented initialization chain remains a prerequisite
checklist. The first program's exact manager subset must be stated: tier 1's
23 routines do not by themselves demonstrate the full dialog/desk-accessory
initialization and behavior described in the later manual.

| Decision | Lead choice for future contracts |
| --- | --- |
| FA-1 | Adopt tier 1 first: an original window/drawing/mouse/quit workload. Tier 2 menus/accessories follow after their dependencies. These are planning targets, not new implementation permission. |
| FA-2 | Use original private QuickDraw state; do not reconstruct undocumented Apple bytes. The documented 206-byte guest reservation and public globals still need a checked interface layout. The 130/76-byte split is size arithmetic, not a recovered private layout. Private-byte interpretation is unsupported, not a prohibition on ordinary bounded guest RAM access. |
| FA-3 | Respect CODE0's declared A5 sizes; do not reject solely because below-A5 is less than grafSize or silently add a second globals reservation. I-163 and II-20 allow development-system placement to vary. The eventual InitGraf contract must validate its actual complete globals span and the writable pointer slot at (A5), with checked arithmetic and overlap rules, before writing. Small declared size does not waive span checks. AL-5/FQ-3 remain open for January 1984. |
| FA-4 | Plan a 512 by 342 monochrome engineering profile. Larger logical guest screens require an explicit extension; scaling the host presentation alone does not enlarge guest screenBits. Do not use the proposed system1-1984 name as proof of historical qualification. |
| FA-5 | Tier 2 uses wholly original DRVR fixtures or an explicitly empty accessory menu. It never runs Apple's accessory code. No DRVR implementation is started here. |

Two metadata cautions remain. A routine's absence from Appendix B means
"not listed", not proof that arbitrary code it invokes cannot move memory;
SystemTask in particular runs accessory code. Transitive callback effects must
be considered separately from the thirty positive list entries. Also, stack
BOOLEAN representation from I-91 does not establish byte placement inside
every resource format: the WIND uncertainty FQ-4 stays open. FQ-1 through
FQ-5 remain recorded; no first-release introduction date is assigned.

## January 1984 baseline audit

The [audit](jan1984-baseline-gemini-review.md) is accepted as a research
checklist with evidence labels, not a behavioral oracle. R-001 stays open.
This intake independently confirms the original report's supplied identity;
it does not prove its unverified source citations or all J1-J26 findings.

Selected later-manual checks support the audit's cautions: I-58's application
Edit-menu illustration has no Select All shortcut and its prose preserves the
old Clipboard for Undo; I-453/454 describe memory retention through launch;
II-372 describes threshold-based doubling rather than exponential mouse
scaling; II-358 illustrates Restart/Resume and describes a jump after restoring
A5 and discarding the stack. These are later edition facts, not confirmation of
the January Finder. Application-menu illustrations cannot settle Finder ordering.

The hardcoded-Str63 implementation inference is not adopted. Neither raw disk
imaging, transcription nor a reference-system run is initiated. Unread owner's
guide/Byte sources, disputed version/date stories and further UX details remain
research leads. Future primary-document review or an approved owner observation
must supply exact evidence before a Finder/desktop/Control Panel specification.

## Carry-forward and boundaries

Update runtime plan steps 1/3/4/5/6 with these decisions. The proposed next
implementation slice is original 68000 JSR/JMP plus a bounded returning
call-into-guest contract, followed by guest patch dispatch. Finalize the exact
instruction/signature/state/error contract and original tests first. Current
QuickDraw and UX research may inform later manager and appearance contracts;
it does not block this basic execution contract or authorize importing drafts.

No code, interface-registry records, build images, fixtures or runtime tests
changed or ran. Existing SPEC-0025 execution/CI evidence remains at its recorded
revisions. The native-application gate, fresh VM/emulator go-ahead, published-only
source rule, UART role separation and no delegation/chats/automation/OneNote
constraints persist. Historical/full-OS/physical/128K/parity/three-edition and
B2 criterion 6 limits remain. AI-assisted lead review by Codex; Claude's work
is attributed research. Human provenance review remains pending.
