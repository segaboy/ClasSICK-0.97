# Project status

Date: 2026-10-08. Current phase: **B1/M0.1 hosted start verified; B2 components in progress, native boots pending**.
M0.0 technical bootstrap remains verified. The owner explicitly authorized the
first bounded graphics subsystem in the project implementation chat on this date.
SPEC-0001 v1 is finalized, implemented and tested under IMPL-0002 / TEST-0004.
The owner authorized the next visible checkpoint. SPEC-0002 v1 adds a native
Windows original-scene viewer under IMPL-0003 / TEST-0005.
The owner reaffirmed real Macintosh/mini vMac booting and instructed continuation.
SPEC-0003 v1 adds bounded core arenas and viewer buffers under IMPL-0004 / TEST-0006.
GPL-3.0-or-later remains approved. No OS boot or historical compatibility verified.

The owner's broader project bootstrap continues in a separate headquarters
documentation/evidence repository and personal project wiki. This technical gate
does not imply that the complete project foundation or any OS boot is finished.

## Achieved

- SPEC-0012 / ADR-0016 / IMPL-0016 / TEST-0018 implements an original bounded
  polling PS/2 set-2 keyboard and interactive successor loop. ACPI must declare
  a controller; startup negotiates untranslated set 2 with bounded replies and
  retries. Initial Space changes the scene; Escape ends early. Hosted verification
  and unloaded EFI inspection are recorded in the
  [keyboard snapshot](development/ps2-keyboard-evidence.md). Codex now leads
  implementation (SRC-0048); no native keyboard or OS boot has been observed.
  UART, boot media, firmware eligibility and B2 remain open.

- SPEC-0011 / ADR-0015 / IMPL-0015 / TEST-0017 adds a cooperative loop that fills
  the scene's progress bar from PM timer seconds for 60 seconds, with stall and
  late-sample diagnostics in owned trace storage. Pinned
  [Windows CI 37812572045](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37812572045)
  passes: Clang x64 81/i686 65, GCC x64 66/i686 50.
  [Snapshot](development/progress-loop-evidence.md). Never executed natively.

- SPEC-0010 / ADR-0014 / IMPL-0014 / TEST-0016 captures the ACPI 2.0+ RSDP before
  exit and walks RSDP/XSDT/FADT through a map-type-checked reader. It extends the
  24/32-bit PM timer and runs a gated native probe recorded in owned trace storage.
  Pinned [Windows CI 37810609408](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37810609408)
  passes at 222a837 (one earlier test-only LLP64 failure retained): Clang x64 78/
  i686 65, GCC x64 63/i686 50. [Snapshot](development/pm-timer-evidence.md). No
  port has been read natively; the 60-second loop and B2 remain open.

- SPEC-0009 / ADR-0013 / IMPL-0013 / TEST-0015 adds a gated native framebuffer
  presenter (RGB/BGR-reserved, zero reserved byte, clipping, no framebuffer reads),
  an original band-rendered boot scene with at most 64 KiB of arena staging, and a
  post-handoff gate that requires a successful exit and ready descriptors. Pinned
  [Windows CI 37806764756](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37806764756)
  passes at c6289e1: Clang x64 66 checks/i686 57, GCC x64 51/i686 42. The EFI
  image links the new objects and three omitted-object links reject; 27 earlier
  fingerprints are unchanged. [Snapshot](development/native-framebuffer-evidence.md).
  Claude led this checkpoint (SRC-0044). Native visible output and B2 remain
  unverified.

- SPEC-0008 / ADR-0012 / IMPL-0012 / TEST-0014 adds original x64 GDT/TSS/IDT
  serialization, four emergency stacks and terminal first-fault record assembly.
  Complete local/remote Clang x64 53-check four-config/i686 48, scoped GCC x64
  38-check three-config/i686 33 matrices and prior controls pass. Four unloaded
  own EFI images/22 symbols/256 vectors, twenty corruptions and two omitted-object
  controls pass; all 29 named local/remote fingerprints match.
  [Immutable evidence](development/x64-exception-evidence.md) pins source 974d781
  and passed Windows CI 37795563016. No actual descriptor/fault delivery, devices,
  firmware adoption, VM, B2 or edition boot is verified.

- SPEC-0007 / ADR-0011 / IMPL-0011 / TEST-0013 implements original x64 UEFI
  orchestration and an owned-stack terminal scaffold. Local Clang x64 48-check
  four-config/i686 43-check and GCC x64 33-check three-config/i686 28-check
  matrices pass with existing debugger/sanitizer/link/endian controls. Four
  unloaded EFI images form matching O0/O2 twins; twelve corruption controls and
  omitted-transition link reject. Twenty-seven named comparisons match locally.
  [Loader evidence](development/uefi-loader-evidence.md) pins immutable source
  c912f2f compiled inputs/97b1049 script and final b fingerprints. Corrected
  [Windows CI 37790529852 passes](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37790529852)
  at 1acd285, full protocol/all 27 hashes matching after expected-control status fix.
  [Firmware follow-up](development/firmware-eligibility.md)
  identifies the actual x64 producer but leaves binary/transitive provenance
  needs-review. No VM, native handoff, device loop, B2 or edition boot is verified.

