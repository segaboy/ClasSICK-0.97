<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0120 - Guarded interpreter execution through native bank backing

2026-11-18. Original engineering bridge from SPEC-0069 / SPEC-0117 to the
existing SPEC-0034 guarded interpreter and SPEC-0118 address/frame instructions.
IMPL-0150 / TEST-0211. No historical implementation or protected program is an
input. This is executable interpreter integration, not System startup authority.

## Shared engine and compatible boundaries

Add `cs_m68k_bus_step_backend` and `cs_m68k_bus_backend` in
`core/cpu/m68k-backend.h`. Existing guest-map/view types and flat entry points
keep their signatures and behavior. Both routes use the same instruction
decoder, staged CPU frame, stack limits and guard events. Flat stores retain
their direct prevalidated pointer path; no per-byte callbacks are imposed.

The new route requires a guard and stack limits, strict 24-bit addresses and
S=1/T=0/I=7. It has no options, gateway, native service dispatch, return token,
image admission or EXEC override. Every A-line is refused before dispatch.
There is no new opcode set. No guest value selects a native callback.

## Backend contract

The trusted native descriptor supplies one context pointer and truthful size,
plus validate, logical-span preflight, scalar read and scalar atomic-write
functions. The context extent covers every retained data byte, including any
nested metadata or descriptors. All callbacks are synchronous, stable,
non-reentrant and do not retain supplied pointers. Backing and controls have
exclusive stable ownership throughout the complete step, including guards.
There is no concurrent retirement, rebind, DMA, remapping or mutable policy.

Validate checks binding/lifetime/geometry without reading backing. Span checks
a positive arbitrary 32-bit logical length fitting a single region, access
mask and permission, without accessing backing or returning native pointers.
Read accepts widths 1/2/4, checks complete coverage and even word/long addresses,
then publishes a big-endian value only on success. Write validates the entire
scalar and commits all bytes or none; a refusal has no external effect.
Writes do not modify backend controls or lifetime counters.

The engine preflights an instruction's possible store before staging effects.
After decode, target/stack/state checks and COMMIT guard, it invokes the final
atomic write. A write refusal returns ADDRESS with its guest-memory error,
address, width and WRITE access, preserving the original CPU frame and RAM.
Only successful write completion is followed by frame publication. Since this
route admits no service callout, no earlier external effect needs rollback.
Preflight is not a promise to skip final validation or a borrowed capability.
Only a single width-1/2/4 write is admitted by the current instruction profile.

## Native envelope and 286 adapter

Before any report publication, the generic entry verifies nonnull callback
slots and representable nonoverlapping native extents for descriptor, frame,
report, limits, guard, complete backend context and optional complete guard
context. Typed controls use the existing admitted ABI alignment checks.
Native objects must already be live and correctly aligned for their declared
types. Envelope failure returns ARGUMENT without changing any output. Normal
CPU, policy and backing failures may update the report, never CPU frame or RAM.

The concrete small-model 286 adapter owns `cs_pc286_m68k_memory`: a SPEC-0117
service plus its expected generation inline. Its descriptor context covers
that entire object. Zero it once, bind the member service with the generation
member as output, then construct the backend descriptor. Copying an active
context does not transfer its identity. Retire/rebind occurs only between steps.

The adapter additionally requires the ENTIRE declared conventional RAM arena,
including gaps, to avoid the current complete CS and DS/SS 64 KiB envelopes.
Thus native source fixtures, callback code, input/output/control objects,
policy contexts and stack cannot alias guest backing in the admitted near
small model. SPEC-0117 independently checks each actual bank's physical
extent, offset wrap, overlap, geometry and lifetime. Ownership of ordinary
RAM and absence of DMA/other owners remain native caller preconditions.

The new numeric-only `cs_pc286_guest_validate` and `cs_pc286_guest_span` expose
that admission without lending pointers. Span rejects zero/high access as
ARGUMENT, then binding errors, then invalid coverage ADDRESS, then PERMISSION.
It accepts arbitrary byte spans, without scalar alignment constraints.
The adapter maps native binding/lifetime/backing errors to CS_GUEST_MAP;
ARGUMENT/WIDTH/ADDRESS/PERMISSION keep their corresponding guest result.
The old guest enum is unchanged. A generation error is never refreshed silently.

## Original executable qualification

The native probe maps 128 KiB over eight scattered banks with odd boundaries.
It fetches a word straddling banks, jumps to code beyond 64 KiB, executes a
long store/load spanning three banks, and performs stack/address operations
and a return using a cross-bank stack. Exact physical bytes and CPU results
are checked, along with guard denial after preflight, final atomic-write
refusal, stale generation, address/permission failures, forbidden A-lines and
native control/context alias rejection. No selected disk or protected bytes
are executed. Mutants must demonstrate that both guard ordering and final
write-before-frame publication are checked.

The native harness pins sources/specs/helpers/toolchain/emulator before build,
checks unchanged identities after all tests, verifies the complete payload
loaded in RAM, preserves media hashes, and rejects runtime library imports.
Payloads exceeding the CLI's 16 KiB total inspection limit are compared over
bounded fresh deterministic replays; each replay verifies the complete payload
checksum in the original loader before execution. This is not one simultaneous
full-memory snapshot. Every replay must reach the same required diagnostic.
Existing flat bus/guard/startup suites are rerun after engine changes.

This closes one real bank-backed interpreter path only. Call/session, resource,
heap, Toolbox and application coordinators still retain flat views and have
not been migrated. A-lines, System boot/handoff, broad 68000 completeness,
physical hardware and full-runtime performance remain separate work. Native
bank geometry is currently revalidated per operand; no whole-OS speed claim.
