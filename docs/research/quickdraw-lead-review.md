# QuickDraw research intake - 2026-10-13

SRC-0117; research considered before SPEC-0026's separately authorized call
service. The supplied draft's implementation/probe suggestions are research,
not instructions or evidence that a graphics contract is ready.

The Downloads copy matches the untracked repository draft exactly: 37,671
bytes, SHA-256 `5b4823cc6a498c00a74476dd16846c24c84adfb169b55b5abd62e0fcc104a881`.
The companion test plan is 16,831 bytes, SHA-256
`33d744e913749c1a63f0c75cd8b5c7c62e3cef4546265192ddae9dbf01bbaf81`.
Both are preserved unchanged, untracked, with intake copies outside Git.
No QT/OB tests have executed and no original fixture or layout is imported.

## Useful carry-forward and source scope

The drawing model, edition comparisons and original per-pixel/set oracle
strategy are useful review inputs. Explicitly retain the distinction between
documented facts, inference, engineering choice and unresolved historical
behavior. The report's full page-verification and exposure claims remain
attributed, not independently established here.

Selected checks used the existing eligible SRC-0071 copy: full hash/page count
rechecked, I-148/149 (public port fields and visible-region distinction), I-185
(region predicates), I-188/189 (CopyBits/picture prose). Printed footers matched
in extracted text; no new visual-table or Promotional Edition verification is
claimed. The draft's interleaved Promotional Edition page mapping must not be
replaced with a single chapter offset. Reuse already reviewed I-163/A5 facts
from SRC-0114. Only interface prose/field facts are inputs; no manual sample,
private encoding, equate implementation, artwork or Apple resource is adopted.

I-294/295 were additionally read for the no-parameter returning action
procedure shape used by SPEC-0026, together with earlier I-91/95/97 review.
This confirms a calling shape, not availability or behavior in January 1984.
It installs no Window Manager callback or new routine-registry record.

## QK choices at this intake

| Item | Carry-forward disposition |
| --- | --- |
| QK-1/2 | Keep the 108-byte/1985 candidate and the 106-byte alternative with explicit edition labels. Final graphics layout/default/profile selection and each field offset need their own checked contract. No layout is implemented or claimed for January 1984. |
| QK-3 | Pattern-origin alternatives stay open. A future engineering profile may choose one explicit rule, with distinguishing oracle fixtures; no unconditional pixel expectation adopted now. |
| QK-4 | Defer the broad hidden-pen proposal. The picture-recording statement alone does not prove identical behavior for every drawing operation and off-screen port. |
| QK-5 | Exact RectInRgn is a reasonable future engineering choice, not historical parity. I-185 explicitly permits bounding-box false positives; exact answers may change a program's observable result. |
| QK-6 | Preserve the literal CopyBits clipping passage and its separate open question. Do not silently replace it with the ordinary four-region drawing intersection. |
| QK-7 | Adopt original algorithms and independent oracles as the method. State invariants per operation; a pen can extend below/right of a path, so a generic enclosing-rectangle rule cannot apply to every primitive. |
| QK-8 | Defer foreign picture decoding until eligible format evidence exists. An original picture format would be an explicitly noninteroperable engineering format, not a compatible PICT representation. |
| QK-9 | Defer the base-address equality heuristic. It does not establish every overlapping/subimage case of being displayed on screen. |

QQ-1 through QQ-13 remain open. The draft's general four-way clipping formula
must allow its off-screen visible-region exception and operation-specific
CopyBits rules. Its early "only SetPort" summary must retain OpenPort/InitPort
exceptions already listed in the body. Size arithmetic is not a recovered
private layout; unknown region/picture formats and style/random algorithms
remain unimplemented. Original assets/font contracts are separate dependencies.

## Relation to the next runtime step

QuickDraw's bottlenecks are a later consumer of typed guest calls, with their
own selected signature and edition-specific parameter shapes. The safe next
step is the zero-parameter returning service plus checked JSR/JMP, bounded
stack/return/depth/budget checks and original execution probes. No inference
from a grafProcs pointer supplies a convention. Parameter/result marshalling,
QuickDraw managers and its draft test suite follow independent review.

No graphics implementation, copied assets, reference observation, emulator/VM
run or source waiver. Native-application gate and existing historical/physical/
128K/parity/three-edition limits persist. Codex-assisted lead self-review;
Claude's drafts remain research, human provenance review pending. Wiki queued.
