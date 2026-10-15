# SPEC-0035 - First loaded ROM application

2026-10-15, original engineering profile `first-loaded-rom-app-v1`.
SRC-0187..0190 / ADR-0039 / IMPL-0039 / TEST-0043. Human review pending.
One integration milestone: existing resource parser, CODE preflight, owned image
loader, guarded execution, explicit retirement and boot-screen result presentation.
No historical application/desktop/QuickDraw compatibility is inferred.

## Observable acceptance

A cold independently authored64KiB ROM initializes its own kernel services,
loads original CODE0/CODE1 resources, executes one original returning68k program,
then displays D0=97, its written0x5A5A5A5A pattern and six observed completion
phases. The native renderer consumes actual captured results, after explicit
cleanup, rather than substituting success constants. Mouse polling remains the
kernel demonstration's live input, not application Event Manager support.
Hosted integration, compiler construction and an actual boot observation are
separate results. An emulator/reference trial requires explicit owner go-ahead.

## Dedicated execution boundary

This implements the indispensable X-2..X-5 behavior for a deliberately limited
demo profile. SPEC-0034's broader profile remains incomplete: precisely one jump
row, one CODE1 segment, one ordinary root, no legacy session/resources/other
participants. Budget1..1000000, strict24, S1/T0/I7, even bounded A7; A5 is fixed
for this profile. All traps, callbacks, patches, alternate executors, interrupts,
dynamic loading and guest MM operations refuse. No x86 application/SDK.

The image's existing sole FUTURE/REFUSE inventory entry explicitly encompasses
its new embedded execution participant and one-use generation counter. This
is an explicit extension of SPEC33; existing external admission remains EMPTY
only. All original loader/RELEASE checks remain. No execution is granted by a
legacy description, ordinary guarded step, map flag, guest-selected native hook,
raw PC or numerical token. Native objects are live/aligned/disjoint/synchronized,
constructor-owned and immutable except through their APIs; there is no hostile
native-code sandbox or authentication of forged C objects/hidden pointers.

`cs_image_run` validates current/original maps/backings, native outputs and
controls, lifecycle/epoch/zone/globals/cells, exact owned locked records, inventory
and canonical world. It rejects all distinct physical backing aliases. It
preflights actual driver locals before claim; token has eight predeclared RX
bytes/four NOPs outside heaps/world/stack, present before PREPARE seals the map.
The consumed generation and CLAIMED state publish before any guest write. The
whole initial token write is policy-checked and map-checked within the reserved
stack; success publishes incoming A7 and RUNNING without a later fallible step.

The dedicated session constructs D0..D7/A0..A4/A6=0, A5 from layout, SR2700,
PC=first row+2, incoming A7=stack_top-4. The caller frame is preserved completely.
Only the actual interpreter runs. The owner policy is immutable/pure/non-reentrant
and is checked at actual accesses and before commit. A coordinator-only admitted
step may fetch READ-backed owned bytes despite non-EXEC base flags; ordinary
guarded/unguarded map semantics remain. This low-level native primitive requires
an already validated lease; it alone cannot create one.

Code fetch/read covers logical payload[segment+4,segment+length) only, not headers
or extent padding. A row fetch is only row+2 (JMP4EF9); its extension is exactly
four bytes at row+4 from that JMP, with no crossing. Data reads may use code,
above-A5, below-A5 or stack; writes use below-A5/stack only. Cells, low memory,
system heap, unowned payload and the RX token refuse. Actual current bindings,
globals, unrelated records/cells, owned handles and protected above-A5/table
bytes are checked throughout execution. Code immutability follows mediation and
the native host protocol, not a newly invented retained payload digest.

Only a successful RTS from the exact root token slot may select its token and
restore stack_top. It is intercepted before token opcode/data fetch. JMP/branch,
wrong-stack RTS and token data access refuse. Taken targets and complete proposed
PC/A7/SR/A5 are checked before any staged instruction store/frame publication.
An instruction failure keeps that instruction atomic; earlier effects survive.

State: READY -> CLAIMED -> RUNNING -> CLEAN-STOP or GUEST-STOP. CPU/policy/budget
stops retain the actual frame/effects; binding/integrity failures quarantine.
Native admission refusal has no guest effect; unsafe envelope refusal preserves
even report bytes. A consumed generation cannot enter a second root or resume.
Synchronous return establishes no active root/driver/callback/patch/pending exit;
no native service is callable during execution. Legacy owner APIs refuse while
the execution participant owns the image, including after a guest stop.

Explicit `cs_image_run_retire` accepts only stopped integral ownership and the
same generation/epoch/inventory. It validates every destination, scrubs exactly
below-A5 to zero, retaining actual scrub counts, stack bytes and separate stop
snapshot, then hands back to original SPEC33 RELEASE for canonical world clear
and reverse owned-handle disposal. Failures quarantine and retain actual state;
no guessed rollback, retry or automatic cleanup. Original RELEASE is not weakened.

## First artifact and machine budget

Original resource source366 bytes; CODE0 above40/below16/table8/offset32;
CODE1 header first0/count1 then the original12-byte MOVEA/MOVE/MOVEQ/RTS body.
The literal routine offset4 is an engineering choice; AL-2 remains unverified.
The loaded JMP plus four instructions produce five successful interpreter steps.
Original negative programs use the same PREPARE/LOAD pipeline.

Dedicated boot reset SP8000, native stack[2100,8000), native data/BSS[B00,2000),
workspace[8000,1A000), framebuffer[1A700,1FC80), all below128KiB RAM. Workspace
has distinct translated guest low/RAM buffers, not a mapping of native controls.
Only the optimized native ROM profile is built; twin hashes, whole ROM seal,
reset/exception/Line-A encodings, no undefined link imports, core mutable-global
absence and a conservative sum of nonrecursive function frames plus192 assembly/
exception bytes must fit its stack reserve. This proves that static profile's
layout, not a full System1/physical/cumulative observed128K OS result.

## Required evidence and remaining gates

Redraw refinement (2026-10-15, SRC-0193): preserve the existing complete-frame
output while using original project masked-byte fills for the overlay. A separate
native counter update validates a512x342 mono surface before writing only
x[352,432),y[286,306), displays presses modulo10000 using the same original glyphs,
and preserves every other pixel, row padding and tail byte. Descriptor/storage
are ordinary live/disjoint native objects. The boot loop continues polling and
recording ticks, but redraws only this counter when presses changes. No retained
render state, second framebuffer, guest API or historical behavior is introduced.
Use pre-change original frame output, padded/canary and rejection checks, then
an independently authorized own-ROM trial to assess practical responsiveness.

Run complete original-load/result/retirement tests, malformed/denied control/data
paths, budget and single-use/refusal effects, protected native/backing aliases,
balanced/forged returns, protected headers/rows, clean versus quarantined cleanup,
and result-derived rendering. Compiler/regression/CI are grouped at this integrated
checkpoint. A host-generated preview is explicitly not an emulator screenshot.

Both original-System1/ROM directions, R-001/AL-2 and prior open source questions,
native-app, hardware/parity/three-edition/B2criterion6 and human provenance gates
remain. No prohibited implementation/sample/asset, raw draft, emulator source,
new agents/chats/automation or OneNote. Preserve24 originals/private identity copies.
