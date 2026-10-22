# Native PC window boot binding - actual verification

2026-10-22;SPEC-0062/ADR-0071;IMPL-0064/TEST-0073;SRC-0356..0358.

The opt-in -WindowDrawing profile installs native tables and enters the new
session directly,without old scene/timer/UART entry calls or service/A-line probes.
Its160-byte little-endian CWP1 record lives at trace offset2048. Full numeric
preflight,real EXITED/status0/readiness,map/owned/image coverage,resource windows,
PM timer/8042 and framebuffer gates precede the one arena plan and session start.
ACPI reads reject actual translations into mutable arena/trace/device storage.
No firmware calls after exit. Healthy operation has no60s or Escape deadline.

- 328 final Clang64 regression CTests +8 sanitized +8 GCC64 +4 GCC32 focused
  checks pass (348 profile CTests). UEFI code is x86-64 only;GCC32 checks the
  corrected neutral session and Python parser,not an unsupported UEFI ABI.
- Each64-bit profile:34 real binding scenarios/10154263 checks and39 fault-mode
  scenarios/10154319 checks. Fault mode repeats positive cases plus5 explicit
  dependency-result/prefix injections. Most assertions exercise repeated clock
  stall turns/padding;these are not millions of distinct cases.
- Real synthetic ACPI/map/controller startup transcript exercises unknown-down
  suppression,release,title drag,body click,4 recognized Space/Escape events,
  corrupt AUX cancellation,NONE progress clearing,and live operation beyond62
  seconds for metadata1/60/3600. UI count2 differs from legacy bus count3.
- Gate refusals preserve arena/framebuffer;raw refusals also preserve trace.
  Tests inspect actual receipt clearing before PM reads and stale progress after
  clock faults. Injected owner/presentation cases test wrapper diagnostics and
  retention;5 reported rows are synthetic,not5 newly executed presenter rows.
  Existing SPEC-0060/0061 tests retain real owner/partial-row coverage.
- The separate trace reader checks exact size/magic/version/reserved,all field
  offsets,signed/64-bit values,165 malformed inputs and an actual native RAM
  trace roundtrip. It does not guess keyboard/UART layouts or parse UART output.
- Eight Clang/GCC64 O0/O2 wrapper/entry import/global audits pass. Entry imports
  the window loop rather than old drawing/UART/interpreter probe calls. Native
  per-function frame limit2048 is enforced;ASan instrumentation is excluded from
  that native limit after its expected enlarged diagnostic stack failed it.
- Four compiled defective wrappers reject:missing readiness,stale receipt before
  clock,stale publication on clock failure,and a hidden metadata deadline. The
  unmodified identical fail-fast harness passes. Two invalid macro profiles and
  two conflicting switches reject,with no switch-refusal output directory.

Four EFI images are inspected unbooted;each O0/O2 repeat pair is byte identical.
27 PE corruption and26 omitted-object controls reject,including9 window modules.
Old native-UART/A-line entry export omissions are inapplicable only to this new
profile;their0 counters are explicit. Existing profile controls remain intact.
The original legacy units remain linked under the existing no-gc image policy;
entry isolation is not a claim that the entire binary contains no interpreter.
O2 EFI182272 bytes,loaded extent196608,SHA256
`3bd7d5a286f3fa5b3f0c4765d23a23e2de5c78ed1b924c67aceeaf680f8839a9`.
Two original Clang/GCC32 media writers agree on the raw disk;independent filesystem/
payload inspection passes. These local images use an explicitly dirty precommit
source receipt;later CI pins the final code revision. No image was executed.
Actual640-wide arena plan25488 bytes;maximum8192-wide plan55696 bytes. Config,
other kernel/trace/stack/image costs are separate,not whole128K certification.

One Claude Opus5.5 low primary initially returns a correct missing-type blocker
in15.52s (28258B535 lines,no code). Codex supplies the omitted public definitions
in one corrected compact-header packet39048B772 lines. It completes176.84s with
355 C lines. Zero tools,writes or tests observed in either call. JSON fence/extra
brace are extracted;lead integrates rather than recreates the code. Corrections:
return0 directly on forbidden ACPI reads,native arena alignment,bounded owned
image coverage,target validation before owned checks,exact8042 flag,clock category/
module and copyright punctuation. Existing interface assumptions are confirmed
against original APIs,not adopted as historical facts. Lead adds build/profile/
parser integration and tests initially drafted from the frozen API before seeing
new implementation,then extends concrete fault checks. Final tests are not a
strict author-blind oracle after lead production edits. No second worker or new
qualification. Reported list/session cost$0.7532672 is not an account balance or
measured Codex saving. Future briefs must include required type definitions on
the first call;files do not remove context limits. Grok/Copilot/Argon idle.

Correction to TEST-0072:its real-byte test used unrecognized Set1-style bytes,
so the earlier keyboard-drain coverage statement was overstated. The current
test uses documented project Set2 codes and asserts4 actual drains on64/32-bit
profiles. Old source-pinned test results remain historical;no old evidence is
silently rewritten. [Correction](../../provenance/records/TEST-0072-pc-window-session.md).

[Exact receipts/identities](pc-window-boot-evidence.json);
[worker assignment](tasks/pc-window-boot-worker.md).
Captures/builds:C:/ClasSICK/pc-window-boot-20261022-a,outside Git. Claude allowance
snapshot stays private;other remaining balances unavailable.

No VM/emulator/reference/hardware run,release/tag or published asset change.
24 originals,held brief,UART bodies,old rows/checkpoints and retired wiki remain.
Mac M1-M7,PC device/GUI/human/CPUID,source/role/historical/edition/physical/full128K/
B2/native-app/Linux/mini-vMac/both System1 directions remain. BIOS is not added.
Next:source-pinned candidate review and separately authorized exact-image trial;
Mac binding/source work remains necessary for paired-native-preview-2 parity.


Final range review removes an extra page-alignment restriction on the declared image extent;the existing loader ownership contract permits it within admitted owned pages. A regression accepts that case while retaining bounded image coverage. Final tests,candidate twins/media,object audits and faulty controls are rerun on this correction.


Source-pinned verification: `0b23981a83f5c1e0fdb231707d83c3d25c6085c8`;[Windows and68000 CI](https://github.com/segaboy/ClasSICK-0.97/actions/runs/39826971657) both succeed. Follow-up changes metadata only;all runtime/build inputs remain at this verified source.

Retained CI candidate verification: clean source and matching runtime input hashes; EFI182272B and disk67108864B are byte-identical to the locally inspected final candidate. Exact digests and unbooted classification are in the JSON receipt.