- SPEC-0006 / ADR-0010 / IMPL-0010 / TEST-0012 adds original platform validators
  for bounded framebuffer/map data, reserved allocation consistency and a finite
  exit-outcome model. Five behavioral suites and the object audit pass locally.
  The full current matrix passes 43 checks per five Clang configurations and
  28 per four scoped GCC configurations, with existing core/link/debugger checks
  and ARM64 LE/BE preboot-object compile audits. No firmware callback or VM launch.
  [Preboot evidence](development/uefi-contract-evidence.md) pins final results;
  [the proposed PC profile](development/native-pc-profile.md) records ownership
  and device obligations. Exact selected x64 firmware closure/eligibility remains
  needs-review; metadata/license inventory is not firmware adoption.
  [Windows CI 37780598062 passed](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37780598062)
  at 8b09ef3, with the full wrapper and twenty-three named local/remote hashes matching.

- ADR-0009 / IMPL-0009 / TEST-0011 verifies complete current-core linkage under
  Clang/LLD and GCC/GNU ld at x86-64/i686 O0/O2. Eight profile twins match;
  eighteen original functions are retained with the exact named entry and no
  standard startup/runtime library or imports. Four unresolved-helper and five
  image-rejection controls pass. Images were never loaded.
- The full local Clang matrix passes 37 checks per five configurations, existing
  sanitizer/core/endian audits and x64 debugger replay. GCC core-only passes 22
  checks per Debug twin/Release/actual i686; five hosted executable twins match.
  Fresh cached second-compiler setup also passes without system changes.
  [Scope and repeat instructions](development/core-link-evidence.md).
  [Windows CI 37774811569 passed](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37774811569)
  at 3950f33 with the full wrapper and twenty-one matching local/remote hashes;
  native execution and future runtime helpers
  remain separate. The broader GCC Windows viewer build is unverified.

- TEST-0010 / IMPL-0008 completes the existing B1/M0.1 technical acceptance audit.
  Combined synthetic/queued-message startup, timed pixels and Escape shutdown pass;
  early Escape correctly reports incomplete acceptance after clean retirement.
- The owner-operated visible session passes: two Space presses/releases and
  Escape, five consumed records, 409 timer wakeups, destroyed window, zero retired
  arena usage and exit zero. No test-generated input in that session. This is
  scoped keyboard evidence; formal visual/DPI and human provenance review remain separate.
- All 36 CTest checks pass in five fresh configurations, including actual i686
  and x64 ASan/UBSan. Core/endian audits and all six x64 debugger sessions plus
  three invalid launches pass. The operator executable matches the first matrix's
  Debug twins; final fixture-only unsigned-shift correction was followed by a
  second full matrix. Interactive/core/cleanup paths are identical between them.
  [Hosted-start audit](development/hosted-start-audit.md) records scope and results.
  [Windows CI 37726362180 passed](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37726362180)
  at cf8b84e, including the full matrix/replay; eight local/remote hashes match.

- TEST-0009 / IMPL-0007 verifies the bundled LLDB 23.1.1 on x64 Debug twins:
  source breakpoints, typed core/queue/viewer state, stacks, step over/in/out and
  clean resume. A local interactive core session also passed.
- Viewer launch now uses parsed host-runtime arguments. LLDB's observed trailing
  delimiter is accepted; unknown, extra and empty arguments reject with exit 2.
- Six valid x64 debugger sessions and three rejection launches pass. The full
  local five-configuration 34-check matrix, sanitizers and core/endian audits pass;
  viewer hash matches fresh twins and seven other executable hashes are unchanged.
  [Debugger Windows CI passed](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37724597975)
  at 469f307 with the full matrix/replay and eight local/remote hashes matching.
  [Debugger snapshot](development/debugger-evidence.md) pins code/evidence inputs.
- Optional i686 debugging failed at WOW64 exception 0x4000001f, with unavailable
  original frame state. Actual i686 conformance still passes; no i686 debugger
  success, general debugger certification or new tool adoption is claimed.

- SPEC-0005 v1 canonical seconds/nanoseconds, overflow/backward rejection,
  monotonic observations and deterministic advancement, IMPL-0006 / TEST-0008.
  Host QPC conversion/availability/raw-monotonic checks stay outside the core.
- 34 CTest checks pass per fresh x64 Debug twin, Release, actual i686 and validated
  x64 ASan/UBSan configuration. Independent clock oracle: 5,184 boundary pairs;
  thirteen conversion fixtures, injectable faults and 1,000 live QPC reads.
