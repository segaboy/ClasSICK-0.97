# Memory Manager handoff review - 2026-10-12

The owner returned Claude's two research drafts while SPEC-0022 was being
implemented. This is a lead review and proposed engineering direction, not a
finalized Memory Manager specification, source verification, or compatibility
acceptance. The original draft files remain untouched and untracked. No manager
implementation is based on their unverified citations yet.

## Fit with the completed memory layer

SPEC-0022 supplies big-endian byte/word/long data access, checked writable spans,
MOVE/MOVEA, conditional branches and BSR/RTS. Maps persist through caller-owned
buffers; register/data/stack failures leave the instruction uncommitted. The
prior A-line service contract remains intact. This closes the general checked
storage prerequisite; it supplies no allocator, heap-zone policy or globals.

The draft's high-byte/master-pointer issue is a real contract dependency. The
current engineering map deliberately rejects addresses above 24 bits. A future
68000 address-bus adapter should normalize data addresses under an explicit
profile while preserving full address-register values, then call the strict map.
The storage API itself should retain its unambiguous checked-address contract.
Its tests must distinguish logical address normalization from native pointers.
Direct guest writes to a master-pointer flag byte also require the manager to
validate/reconcile that cell before moving or resizing its block.

## Proposed decisions for the first manager contract

These are engineering directions pending the source/contract review below.

| Draft decision | Proposed direction |
| --- | --- |
| DEC-1 representation | Adopt a guest-visible four-byte master cell with a separate address and flag-byte interpretation after verifying the published interface. Keep the original allocation metadata independent of Apple's internal heap structures. |
| DEC-2 address width | Add explicit 24-bit normalization at the 68000 data-bus boundary. Preserve full An values; retain strict map APIs for already-normalized logical addresses. Instruction/stack policy needs separate tests and review. |
| DEC-3 metadata | Caller-owned bounded native side tables first, with every byte charged to the Macintosh RAM budget. No hidden host-only capacity or allocation; no copied header/zone/list layout. |
| DEC-4 invalid arguments | Validate live ownership and mapped cells; return defined errors for NIL, invalid, freed and foreign objects. Malformed map/state is a distinct internal contract failure. No undefined native access or partial failed mutation. |
| DEC-5 size extremes | Reject negative/unrepresentable requests. Define zero-size objects explicitly with a nonzero physical reservation while retaining logical size zero. Exact maximum follows checked zone capacity, not an unverified historical constant. |
| DEC-6 initial bytes | Without CLEAR, make no content guarantee. CLEAR must zero the logical payload. Tests must not infer an initialization promise from reused RAM. |
| DEC-7 resize | Preserve the common prefix and leave object/state/data unchanged on failure. Nonrelocatable pointers and locked handle payloads keep their addresses. |
| DEC-8 trap flags | Accept SYS/CLEAR only for the specified allocation routines; preserve Get/SetTrapAddress's existing flag restrictions. Unsupported combinations get an explicit profile result. |
| DEC-9 disposal A0 | Select the later published register-preservation rule for the engineering profile; preserve the conflicting earlier chapter as an open historical question. |
| DEC-10 BlockMove | Overlap-safe copying in both directions, with complete span preflight and defined zero count; historical applicability stays separate. |
| DEC-11 heap size | Parameterize system/application zone bounds. Do not make either later manual's typical 128K system-heap size a January 1984 assertion. |

Read validated guest globals at call boundaries instead of trusting a stale
native cache. Guest writes cannot expand a zone beyond the originally granted
RAM or introduce overlap. System/application identity and current-zone selection
must remain distinct. Interrupt context is an explicit caller input until a
broader execution model exists; this layer cannot infer it from SR alone.

## Review required before finalizing

Independently check the cited published routine/register summaries, trap numbers,
flag-byte bit meanings and low-memory locations. Confirm what writes are part of
the public interface, without using internal headers, placement order or heap
algorithms as implementation inputs. Keep all later-manual/January 1984 gaps.
The drafts' test-plan cases that require a particular placement or successful
compaction must be expressed against an explicit original allocator contract.
Separate function-result errors from mapped-memory/internal-state faults.

Then finalize a bounded hosted manager contract over SPEC-0022, implement and
test original pointer/handle allocation/lifetime/size/lock/copy behavior, and bind
each trap separately. Add explicit data-address normalization tests before guest
handle-dereference probes. New native images/trials need their own admissions and
evidence; the previous admitted boot images are unchanged by SPEC-0022.
