# SPEC-0036 - First loaded application rectangle calls

Version1, 2026-10-16; owner continuation SRC-0202; source check SRC-0203.
Original engineering profile `loaded-rectangles-v1`, not complete QuickDraw or
January1984 compatibility. Extends SPEC-0035's dedicated one-row/one-segment
root solely with the two fixed builtins below. The original cs_image_run entry
and ordinary guarded CPU entry continue refusing every A-line.

## Published facts and explicit choices

SRC-0071, identified owner's 1985 volume set: I-140/141 defines Rect's signed
integer top,left,bottom,right order and empty/inverted rectangles. I-149 describes
off-screen drawing and clipping; I-177 documents PaintRect with pen pattern/mode
and EraseRect with background pattern in patCopy mode. Neither changes the pen
location. Appendix C III-219/216 gives A8A2/A8A3. The documented Pascal-stack
convention passes the Rect address and the procedure consumes its four-byte
argument (I-91/94/95; existing SPEC-0029 review). These are later-manual facts.
Selected routine/trap pages were checked as images; no manual sample is used.

Engineering choice: a host-owned 128x64 off-screen mono port, stride16, MSB-first,
origin0, rectangular clipping to [0,128)x[0,64), black pen in patCopy and white
background. Only this selected pattern/mode/clip case is supported. No guest
GrafPort, InitGraf, regions, color, pen setters, picture/region recording,
custom grafProcs, callbacks, patches, dynamic loading or interrupt contexts.
QQ-1..13, R-001, AL-2 and edition/layout questions remain open.

## Native port helper

`cs_qd_rect_port` contains pixels[1024] and native diagnostic paints/erases.
Its constructor is ordinary caller zeroing. `cs_qd_rect` accepts exactly eight
borrowed big-endian Rect bytes and canonical word A8A2 or A8A3. Native objects
are truthful, live, aligned, synchronized and mutually disjoint. No allocation,
libc, mutable global, pointer retention or callback. Null/unknown operation
refuses before any output change. Signed16 coordinates are promoted before
clipping; empty/inverted/outside rectangles succeed without pixel changes.
Logical right/bottom boundaries are excluded. Paint sets covered bits, erase
clears them; every other bit survives. The selected native diagnostic counter
increments even for empty calls, saturating at UINT32_MAX. This is not a guest
global or historical field. No pen position exists to mutate in this profile.

## Owned execution and trap completion

New explicit `cs_image_run_rectangles` borrows a native port synchronously.
The common execution envelope adds its full fixed extent to the disjoint known
controls/outputs, against all image/manager/lifecycle/map/inventory controls and
all guest physical backings. Ordinary entry uses no port. Neither native entry
admits a guest-selected handler, generic dispatcher or host function pointer.
The image's existing exclusive execution participant encompasses this borrowed
nonretained port for the duration of its one synchronous root.

Keep original generation/epoch/owner, native-local preflight, entry token,
instruction budget, strict24, A5/SR/A7, protected tables/cells/header, actual owned
code/data and return/retirement checks. A successful rectangle call counts as
one instruction. Canonical A8A2/A8A3 only: auto-pop, OS, reserved, unknown and
all other A-lines refuse before service effects. No gateway directory mutation
or fallback, saved builtin chaining or arbitrary direct veneer is added.
The two directory slots must still be unsupported at admission and at each
call; a previously installed patch refuses before entry. Availability is only
through this explicit fixed-port coordinator, not a globally installed QuickDraw
manager. Get/Set/direct-call equivalence and default-port installation remain
unimplemented. No misleading logical builtin address is invented here.

For each permitted actual opcode, independently preflight the owned opcode,
four-byte argument at A7, eight-byte even guest Rect span, six-byte exception
scratch at A7-6 and the owned continuation PC+2. Rect reads use only current
owned payload/below-A5/above-A5/stack ranges under the existing owner policy.
All argument/scratch bytes stay in the reserved stack; parameter pop must not
consume the root token slot. Prepare a frame preserving all D/A registers and
SR except PC+=2 and A7+=4; validate it with the owner policy before any effect.
This deterministic preservation is an engineering choice within the published
convention, not a claim about historical scratch-register/CCR output.

Snapshot the Rect bytes before commit. A successful call writes saved SR and
continuation PC into the already validated six-byte scratch, paints the native
port and publishes that frame. All destinations and computation are established
before the first effect; there is no fallible action after commit begins.
Malformed calls keep that call's RAM/frame/port atomic. Earlier guest stores and
successful drawing survive later CPU/policy/budget stops; faults are not rolled
back or disguised. Caller frame stays unchanged; the actual stop frame is in
the report. No resume or second execution in a consumed generation.

Explicit retirement uses the original stopped-owner path: scrub below-A5 and
release its locked CODE handle. Port output and diagnostic counts survive
retirement as caller-owned results; no native reference is retained by the image.

## Original artifact, rendering and verification

The original resource/CODE loader admits a 40-byte independently authored
program. It builds an outer Rect(8,8,56,120), paints, changes it to
Rect(16,16,48,112), erases, returns D0=97 and RTS. Including the loaded JMP this
takes13 successful instructions and yields one paint, one erase and2304 black
pixels. Source/staging capacity may grow for this fixture; existing constructor,
default application and explicit retirement semantics remain intact.

Render the actual retained128x64 pixels into x[192,320),y[136,200) of the original
512x342 scene after its normal result rendering. Clear the prior checksum-bit
visualization's interior x[52,460),y[132,204), preserving its frame and every
phase/progress mark. Keep the actual checksum in native result diagnostics.
Native counter updates remain
disjoint at x[352,432),y[286,306). This is a display copy of guest-produced pixels,
not a simulated application result. The small native port costs1024 pixel bytes,
not a second full framebuffer. Pattern/result fields derive from actual guest
return and a checksum of port bytes. Preserve the old released alpha artifacts.

Original pixel/set oracles cover signed extremes, empty/inverted/clipped ranges,
bit edges, mixed sequences, unrelated pixels and invalid helper inputs. Loaded
programs test both traps, caller/stop/stack effects, invalid pointers/bounds,
unknown/flagged calls, continuation/return forgery, budget-retained drawing,
native/backing aliases, cleanup and single use. Compiler/CI, old regressions,
original assembly encoding, no-runtime link, static bounds and ROM twins must
pass. No reference pixels, manual sample code or emulator source. New ROM is a
candidate, not a release or boot observation. Actual trial needs fresh owner
go-ahead; all native-app/historical/physical/full128K/parity/edition/B2/human and
both required System1/ROM gates remain.
