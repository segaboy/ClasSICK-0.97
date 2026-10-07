# Project status

Date: 2026-10-07. Current phase: **M0.0 technical bootstrap complete**;
license decision and GitHub workflow permission pending. No OS subsystem implemented.

## Achieved

- Public repository established; slug `ClasSICK-0.97`, human name ClasSICK 0.97.
- Founding clean-room policy and contribution/publication safeguards committed
  first, before implementation work.
- Charter, native-portability architecture/HAL/personality/runtime strategy,
  measurable boot gates, roadmap, research backlog and eight initial ADRs authored.
- Sanitized workstation inventory and minimal Windows toolchain evaluation.
- Pinned portable tool setup exercised using built-in Windows PowerShell.
- Two fresh native hosted development probes passed CTest with identical hashes.
- Fresh local-clone/offline-setup replay passed, producing two further matching
  native probe builds. Source/index/links/JSON/payload guard passed, including a
  simulated forbidden-extension rejection check.

## Pending decisions and future work

- Owner's license choice; external code/asset acceptance and releases remain gated.
- Reviewed Windows CI workflow awaits GitHub `workflow` authorization. The CLI
  rejected publishing that file with its current scope; no remote CI pass claimed.
- Exact historical profile and 1984-applicable behavioral sources.
- Macintosh 128K native feasibility, hardware startup and size budgets.
- Debugger/sanitizer workflow, cross compilers, alternate native targets and boot.
- First OS subsystem: SPEC-0001 bounded graphics surfaces, after bootstrap review.

See [evidence](development/bootstrap-evidence.md) for exact verified claims and
[compatibility status](compatibility/README.md) for the presently unimplemented APIs.
