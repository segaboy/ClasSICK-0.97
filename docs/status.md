# Project status

Date: 2026-10-07. Current phase: **M0.0 bootstrap complete**.
GPL-3.0-or-later approved; local and remote verification passed. No OS subsystem implemented.

## Achieved

- Public repository established; slug `ClasSICK-0.97`, human name ClasSICK 0.97.
- Founding clean-room policy and contribution/publication safeguards committed
  first, before implementation work.
- Charter, native-portability architecture/HAL/personality/runtime strategy,
  measurable boot gates, roadmap, research backlog and eight initial ADRs authored.
- Sanitized workstation inventory and minimal Windows toolchain evaluation.
- Pinned portable tool setup exercised using built-in Windows PowerShell.
- GPL-3.0-or-later approved by the owner for project-owned code, docs and assets;
  standard license text and matching inbound terms added after approval.
- Two fresh native hosted development probes passed CTest with identical hashes.
- Fresh local-clone/offline-setup replay passed, producing two further matching
  native probe builds. Source/index/links/JSON/payload guard passed, including a
  simulated forbidden-extension rejection check.
- Licensed-source builds passed both outside and inside the source checkout with
  matching hashes after correcting debug prefix-map precedence.
- [Windows CI passed](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37670548073)
  for source revision `95adfa72b0fd4d0890a9fab73712e5bbb5254b0e`: pinned setup,
  80-file repository audit, two fresh native builds, CTest and matching hashes.

## Pending decisions and future work

- External code/assets and releases still require provenance and third-party
  license review under the approved project license.
- Exact historical profile and 1984-applicable behavioral sources.
- Macintosh 128K native feasibility, hardware startup and size budgets.
- Debugger/sanitizer workflow, cross compilers, alternate native targets and boot.
- First OS subsystem: SPEC-0001 bounded graphics surfaces, after bootstrap review.

See [evidence](development/bootstrap-evidence.md) for exact verified claims and
[compatibility status](compatibility/README.md) for the presently unimplemented APIs.
