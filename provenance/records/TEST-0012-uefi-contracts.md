# TEST-0012 — UEFI preboot data, ownership and finite exit model

Date 2026-10-08. SPEC-0006 / IMPL-0010 / ADR-0010, original fixtures and public
interface prose only. No reference system, firmware callback, VM launch or OS boot.

Protocol: full Clang Debug twins/Release/actual i686/x64 ASan+UBSan matrix now
43 checks per configuration; GCC portable/core/preboot matrix 28 per Debug twins/
Release/actual i686. Existing sanitizer/debugger/endian/core-link controls remain.
UEFI data/model object import/global-data audits run per configuration; additional
ARM64 LE/BE contract compile/import audits make no execution claim.

Five new behavioral suites: 11,520 independent framebuffer cases; 10,240 bitmap-
oracle map layouts including unaligned buffers/extension strides/unsorted inputs;
maximum 1024-descriptor/256-KiB map and eight-allocation ledger; literal type/
runtime/overlap/exhaustion/overflow ownership failures; 27 finite exit-outcome
traces; separate caller outputs/states and rejection stability. Release failures
are explicit, independent of NDEBUG. Physical addresses are never dereferenced.

The wrapper compares Clang/GCC preboot executable Debug twins and retains the
prior twenty-one core/hosted/link fingerprints, for twenty-three named comparisons.
The earlier eight standalone profiles remain current-core fixtures; they do not
include this new platform validator or establish its complete native link.

Initial development six-suite pass and first full matrix pass precede the final
maximum-map/eight-allocation boundary fixtures. The final fresh matrix is recorded
separately in [the snapshot](../../docs/development/uefi-contract-evidence.md),
with immutable code/source, hashes and remote results only after observation.

Final review corrected the excessive-allocation-count fixture to supply a truthful
nine-element array; validators and supported behavior are unchanged. A fresh full
matrix follows that fixture-only correction and is the pinned final local run.

Failures/limits outside conformance: VirtualBox help COM E_ACCESSDENIED; web HTML
403/PDF size/timeouts handled through publisher/local manuals. First reviewed
VBoxPkg manifest covers ARM, not x64; selected x64 firmware closure remains unknown.
Oracle's Bhyve license inventory includes Apple-attributed portions; this is an
eligibility concern, not a finding of Apple ROM code in the installed firmware.
No uncertain firmware was adopted, executed or inspected as implementation.

Human provenance review pending. No hardware keyboard/timer/graphics, UEFI ABI,
native entry/stack/exceptions, page access, ARM64 execution or historical claim.
B1 remains separately passed; all native/physical edition gates remain open.
