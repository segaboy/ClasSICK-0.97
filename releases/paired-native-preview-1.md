# Paired native drawing preview 1 - release target

2026-10-19 qualification checkpoint: both exact candidates boot and all logical pixels match at equal counts. Mac human check passes; PC human check failed on intermittent clicks.21/21 synthetic presses and Space pass; diagnostic stops consuming input after60 guest seconds. Release remains blocked. [Evidence](../docs/development/paired-preview-boot-trial.md); [clear test procedure](../docs/development/paired-preview-input-check.md).

Draft target2026-10-19; SRC-0291 / ADR-0055 / SPEC-0049. No publication,tag or
target trial authorized here. [Machine readiness](paired-native-preview-1.json).

Deliver the same original native drawing demonstration on Macintosh68000 and
x86-64 PC. Proposed next versions: ClasSICK ROM0.3.0-alpha.1 and ClasSICK PC
Preview0.1.0-alpha.1. These are planning targets; published/current component
versions remain governed by components.json. The System remains unreleased.

| Acceptance item | Mac path | PC path | Required evidence |
|---|---|---|---|
| Original native execution | own ROM / direct68000 C | own EFI / direct x86 C | native symbols/closure,compiler twins |
| Shared scene | mono512x342 | centered512x342 in available framebuffer | exact hosted palette-normalized pixels |
| Outline and clipping | SPEC-0049 ring/rails | identical shared workload | 456/384 pixels,840 union,real selected view |
| Completed workload | value97,six phases,two selections | same | actual counters/hash,negative-path checks |
| Counter redraw | native Mac button path | current native PC button/Space path | exact-byte approved trials,ordinary human input |
| Presentation progress | static0 | static0 in new outline mode | hosted steps0/9 and inputs0/2 parity |
| Distribution | ROM,source,instructions | EFI/disk,source,instructions | original-only contents,checksums,rebuild recipe |

Work queue,kept in release order:
1. Finish SPEC-0049 integration,independent tests,strict/sanitized/compiler audits
   and construction of both unbooted candidates; record exact source and hashes.
2. Prepare local DRAFT paired bundles and a short first-run guide. Check inventory,
   identities,source/license provision and reproducible construction. No binary in Git.
3. With fresh owner trial go-ahead,boot exact candidates in the named supported
   Snow/VirtualBox configurations and verify workload/counter/static pixels.
   With the owner available,verify ordinary input and record any mapping/capture
   limitations. Store reviewed own-product captures in the public screenshot index.
4. Resolve or explicitly accept the PC diagnostic session lifecycle for a preview;
   document how the session ends and how to restart. Continuous desktop operation,
   pointer motion,USB-only hardware and general hardware support are not promised.
5. Freeze notes,versions,source/image/firmware/configuration identities,packages
   and remaining limits. Request publication of the concrete reviewed pair.

Until3-5 are complete this is not release-ready,even if all compiler tests pass.
The paired previews may honestly demonstrate narrow native graphics without
claiming System1 compatibility,a usable desktop,full B2/128K/physical validation
or general Macintosh application support. Existing ROM/System cross-boot goals
and the Linux/complete-edition requirement remain separate. No Apple ROM/code,
reference media,manuals,private HQ or third-party emulator/compiler binary is bundled.

Future task rule: name this candidate and state the release criterion advanced.
Prioritize verified packaging/qualification fixes before new optional graphics
features. Mac/PC feature parity is mandatory; version-number equality is not.
