# TEST-0022 — Isolated native-PC trial

2026-10-08. SPEC-0015 / ADR-0019 / IMPL-0019; SRC-0060 owner trial approval.
Status: native VM runtime and operator Space observed; formal B2 partial.
The [snapshot](../../docs/development/vm-trial-evidence.md) and sanitized JSON
record the final sequence, limitations and separate input classes.

The unchanged pinned Windows Verify-BootMedia.ps1 -Sanitizers chain was run
in fresh root vm-media-verification-20261008-a. Local log/results are retained.
Clang x64 106/i686 82; scoped GCC x64 91/i686 67; nine writer builds reproduce
SPEC-0014 image hash 370b7d4e7fc4200b77c367e09afa0c4944f534c2623d7f5f68db159c1da46078.
Independent checker, 30 corruption controls and three tool refusals pass.
This full chain is packaging/hosted/unloaded evidence, not a VM boot.

Installed VBoxManage --version and existing VM inventory queries work in the
approved management session. Subcommand --help probes returned public usage
with nonzero exits (not successful management actions); their outputs are retained
under vm-trial-20261008-a. Earlier COM denial remains historical evidence.
The original preparation script parses successfully. Its actual preparation,
configuration acceptance, failures and launch observations are retained below.

Initial preparation vm-trial-20261008-a converted and round-trip-verified media
and created a project machine, then failed modifyvm: the PowerShell argument
builder combined each `--nicN` and `none` into one argument. Correcting the array
expression separates them. The failed machine/logs remain powered off with no
disk attached; no launch occurred. A fresh -b preparation verifies the corrected
script rather than overwriting the failed run. Compiled/native code is unchanged.

The -b configuration succeeds. After fresh variable-store initialization,
secureboot --disable refuses because no platform key is enrolled; public
showvminfo nevertheless explicitly reports SecureBoot="off". No key is enrolled
and no NVRAM contents are read. SPEC-0015 now admits this exact refusal only
with the explicit public off readback; arbitrary failures remain fatal. The
original refusal is retained, and preparation resumes through recorded commands
on that same newly owned, powered-off -b UUID. It does not create a third VM or
overwrite any earlier command output.

46 required public settings were accepted before the -b launch; inventory checks
admitted only the new project identity. Raw/VDI/RAW round-trip hashes agree.
The complete fresh Windows verification passes 846 CTest checks across nine
configurations, with the existing sanitizer/link/debugger/unloaded audits;
64 labels/58 distinct fingerprints, all 59 prior UART labels unchanged.
The retained full log SHA-256 is
`4fa41e9fe777f3a700bf4e5492e1a4e27e4716c795114e586a77d22049ce321f`.

cold-01 through cold-06 finish with guest sec=0000003C (60) and
result=00000000 kbd=00000001. cold-01's one Space was injected. cold-03's
visible operator attempt received no input; the owner could not see its window.
cold-04 was brought forward through the installed Computer Use interface; no
generated key input was used. Own records count two Space presses, and the owner
confirmed “I tapped Space twice and saw the scene change.” That run completed
normally. Original scenes and changed progress bars were inspected locally.

The first cold-01 collector aborted on a 0x0-display screenshot refusal; a later
own-scene retry succeeded and the full guest transcript survived. Initial
cold-02/03/04 captures also refused before the display existed; later captures
succeeded. These observer failures are retained, not erased or guest passes.
The first cold-04 serial reconfiguration was refused while the prior GUI session
was unlocking; it succeeded after powered-off state was confirmed.

cold-05/06 retain line-arrival observations nominally every 100 ms, with actual
poll brackets. Readiness-to-result differences are 59.972246 / 59.971268 host
seconds, consistent with approximately 60 seconds including observation/serial
latency. Earlier 120-second collectors retain original-scene initial/intermediate/
final captures. No exact clock calibration or physical baud measurement is claimed.

The cold-06 observer accidentally used the reserved PowerShell Input variable,
so its intended Escape selection did not inject anything. The unchanged no-input
run and faulty observer are retained. The corrected TrialInputMode harness and
fresh cold-07 separately check synthetic Escape's expected early termination.
cold-07 reports result=0000000C kbd=00000001 after 5.312718 observed host
seconds from readiness; its own final scene was captured and inspected.
This synthetic path is not a second operator test or a duration pass.

Successful preparation commands with no output originally created no .txt file
through the output pipeline; their argv/zero exit and later configuration readback
remain recorded. The manifest marks those output hashes null. The published helper
now explicitly writes empty output records. It is syntax-reviewed; the final
missing-PK/empty-output revision has not been run end-to-end on a third machine.
The actual -b trial used the retained reviewed resumption, not an invented replay.
After all trials the project VM is powered off and the VDI hash still equals
its admission hash. The failed -a machine remains powered off/unattached.

Own serial output is gated by the reviewed successful-exit/owned-ready/device
contract. A post-scene cold-01 register query showed RIP within our loaded image;
no memory/code dump was taken. Explicit A20/machine-state self-test qualification,
firmware image fingerprint/binding and actual exception delivery remain unverified.
No firmware internals or existing owner VM configuration is a debugging input.
Formal B2 remains partial; all edition flags and Mac goals persist.
