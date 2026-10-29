<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0082 - PC286 span construction and conversion experiment

2026-10-29;SRC-0536..0539;SPEC-0069;IMPL-0090;TEST-0104.
Adopted with changes for an isolated probe. No production guest ABI,provider,
link,boot or physical-memory access is admitted. Flat68000/x64 paths are unchanged.
[Executed evidence](../development/pc286-span-build-evidence.json).

## Inputs and representation

The unchanged bank-plan/catalog/store4 contracts apply. A numeric catalog entry
supplies bank ID,physical base and native offset. The real-mode relation is
`physical = 16 * segment + offset`;Intel's published October2013 core developer
manual,section5.2 printed/PDF66,explicitly describes compatibility with80286
real mode (SRC-0539). Only this prose addressing relation,paragraph bases and
64KiB windows are used. No sample code or emulator implementation is an input.
The strict sub-1MiB envelope and refusal of wrap are OUR engineering restrictions;
they are not a claim that every real-mode address lies below1MiB.

## Numeric construction:cs_bk_plan

Refusal order:NULL output/count is ARGUMENT;then whole cs_bc_validate gives
ARGUMENT,MAP or BACKING;then REPRESENTATION;then CAPACITY. All refusal paths
preserve the output array AND count. Caller-provided capacities/extents are true.
Representation requires native_offset<=physical_base,and their difference is
a paragraph multiple below1MiB. The encoded0:0 pair is refused as REPRESENTATION
because store4 requires non-NULL data. This conservative policy also applies to
host simulation. Segment0 with a nonzero offset remains admissible.

Success emits chunks in MAP BANK-ARRAY ORDER,independent of catalog/ID order.
Each bank is covered contiguously from offset0 by1..3 chunks of1..32767bytes.
Each chunk contains bank,logical bank offset,bytes,segment and native offset.
Catalog admission ensures native_offset+bank bytes<=65536;no chunk crosses its
64KiB window. The retained general span capacity is192. For this nonoverlapping
sub-1MiB profile,the tighter reachable maximum is95:with n<=64 banks and k
chunks,bytes>=n+32767*(k-n),so k<=64+floor((1048576-64)/32767)=95.
The95-chunk fixture uses31 banks of32768bytes,32 of1byte,and one of2bytes,
contiguously from physical16,with reverse IDs. This is an original bound/fixture.

## Representation conversion:cs_bk_bind

Refusal order:NULL chunk/output,zero count,or the wrong arena convention gives
ARGUMENT;then count>capacity gives CAPACITY;then any invalid chunk gives
REPRESENTATION. Every chunk is checked before any output effect. Require
bytes1..32767,native_offset+bytes<=65536,linear start<1MiB,end<=1MiB,and
segment:offset !=0:0. Refusal preserves all output bytes. It copies bank/offset/
length labels and converts the data pointer;it does not validate those labels
as a complete logical binding or establish ownership. Store4 still validates
the whole logical binding before access. Bind need not enforce the planner's
95-chunk bound;its caller supplies a truthful count/capacity.

Host:arena is a live1MiB byte array;data=arena+16*segment+offset.
Pinned Open Watcom16:arena MUST be NULL;the single conversion is integer-to-far
pointer with segment in the high16bits and offset in the low16bits. This is
implementation-defined,qualified only for the selected compiler/models.
A sizeof(data-pointer)==4 guard refuses ms/mm for THIS arbitrary-span profile;
it does not prove that every alternative near-data ABI is impossible.
mc/ml and od/os compiler closure/import results are in the receipt.

## Caller obligations and limits

P1-P6 of the [binding proposal](../development/pc286-native-binding-proposal.md)
remain prerequisites:true live extents,physical nonaliasing,stable exclusive
ownership,no concurrent remap/callback,and infallible stores. Input descriptors,
output arrays,count storage and backing data must not overlap during calls.
On C16,each metadata array/object must itself fit an addressable segment window;
no huge-pointer normalization or cross-window metadata traversal is supplied.
Output-count preservation assumes it does not alias output storage. No forged
native pointers are dereferenced in a validation test. Numeric validation and
host simulation cannot establish native ownership/lifetime/physical validity.

The unchanged store4 adds a checked16-bit in-span delta;the admitted window
prevents offset wrap. Selected own compiler listings confirm the conversion
word order and offset-only add;zero unresolved imports exclude helper calls in
this compiled closure. This is not a full instruction/ABI/alignment/stack audit.
Diagnostic -s suppresses stack checks only for this unlinked experiment;there
is no production stack-policy waiver. Native linking,provider admission,actual
RAM use,target execution and measured cost remain open. No ROM,CPU,Mac device,
emulator,reference,hardware,native-application or release gate is closed here.
