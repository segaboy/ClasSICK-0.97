<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Native286 banked memory provider - 2026-11-18

SPEC-0117 / IMPL-0147 / TEST-0207 turn the earlier numeric/far-access probes
into an original native small-model service. It copies bounded region/bank
descriptors, binds an exclusive ordinary-RAM arena and issues a monotonic
generation. Scalar1/2/4-byte big-endian operations can cross scattered banks
within one logical region. Every destination is resolved before the first
physical write; failed admission preserves the output/backing. Retire/rebind
invalidates earlier generations. No native far pointer escapes the service.

Canonical report: C:/Repos/.task-temp/pc286-banked-guest-20261118-d/report.json.
Five native BootHush outcomes pass:

| Case | Payload bytes | Modeled cycles | Marker |
| --- | ---: | ---: | --- |
| Optimized | 4685 | 580699 | B8 |
| Unoptimized | 7086 | 1025675 | B8 |
| Wrong byte order detected | 4682 | 100778 | E1 |
| Missing generation check detected | 4678 | 237576 | E2 |
| Premature write detected | 4717 | 251063 | E3 |

The original diagnostic observes actual far RAM and canaries, a128KiB logical
mapping, four one-byte banks, cross-bank reads/writes, offsetFFFF,24-bit and
native-offset wrap refusal, copied descriptors, stale/copy/retire/rebind
behavior, permissions and native-segment/backing alias rejection. The entire
loaded payload matches in each run. Sources/spec/helper, selected pinned
Watcom tools, BootHush and media identities are checked before/after.
No default/runtime library is linked. The emulator is the exact7adb8a74 build.

An early probe exposed an inline selector-helper clobber declaration problem;
declaring AX modified fixed the native ABI. Failed outputs a/b and intermediate
c are retained; final-d includes24-bit/offset-wrap controls. Independent review
found no concrete bounds, lifetime or atomicity defect under the documented
preconditions; that review did not cover the harness. Root separately read
the implementation and interface. Worker effort/usage were not measured.

The native service occupies670 bytes. It requires small-model DS=SS and stable,
exclusive caller-reserved conventional RAM disjoint from entire code/data/stack
segment envelopes. It is not an allocator, physical ownership detector, MMIO
provider or multi-owner synchronization scheme. Geometry is revalidated on
every operation; integrated performance and stack high-water remain unmeasured.
The existing flat guest API and interpreter are unchanged by this slice.
Earlier isolated far-pointer prototypes already existed; this adds ownership
and binding integration, not the project's first-ever far memory observation.

Reproduce with scripts/Test-PC286BankedGuest.py, the pinned native Watcom
toolchain, exact BootHush executable and a fresh external output directory.
No private disk, external implementation, UART body, HQ material or excluded
research was used. System/Finder execution and physical validation remain open.
