# Native performance requirements

2026-10-23; SRC-0383..0385 / ADR-0076. Owner requirement and engineering plan,
not measured performance or a new implementation/target-run authorization.

## Goal

Make native performance a first-class requirement for Macintosh 128K/68000,
the proposed PC/AT 286 build, and native x86-64 UEFI (with the existing future
BIOS/UEFI shared-payload goal preserved). Seek the best practical speed within
each target's ROM, RAM, stack, disk and hardware constraints. Portability must
not become a reason to accept a demonstrated, avoidable performance loss.
No proof of globally optimal code or zero abstraction overhead is claimed.

## Shared behavior, specialized execution

- Keep the shared freestanding C semantics and independently checked reference
  paths. Compile OS/managers natively for each CPU; interpret only guest 68k
  application/compatibility code on PC. Equal behavior does not require equal
  instruction sequences, memory layouts or execution time.
- Improve algorithms, data layout, redraw area, copying and native service
  boundaries before speculative instruction tuning. Profile the actual hot
  path; eliminate unneeded work and repeated dispatch where measurements justify it.
- Select platform backends at build time where practical. Do not impose a
  per-byte/per-pixel callback or a weakest-target implementation on every build.
- Permit separately contracted, original architecture backends, intrinsics or
  assembly in architecture/platform layers when they provide a demonstrated
  benefit. The shared core remains architecture-neutral C. No backend, ABI,
  opcode, CPU feature or assembly implementation is admitted by this plan.
- Preserve guest endian/alignment rules, permissions, transaction/fault behavior,
  lifetimes and interrupt/device ownership. Checked bulk paths may amortize
  validation only under their proven contracts; optimization cannot erase checks
  or alter compatibility results. Keep a conformance/reference path.

## Target-specific questions to measure

| Target | Initial optimization questions, not established results |
| --- | --- |
| Macintosh 128K, original 68000 | ROM/RAM/stack cost, narrow dirty drawing, copy/fill cost, native trap overhead and interrupt latency within the existing hardware contracts |
| PC/AT 286, proposed 6MHz/640KiB/EGA | True C16 ABI, segmented access/normalization and 32-bit arithmetic cost, interpreter dispatch/fetch/native-manager crossings, EGA presentation and hard-disk I/O |
| Native x86-64 UEFI | Bulk memory and rendering, buffer/dispatch cost, alignment and target compiler output; any extra CPU feature needs a guarded, separately admitted contract |

Do not use a newer CPU, extra RAM, emulation acceleration, or an unqualified
compiler helper to claim performance on the baseline machine. Memory fit,
instruction eligibility, correctness and usable speed are separate requirements.

## Evidence and acceptance

Before a substantial optimization, freeze the workload and baseline artifact,
compiler/options, machine profile, counters and expected output/fault results.
Measure the candidate against that baseline on the same configuration. Record
time/latency, throughput where meaningful, ROM/code/data size, RAM/stack use,
measurement overhead, variability and any speed/space tradeoff. Set numerical
budgets from admitted hardware constraints and observations, not invented rates.

Use original workloads for interpreter operations, native-manager calls,
redraw/scroll/drag, input latency, boot/launch and file operations. Later, a
specifically admitted, 128K-appropriate Word edition should create/edit/save/
reopen documents at usable speed. No particular Word edition or usable speed
is yet qualified; native OS speed alone does not establish guest-app performance.

Compiler inspection and hosted conformance are preliminary evidence. Emulated
timing is an estimate with recorded profile/settings and sustained real-time
emulation; physical-target measurements are required for physical speed claims.
An optimization passes only after relevant semantic/fault conformance and
target-specific resource/performance checks. Keep regressions visible; reject,
revise or explicitly justify a measured tradeoff rather than hiding it in parity.

## Scope and next work

This is a planning/owner-requirement record. No benchmark case was executed,
fixture generated, code changed, tool installed, target run, release or external
worker call made. Existing source, native-application, trial and release gates
remain. Preserve current paired-native-preview-2 ordering; add bounded baseline
and optimization work where a measured bottleneck affects its acceptance.
The first 286 prerequisite remains the genuine C16 compiler/portability proof,
then a measured memory/backend contract before a full port.

For later substantial work, one worker may deliver a bounded patch plus useful
measurement artifacts; Codex integrates and verifies. Use a second worker only
for a distinct unresolved correctness/performance risk. Small records stay direct.
