# Mac pointer source-sufficiency review - 2026-10-19

SRC-0314/0315; continuation of SPEC-0051 / ADR-0059. This completes the
first-slice **source-sufficiency assessment**, with explicit adapter blockers.
It does not complete or authorize a hardware motion binding.

## Owner disposition and exclusions

The owner answered **"Continue the narrow published-source review"** to the
sanitized exposure question. This resolves the held source-review lane recorded
at public c2795041a1ee4aefd026cdda037c675af8f37718. The earlier receipt remains
an accurate historical checkpoint. The unrelated sound declaration remains
excluded, unused and undistributed; there are no affected implementation paths
or commits. No fresh implementer was required by this owner disposition.

Review is limited to published mouse prose, direction/interface tables and
figures. Printed symbolic declarations/sample code are excluded from adoption;
the button-bit fact below is independently visible in the circuit diagram.
No manual text, page images or protected assets go into Git or worker packets.
The owner answer grants no implementation, target run or release authority.

## Source identity and visual check

Already-admitted SRC-0080, *Inside Macintosh* Volume III, 1985 edition,
eleventh printing June1990: 50,687,193 bytes, 290 PDF pages, SHA-256
`4c0de9a3a4d41c41b8462b2a77aad8b95b92e1980afff165c1f986da0a81988b`.
Identity was rechecked. Printed III-27, III-28 and III-29 correspond to
1-based PDF34,35,36. Footers and the direction table/Figures5,6,7 were
visually checked, using only the mouse portion of III-29. Private renders and
their identity manifest stay outside Git. No further SCC/VIA manual pages
were reviewed in this continuation.

## Verified published interface facts

- III-27 describes relative movement and quadrature, with paired signals on
  each axis. X1/Y1 reach SCC DCD A/B; X2/Y2 reach VIA port B.
- III-28 Figure5 and III-29 Figure7 identify X2 as PB4 and Y2 as PB5.
  Figure7 identifies the switch as PB3. III-27 describes a pressed switch
  grounding its line and checking the button during vertical blanking.
- The III-27 table gives the following direction interpretation; Figure5
  independently illustrates the downward Y case.

| X1/Y1 transition | X2/Y2 level | X direction | Y direction |
| --- | --- | --- | --- |
| Rising | Low | Left | Down |
| Rising | High | Right | Up |
| Falling | Low | Right | Up |
| Falling | High | Left | Down |

Figure6 supplies the connector mapping, cross-checked with Figure7. These are
published interface facts, not Apple software or a register-programming recipe.
The prospective engineering conversion to SPEC-0051's positive-right,
positive-down coordinates must preserve these directions. A raw SCC status-bit
transition cannot be substituted for X1/Y1 polarity without its own proof.

## Sufficiency and unresolved prerequisites

**Sufficient for a signal-level interface description; insufficient for a
complete, safe motion adapter.** This is a conclusion about the reviewed pages,
not a claim that eligible documentation is unavailable elsewhere.

| Gap | Required before Mac adapter admission |
| --- | --- |
| M1: SCC programming | Eligible register/status, DCD polarity, external-status enable, reset/acknowledgment and interrupt-priority semantics; exact addresses/access rules |
| M2: VIA ownership | Reviewed input-direction and read semantics, preservation of unrelated bits/functions; coherent button/quadrature observations |
| M3: CPU/loop binding | Exact interrupt routing, vector/mask/re-entry and startup/shutdown ownership; compatibility with the existing native loop |
| M4: transition integrity | Both edges, simultaneous axes, initial baseline, sampling order, bounded latency and detectable versus undetectable loss; no assumed queue/history in hardware |
| M5: delivery/recovery | Original bounded capture/accumulation, saturation/drop/cancel policy, publication from one native owner and teardown/fault handling |
| M6: applicability | Separate verification of target Macintosh/SCC/VIA revisions, Snow model behavior and physical hardware; 1985 text does not establish January1984 software behavior |
| M7: cost/evidence | Measured ROM/RAM/stack/interrupt cost and separately authorized exact-image directional tests; no present test result |

The button's vertical-blanking description does not prove that sampling motion
once per frame preserves edges. No polling decoder, interrupt routine,
controller initialization or Apple motion/scaling algorithm is adopted.
Any additional published interface manual needs its normal eligibility and
source review before use; neither browsing nor a hardware/reference trial
is performed in this continuation.

## Completed slice and next work

The first contract/inventory slice is complete: SPEC-0051, 68 unexecuted
TEST-0061 identifiers, the PC evidence inventory, and this qualified source
assessment. Mac adapter admission remains blocked by M1-M7. PC E2 delivery
still does not establish GUI/human reliability. Release2 is not ready.

Next recommend the **bounded cursor composition contract** using the existing
pointer contract and original art/memory limits, before implementing shared
pointer/cursor helpers. Resolve live-buffer binding, save/restore ordering,
dirty-area repair, clipping, revision exhaustion and footprint; use one external
primary for that engineering draft, with a second reviewer only for a distinct
unresolved risk. Manuals remain lead-only. This recommendation itself installs
no API, art, fixtures or code and grants no new execution authority.

No new worker call was needed for this lead-only source review. Existing Claude
and Copilot work is reused unchanged. All four external remaining allowances
remain unavailable; no quota saving or fresh provider usage is asserted.
Ordinary document guards check records only; none of the68 feature cases ran.

[Input/evidence inventory](pointer-source-and-input-inventory.md),
[prior contract checkpoint](pointer-input-contract-review.md),
[release2 plan](../../releases/paired-native-preview-2.md).
