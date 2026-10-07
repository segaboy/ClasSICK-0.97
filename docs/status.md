# Project status

Date: 2026-10-07. Current phase: **SPEC-0001 headless subsystem verified; M0.1 partial**.
M0.0 technical bootstrap remains verified. The owner explicitly authorized the
first bounded graphics subsystem in the project implementation chat on this date.
SPEC-0001 v1 is finalized, implemented and tested under IMPL-0002 / TEST-0004.
GPL-3.0-or-later remains approved. No OS boot or historical compatibility verified.

The owner's broader project bootstrap continues in a separate headquarters
documentation/evidence repository and personal project wiki. This technical gate
does not imply that the complete project foundation or any OS boot is finished.

## Achieved

- First subsystem: caller-owned MSB-first 1bpp/RGBA8 surfaces, clear and clipped
  half-open rectangle fill; stable errors, checked sizes and padding preservation.
- Six CTest checks pass in two fresh x64 debug builds, x64 release, x86 debug,
  and validated x64 ASan/UBSan. The matrix covers 8,424 whole-buffer cases per run.
- ARM64 little/big-endian ELF compile/import checks pass, with no execution claim.
  Optimized core objects have no undefined symbols/global data; native debug
  archive/header dependencies were also inspected. Negative audit probes reject
  an external call and mutable global. Strict warnings are errors.
- Fresh x64 test executable hashes match. Core static archive hashes differ due
  to COFF object timestamps; archive/image reproducibility is not established.
- [Surface test/provenance results](../provenance/records/TEST-0004-surfaces.md)
  distinguish our contract from historical behavior and boot completion.
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
- Interactive debugger, other sanitizer targets, m68k toolchain/runtime and boot.
- Next: separately specify Windows presentation; then arenas/events/clock and the
  remaining B1/M0.1 checks. No subsequent subsystem is implemented by this task.
- Three bootable editions remain separately not-started: original Macintosh,
  native PC without Linux, Linux PC. Historical identity/QuickDraw and edition
  parity remain unverified; no Linux divergence has been introduced.

See [evidence](development/bootstrap-evidence.md) for exact verified claims and
[compatibility status](compatibility/README.md) for the presently unimplemented APIs.
