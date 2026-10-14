# Hosted heap-lifecycle execution evidence - 2026-10-14

SPEC-0030 v2 / ADR-0034 / IMPL-0034 / TEST-0038 / SRC-0151..0154.
[Machine-readable record](heap-lifecycle-evidence.json) retains exact identities.
Lead self-review; human provenance review pending. Compiler/local verification
is complete; publication CI and its exact revision are pending in this initial record.

The original host coordinator retires application allocations while preserving
system state and admitted resident scrap. Reset and teardown use distinct phase
labels for the same required scrap preservation. No guest trap, loader, manager
callback or boot entry is bound. [API/lead review](heap-lifecycle-implementation.md)
states ownership, report tags, worker dispositions and limitations.

## Local execution and corrections

The full nine-profile matrix passes1713 CTests:203/203/203/176/203 with
Clang (debug twins/release/i686/ASan+UBSan), then188/188/188/161 with GCC
(debug twins/release/i686). All existing verification-chain checks complete
through typed Toolbox objects, own unloaded EFI/media and prior subsystem
regressions. Existing UART checks run without body inspection or modification.

Grok's bounded review prompted clearer post-publication fault diagnostics and
additional independently authored checks; the final affected11 lifecycle tests
pass in every profile (99), retaining1614 unchanged regression checks. These
counts distinguish final execution from development repetitions and proposed
research/worker cases. Normal tests cover both operations for logical sizes0..34,
exact boundary/refusal matrices, live-zero versus NIL, persistent system scrap,
single-use/cancel/foreign plans, changed input and unsafe-report handling, raw
address reuse, epoch saturation and saved CPU/control preservation. The test-only
suite injects an internal failure after every counted write boundary for a
five-byte scrap reset and teardown, checks actual published-handle validity and
retained first fault, and refuses subsequent coordinated manager/lifecycle entry.
No new guest instructions, application binary, reference case or boot is run.

The first full attempt stopped on GCC's misleading-indentation warning; the
line was split. The subsequent complete regression matrix passed, but the new
32-bit Clang O0 object audit exposed an unintended memcpy helper from a constant
request initializer. Explicit scalar field initialization removes that dependency.
The final99 affected checks, ten object audits and both compiler debug twins pass
on the corrected source. An additional development assertion expected STATE for
an initialization alias; the zeroed cell band correctly produces ALIAS, so that
fixture expectation was corrected before the final matrix. Initial failed output
roots/logs remain preserved outside Git; no earlier failure is described as a pass.

The final lead review found a separate ownership edge: a stable controller could
prepare into another controller's still-active plan. An explicit occupied-plan
refusal and original cross-instance prepare test now preserve the foreign plan.
New plans start zeroed. The earlier published candidate and its CI are retained
as superseded history; the corrected source requires its own final checks and CI.

Production width/optimization audits cover Clang/GCC32/64-bit at O0/O2 plus
little/big-endian AArch64, allowing only existing original checked-map/manager/
gateway imports. No writable global, external helper or test-injection symbol
is admitted. Debug executable twins match each compiler; hashes are in JSON.

## Original 68000 compiler accounting

The pinned GNU68000 compiler builds guest-map, manager, gateway and lifecycle
at O0 and Os: eight module audits and two complete relocatable links with no
external imports or mutable data. These are compile/link checks, not execution
on a Macintosh or emulator. Complete source/tool fingerprints and all function
frames are in the manifest. Earlier provisional compile-only outputs are retained;
the final manifest is `m68k-final3/manifest.json` in the private slice workspace.

68000 controller/plan/entry/report/scrap/request sizes are112/5200/28/92/20/72
bytes. With64 caller inventory entries, one report/request/descriptor and one
plan/controller, fixed coordinator storage is7288 bytes plus caller staging.
The plan already contains its copied inventory/manager/gateway metadata; those
are included in its5200 bytes. The existing live manager and other session/OS
state remain separate costs. Maximum individual closure frame sizes are608/O0
and604/Os (the existing manager); these are neither simultaneous stack usage nor
measured high-water. No final ROM, cumulative stack or128K proof follows.

## Review, identity and boundaries

Claude supplies12 unexecuted failure proposals/six gaps from three Read-only
inputs; Grok maps14 requirements and supplies five candidate defects from four
Read-only inputs. Both finish within their turn budgets. Their scope/exposure,
lead dispositions, report hashes and client-reported usage are SRC-0152/0153;
raw prompts/reports/streams remain private. Final code includes lead corrections
beyond the frozen review packet. No worker test execution or independent hash
computation is claimed. Token/cost estimates do not identify invoices or quotas.

All24 research originals and byte-identical intake copies remain unchanged and
untracked. Existing boot sources, assets and thirteen interface records remain
unchanged; the complete boot-media result equals the previous verified checkpoint.
No HQ content is exported into public source. Earlier exact-image VM/operator
observations retain their original identity; none executes this coordinator.

The native-app gate, prohibited inputs, UART body boundary, historical questions,
physical/128K/parity/three-edition/B2 criterion6 and human provenance review remain
open. No new VM/emulator/reference run, recovery, disk scrap, system scrub,
callback closure or arbitrary application readiness is implied. Next: a bounded
Resource Manager service/lifetime contract before CODE-only loading; the seven
boot/asset owner decisions can remain pending during that design.