- Fake/live hidden viewer tests verify deadline strip pixels, repeat behavior,
  delayed wakeup, real Windows timer dispatch and destruction. Eight executable
  hashes match fresh twins; all prior core/adapter tests remain verified.
- Clock core import/data audits and ARM64 LE/BE compile-only checks pass.
  [Clock Windows CI passed](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37722447834)
  at f814fd4: full matrix and all eight local/remote hashes match.
  [Clock snapshot](development/clock-evidence.md) pins implementation/evidence inputs.
- Owner's VirtualBox 7.2.16r174877 is available for future native-PC boot tests;
  read-only version query only, no VM or firmware configured/adopted/startup verified.

- SPEC-0004 v1 bounded keyboard FIFO and Windows mapping, IMPL-0005 / TEST-0007.
  Synthetic/native-message Space/Escape share one viewer queue and consumer;
  explicit overflow/repeat/release/reset/sequence-exhaustion contracts.
- 24 CTest checks per configuration pass in fresh x64 Debug twins, Release,
  actual i686 and validated x64 ASan/UBSan. Independent list oracle: 327,680
  eight-operation traces; 120 Windows mapping cases and actual hidden viewer tests.
- Input records share the arena pool; exact input reservation and one-byte-short
  storage failure pass. Core import/data and ARM64 LE/BE compile-only audits pass.
- [Input results](../provenance/records/TEST-0007-input.md) and
  [input snapshot](development/input-evidence.md).
- [Input Windows CI passed](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37719495759)
  at e237cbb; complete 24-check matrix and all six local/remote hashes match.

- Bounded caller-owned native arenas: alignment, checked arithmetic, stable errors,
  exhaustion and all-span reset without heap calls or backing writes in the core.
- Viewer color/mono/scratch buffers now share one host-owned pool reserved by the
  core arena. Exact layout and a one-byte-short pool failure are checked.
- Sixteen CTest checks pass in fresh x64 Debug twins, Release, actual i686 and
  validated x64 ASan/UBSan. Arena matrix: 751,740 whole-buffer cases per execution.
  ARM64 little/big-endian arena compile/import checks pass; no execution claim.
- Fresh arena-test and updated viewer executable hashes match; unchanged surface
  and presentation-test hashes still match the preceding verified milestone.
- [Arena test/provenance results](../provenance/records/TEST-0006-arenas.md) and
  [immutable arena evidence](development/arena-evidence.md).
- [Arena Windows CI passed](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37699177939)
  at evidence source 09907fb; complete matrix and all four local/remote hashes match.

- Windows color/mono viewer: explicit BGRX conversion, integer scaling/crop,
  clipping and resize/DPI handling. A visible preview was launched and inspected
  through an original-scene capture; owner's manual acceptance is separate.
- Ten CTest checks pass in fresh x64 Debug twins, Release, i686 and validated
  x64 ASan/UBSan. Four added suites cover conversion/bounds, GDI pixels and hidden
  window lifecycle. Fresh viewer/test executable hashes match. Core unchanged.
- [Presentation results](../provenance/records/TEST-0005-windows-presentation.md).
- [Immutable presentation snapshot](development/presentation-evidence.md) records
  executed source and qualified executable hashes.
- [Presentation Windows CI passed](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37691938711)
  at evidence source 5c166dd, with all ten checks and matching local viewer/test hashes.

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
- [Immutable surface build snapshot](development/surface-evidence.md) pins the
  implementation revision and qualified local artifact hashes.
- [Corrected Windows CI passed](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37685301136)
  at source `aeb7b1d57182a2bc254bbcfe1c34cb3c390abad4`, including the complete
  sanitizer matrix. The preceding UBSan probe validation failure and native
  diagnostic-capture correction are retained in the evidence record.
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
- WOW64 debugger support, other sanitizer targets, m68k toolchain/runtime and boot.
- Next: polling UART diagnostics,
  reviewed boot media and exact firmware eligibility. Presenter, PM timer and
  loop and two-key keyboard are hosted-verified only (SPEC-0009–0012).
  Loader/entry/stack/exception scaffolds are built and inspected; real handoff,
  descriptor installation/fault injection and B2 remain unverified. B1 stays a
  Windows development start.
- Three bootable editions remain separately not-started: original Macintosh,
  native PC without Linux, Linux PC. Historical identity/QuickDraw and edition
  parity remain unverified; no Linux divergence has been introduced.
- mini vMac is an owner-requested Mac-image validation path, with our replacement
  firmware. Exact emulator/configuration/startup support still requires review;
  emulator testing does not replace the required original-hardware result.

See [evidence](development/bootstrap-evidence.md) for exact verified claims and
[compatibility status](compatibility/README.md) for the presently unimplemented APIs.
