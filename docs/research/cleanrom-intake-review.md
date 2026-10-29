# CLEANROM eligibility review — 2026-10-29

**Disposition: held implementation; sanitized research intake recorded.**
SRC-0534/0535. This is an eligibility and delivery review,not acceptance of a
working ROM or a new ClasSICK release. The supplied implementation,ROM,assets
and test hardware model remain outside Git and outside worker inputs.

## Material and checks

The owner supplied a ROM,source archive and nine-page build report attributed
to Claude. [Identity manifest](cleanrom-intake-manifest.json) records all three
SHA-256 identities. Byte-identical intake copies remain private.

| Supplied item | Bytes | SHA-256 |
| --- | ---: | --- |
| CLEANROM.ROM | 65536 | 214fc980068a1b14e76d363b90d1681097edca258ca26bd7c28e694ccd01a970 |
| cleanrom-src.tar.gz | 20247 | 06e5c2445b0e2bf0ba4283090c672167b1b70987f5e991b45e586060b28a9e65 |
| CLEANROM-build-notes.pdf | 439196 | 0204d09c02a0d67371a86bab05bda8574b4a816d6f12d592b821e731ba9488c2 |

Executed checks:copy identity,archive member/path/type/size review,PDF metadata,
and SHA-256 equality of the archive's ROM with the separately supplied ROM.
The archive has seven regular files and three directory entries,127874 bytes
uncompressed;no links,traversal,absolute paths or duplicate member names were
found. These checks establish delivery consistency,not code origin or correctness.

Semantic inspection was limited to the README and build-report narrative on
pages1-2. Source bodies,ROM/header,asset generator,build script and hardware-model
implementation were not opened. The remaining seven PDF pages were not reviewed
after the source-history finding. No assembly,link,rebuild,execution,boot,
disassembly,worker call or reference-system experiment was performed.

## Eligibility finding

The report's page2 explicitly declares reading MAME and Mini vMac implementation
source for hardware models and emulator behavior. Its claim that no Apple ROM,
disassembly or Apple assets were used does not satisfy this project's separate
emulator-source exclusion. The README also describes a Musashi-dependent test
hardware model and deliberate accommodation of emulator ROM patching.

[AGENTS](../../AGENTS.md) excludes emulator source and its derivatives as
implementation inputs. The [clean-room policy](../clean-room/POLICY.md) requires
affected work to stop and prohibits reuse or redistribution pending owner
disposition. Review stopped before inspecting implementation files. The owner
was notified privately;the detailed exposure boundary is retained outside Git.

This finding does **not** establish that Apple code is present. It establishes
that the declared development inputs conflict with this project's rules. The
report itself is also excluded as a source of new hardware/implementation facts.
Only sanitized identity,eligibility and explicitly attributed claim metadata are
recorded here. Nothing from its hardware table is promoted into a ClasSICK spec.

No ClasSICK implementation path was changed or authored from this intake. Existing
independently sourced contracts and their dated evidence retain their own status.
The hold applies to this package and further affected review/reuse;it grants no
permission to restart an affected implementation lane.

## Claims and verification gaps

| Reported deliverable | What is established | Consequence for ClasSICK |
| --- | --- | --- |
| 64KB Macintosh128K/512K replacement ROM | Exact supplied size/hash;its archive copy matches | Boot and model compatibility are unverified |
| 20670 bytes used;reported checksum | Author's report only;no source,link map or header audit | No equivalent-work size or performance comparison |
| Quadrature mouse,arrow cursor and window dragging | Described in README/report | No demonstrated fix for current mouse/drag blockers |
| Original hand-drawn font/cursor | Author's statement only;asset bodies not reviewed | Separate authorship,derivation and rights review needed |
| Build and hardware-model tests | Report p1 says no boot and the harness could not run | Build claims are unreproduced;harness is not passing-test evidence |
| A standalone68000 assembly implementation | Declared package scope;no portable-C/native-PC deliverable described | Does not establish native-PC parity or an integration contract |

The archive has no separate LICENSE member. This does not prove that its source
headers lack notices,or negate the project's inbound contribution terms;those
headers and the contribution's complete rights/dependency history remain unreviewed.
No conclusion about instruction correctness,hardware initialization,asset originality
or overall code quality is made from unopened implementation files.

## Useful continuation

1. **Obtain an auditable source history.** Use the complete
   [provenance-only prompt](../development/tasks/cleanrom-provenance-followup.md)
   in the original Claude conversation. A new worker cannot recover that chat's
   history or certify the original author's independence by guessing.
2. **Retain the current independent Mac input path.** SPEC-0072,0074,0075,0077,
   0078,0080 and0081 and their pre-intake evidence already describe the controller,
   capture/loss rules and timing experiments. Further authorized work should start
   from those reviewed contracts and eligible published sources,with the required
   unexposed implementer where the role policy applies. No CLEANROM code,hardware
   table,emulator-dependent adaptation or inferred algorithm becomes an input.
3. **Consider independent controller-level fixtures.** As a future bounded task,
   specify SCC latch/reset/interrupt behavior and VIA phase-event schedules from
   admitted chip documentation,then exercise the existing controller callbacks.
   Keep model authorship separate from the driver and test races/loss explicitly.
   This can help investigate capture failures without requiring a new CPU core.
   It remains a proposed test contract,not implementation or target qualification.
4. **Review separable assets only after provenance is resolved.** Independently
   originated contributions may be considered file by file once rights and source
   boundaries are demonstrated. No asset from this intake is currently admitted.
5. **Keep native parity explicit.** Any later feature acceptance needs reviewed
   shared behavior,original native68000/native-x86 implementations and actual
   target evidence. An assembly prototype is not a portable subsystem submission.

These actions preserve existing project goals;they do not rehabilitate excluded
inputs by paraphrasing,translation or a clean-room label. The owner decides the
source-exposure disposition. No new runtime,emulator,VM,hardware,release or
publication scope is granted. Current paired-release mouse/drag gates remain open.

## Intake record

The three supplied artifacts remain private and unchanged. Only this review,
its identity manifest,the provenance question prompt and normal source/HQ records
are intended for ordinary Git publication. No original attachments,implementation,
ROM,generated assets,manual pages or report images are published.
