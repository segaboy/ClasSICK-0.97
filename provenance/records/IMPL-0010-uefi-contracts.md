# IMPL-0010 — Original UEFI preboot validators and exit model

Date 2026-10-08. Owner SRC-0033, public interface SRC-0034, review metadata
SRC-0035, proposed timer source SRC-0036; SPEC-0006 / ADR-0010 / TEST-0012.
Codex GPT-6 AI-assisted original C/PowerShell/CMake and documentation; self-review,
human provenance review pending. Existing portable core algorithms are unchanged.

Paths: platform/uefi/contract.h and contract.c, tests/uefi-contract.c,
scripts/Verify-UEFIContracts.ps1, CMake/workflow, specification/profile/provenance.
Physical values stay uint64_t, maps are explicit little-endian byte reads without
UEFI headers or native struct casts. Original bounded pairwise validation and
owned-span containment use constant scratch, no heap/callbacks/global state.
The exit model records only caller observations and never invokes firmware.

Original independent fixtures use literal fields, byte encoders, bitmap occupancy
oracle, output sentinels, separate instances, maximum map/allocation bounds and
finite firmware-outcome traces. Hosted stdio/malloc/runtime belong to test programs
only. No external implementation, loader library, runtime helper, protected asset
or Apple OS/ROM code used. Project work GPL-3.0-or-later.

UEFI prose/declarations and adjacent examples were visible; examples were neither
copied nor adopted. The restricted two-pixel-format profile does not implement the
manual's bitmask example. Oracle license/module/build metadata informed eligibility
only; no firmware implementation source or binary was fetched/inspected. Source
search snippets unrelated to primary documentation were discarded as inputs.

Tests prove generic platform data/model behavior, not actual UEFI calls, PE entry,
firmware honesty, page permissions, kernel startup or B2. Firmware eligibility and
register-level drivers remain separate. Immutable results are pinned in TEST-0012.
