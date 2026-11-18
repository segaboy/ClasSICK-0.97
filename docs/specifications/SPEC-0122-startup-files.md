<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0122: startup file selection and resource-source preparation

2026-11-18. Original engineering composition of SPEC-0119 boot-header metadata,
SPEC-0115 MFS lookup and SPEC-0116 resource transport. No new historical format
fact or executable admission. The native product entries currently run original
service/application probes and a scene; this additive service does not change them.

## Operation and publication

Read exactly boot blocks 0 and 1, decode the admitted common header, and mount
the same immutable device. Resolve the System and Shell fields, in that order,
with the existing first exact counted-byte MFS lookup. Embedded NUL/high bytes
are ordinary bytes. No case folding, fallback filename, search directory,
duplicate-name policy or interpretation of version/page/heap fields is added.
Equal names can select the same entry. Empty names use the existing exact lookup.
The seven header strings retain their existing length validation.

For each selected file, validate its resource-fork chain, bind the stream source,
and open its full admitted resource-map profile. An empty or invalid resource
fork is a classified refusal, never an implicit skip. Neither data-fork content
nor resource payload is loaded. Permitted metadata cache read-ahead is unchanged.

Use an explicit caller workspace for construction. On complete success copy its
candidate into the destination and rebase adapter volume/entry pointers and both
resource-view source contexts to that destination. Publish no part before both
opens succeed. No fallible call follows the copy. All output bytes, including
padding, are preserved on every failure; argument refusal also preserves report
and workspace. Other failures may change report/workspace/provider state. A later
failed preparation may reuse the same workspace without changing an existing
published session, whose disk scratch remains usable after that failure.

The session owns header/volume/file/adapter/view metadata and borrows the live
workspace's disk scratch plus the provider context/media. It must stay at its
original address; copying or moving it invalidates internal pointers. Media and
retained controls remain immutable, accesses exclusive and non-reentrant. Native
objects are truthful/disjoint and each fits one native segment. Only exact
top-level object aliases are detected; arbitrary overlaps are preconditions.

## Diagnostics and scope

Return OK, ARGUMENT or REFUSED. The separate numeric report identifies header,
mount, find, bind, open or complete stage; System/Shell/no-file role; boot/MFS/
resource status domain; original status; resource field/position; and completed
open/type/reference/warning totals. Checked sums exclude a failing open. There
are no filenames, resource keys/IDs, payload bytes or payload digests in reports.
Names remain internal controls needed by lookup. Sources expose the existing
bounded resource transport, not an executable entry or OS Resource Manager.

No claim of ROM dispatch, INIT selection/order/execution, Resource Manager traps,
memory placement/sizing, executable admission, Finder launch or System boot.
Published 1985 InitResources describes opening the System resource file, but
does not justify inferring an executable INIT policy from this operation. This
engineering coordinator selects the two explicit header names; its paired
preparation/atomicity contract is original and is not attributed to Apple.

## Verification

Original generated fragmented MFS media includes counted binary filenames and a
resource map beyond64KiB. Host checks cover exact/missing/case-mismatched names,
raw metadata, both post-return sources, duplicate first-match/equal-name behavior,
header/mount/bind/map/provider refusals, whole-output preservation, success after
failure and reuse of a prior session after a failed preparation. Deliberate
pointer-rebase/publication/lookup mutants must fail the unchanged checks.

Execute native Mac68000 and 286 probes using current modeled block providers.
After return, erase the construction candidate and reopen both published sources
to detect borrowed construction pointers. Late second-map failure checks every
destination byte; cold recovery repeats the baseline. Retain exact input/source/
tool identities and measured code/BSS/frames/cycles. The larger isolated Mac
probe declares and enforces its own data/BSS and stack bounds below framebuffer
0x1A700; this is not a change to product memory layout or physical evidence.

An explicit optional DC42 route uses the reviewed container helper, private
external copies and numeric diagnostics only. Original generated inputs qualify
it; selected owner media is operated only by the coordinating agent. Classify
refusal separately from harness validity. No protected bytes enter fixtures/Git.
