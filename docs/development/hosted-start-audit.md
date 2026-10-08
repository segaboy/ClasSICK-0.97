# Hosted-start acceptance audit

Date 2026-10-08. **B1/M0.1 technical acceptance: passed.** This applies the existing
[boot requirement](../architecture/boot.md) and [roadmap](../roadmap.md); it does
not alter those requirements. IMPL-0008 / TEST-0010 reuse SPEC-0001–0005 and
TEST-0009. Self-review; human provenance review remains pending.

Final code/test/build source: `f3ce747913628a2fa4debc919704fc7a420e2687`.
Operator-session and first full matrix source:
`0405f2842db446f8ca249dcc9a0864119dd6a571`.
The only subsequent implementation change constructs the posted release flag
with an unsigned shift for 32-bit correctness. Interactive keyboard/presentation,
core, event-loop and cleanup paths are unchanged. The final matrix was rerun;
no second operator session or operator validation of the new binary is claimed.
Evidence/CI source: `cf8b84e50ca92b8130d9f24a9572a33a8df04031`.
[Windows CI 37726362180 passed](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37726362180):
observed guard, pinned setup, bootstrap twins, all 36 checks in each matrix
configuration, sanitizer/core/endian audits, six valid x64 debugger sessions and
three rejected launches. All eight named executable hashes match local/remote.
CI injected no live operator input; it does not replace the earlier owner session.
This subsequent result update changes documentation and runner completion-message
wording only; compiled inputs and verification logic are unchanged.

| Existing requirement | Evidence and result |
| --- | --- |
| Native Windows process initializes the core from supplied arenas | Combined start checks, exact 1,342,744-byte arena and prior exact/short-pool tests: pass |
| Independently authored scene through abstract surfaces | Original color/mono scenes and GDI pixel predicates at both scene states: pass |
| Normalized synthetic input | Direct synthetic event uses the same bounded queue/consumer: pass |
| Live input | Separate owner-operated visible session, two Space presses/releases and Escape, no harness-generated input: pass |
| Clean exit and ownership | Escape, destruction, timer removal, empty queue, arena retirement, class unregistration, pool release, exit zero: pass |
| Headless bounds/ownership/event contracts | Full 36-check matrix per five configurations, validated sanitizers: pass |
| No hardware imports in core | Optimized freestanding import/data and ARM64 LE/BE compile-only audits: pass |
| Debug workflow exercised | Six x64 source/state/step/resume sessions, three argument rejections and preceding interactive core record: pass |

Automated `--verify-start` posts key messages into the actual Windows queue and
uses the normal event loop. It checks synthetic/native ordering, repeat/release,
pixels, a real clock deadline, then Escape shutdown. This is stronger integration
coverage than the earlier separate SendMessage probes, but remains injected input.
`hosted.incomplete` confirms early Escape exits cleanly with acceptance rejected.

The distinct `--validate-start` session ran only after the owner requested opening
the viewer. The agent/harness generated no input. It observed two Space presses,
two releases and Escape, five consumed records, 409 timer wakeups, destroyed window,
zero arena usage after retirement and exit zero. That operator artifact matches
the first matrix's Debug twins at 0405f28, SHA-256
`ef792283eea1d4c2755da9c3108812943aefe0724302e3dff05369dbbbd5e1fa`.
Counters alone do not prove the source of an arbitrary future run. No formal
visual/DPI review or screenshot is claimed; independent pixel checks cover rendering.

Pinned LLVM-MinGW 20260908 / Clang/LLDB 23.1.1, CMake 4.4.4, Ninja 1.13.2 and
Windows PowerShell 5.1. Fresh Debug twins/Release/i686/x64 ASan+UBSan pass 36 checks
each, sanitizer controls and core/endian audits. Six valid x64 debugger sessions
and three rejection launches pass. Final viewer Debug twins match SHA-256
`2040e263ad68ff877b00625fc05f409b1321e083702e484bcc8ca21debbf7a92`.
Seven other executable hashes remain listed in
[the preceding debugger snapshot](debugger-evidence.md). WOW64 debugging still
fails; actual i686 conformance passes. One compiler; ARM64 compile-only.

## Repeat the check

Run the existing full matrix in a fresh build directory:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Verify-Debugger.ps1 -BuildRoot C:\ClasSICK\hosted-start-local -Sanitizers
```

For an operator check, run the Debug viewer with `--validate-start`, click it,
tap Space twice (color → monochrome → color), wait for the bottom strip to clear,
then press Escape. Save stdout/stderr and exit status with the executable hash.
Closing early or using the window Close button cannot pass that validation mode.
This check is interactive and is not run by CI. Normal no-option launch is unchanged.

Next: native freestanding linkage/runtime ownership and a second compiler core
build, then reviewed firmware/device contracts before B2. Real Macintosh and
mini vMac remain intended independent-firmware targets. Original hardware,
native PC and Linux PC boots remain not-started; parity and historical compatibility
remain untested. B1 is a Windows development start, not an OS boot.
