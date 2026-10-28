# ClasSICK 0.97 working instructions

Compacted 2026-10-28 from public baseline `11ad7a937e58f752cfecda8b6805c76bc7f45ff7`. Earlier Git instructions are preserved in the [byte-identical snapshot](AGENTS-history-2026-10-28.md). Archived dated entries retain their scoped authority, not a claim of current progress or a blanket new grant. Paths in the archive are relative to the original base. Later explicit owner amendments take precedence. Any other unsuperseded durable rule in the archive still applies. Consult the archive only for a specific historical scope. Do not reread it routinely.

## Read first (once, and again when changed, before affected work)

- `docs/clean-room/POLICY.md`, `CONTRIBUTING.md`, `provenance/README.md`.
- [Orchestration](docs/development/agent-orchestration.md) and [recordkeeping](docs/development/recordkeeping.md).
- When relevant: [native parity](docs/development/native-parity-plan.md), [display](docs/development/display-target.md), [performance](docs/development/native-performance-plan.md), [x64 PC](docs/development/unified-x64-pc-plan.md), [286](docs/development/pc286-feasibility.md), [System1/ROM](docs/development/system1-rom-boot-requirements.md), [sources](docs/research/system1-sources.md), [releases](releases/README.md), [milestones](docs/roadmap.md).

## Current scope
Current owner scope (2026-10-28, SRC-0521): resume paired release work and retained approved isolated original-candidate trials. Use windowed Display1 and readiness handshake. Release/tag publication,reference and hardware runs remain separate. Paired release work is PRIMARY; bounded286 work remains separate. [Current diagnostic receipt](docs/development/release-resumption-evidence.json); [owned input](docs/development/owned-window-input-evidence.md). Earlier scopes retain their exact applicability.


## Identity and license

- Keep the exact name **ClasSICK 0.97**.
- Project code, documentation and assets are GPL-3.0-or-later (owner, 2026-10-07). Contributions are inbound=outbound. Changing either needs explicit owner approval. Third-party inputs need their own license and provenance review.

## Clean room and sources

- The goal is behavioral compatibility. Apple's implementation is never a source.
- Never fetch, inspect, copy, disassemble, decompile, translate or prompt with prohibited implementation inputs: Apple or Microsoft implementation code, Apple ROM/media code, leaked source, disassembly, emulator source, firmware internals, copied protected assets or their derivatives. No manual sample code is an implementation input. Separately admitted lawful black-box reference use remains subject to POLICY; it grants no source inspection, worker transfer or redistribution.
- Use published editions only (SRC-0067). The 1984-85 CONFIDENTIAL pre-release drafts are excluded (SRC-0070). Facts that came from them must be re-sourced. The excluded cached summary stays excluded. The owner's Inside Macintosh PDF stays outside Git.
- A source with uncertain provenance is ineligible until reviewed. Do not adopt firmware based on product or license names. Record source IDs plus edition, page and section.
- On uncertain/prohibited exposure, stop the affected lane, exclude the material and record/notify privately as required by POLICY; owner disposition controls restart. Reviewed interface-fact paraphrases may go to fresh workers within SRC-0402..0411; manual pages remain lead-only.
- Microsoft FAT32 terms are accepted for PC boot tooling only, outside the System 0.97 clean-room scope.
- Admit exact platform/source/build before any trial. Existing Snow and VirtualBox clearance is black-box only (SRC-0059/0076); no emulator fork/source or firmware extraction. Mini vMac availability is not clearance; QEMU, OVMF and MAME remain uncleared. Independent distributions do not require Apple ROMs; reference-only interoperability stays separately gated.
- Keep historical facts, observations, decisions and implementations distinct. A later manual does not prove 1984 behavior.
- When another tool needs research, give it a complete prompt that can be copied as-is (SRC-0131).

## Engineering

- Core code is freestanding, architecture-neutral C. Keep guest addresses separate from native pointers. Hardware code belongs in platform or architecture layers.
- Implement from approved specifications. Every compatibility change needs provenance, tests and an explicit status.
- Native performance is required on Mac128K/68000, 286 and x64 UEFI (ADR-0076): shared semantics, measured fast paths, and no avoidable cost accepted only for portability. Size budgets are deferred, not dropped.
- Native parity (SRC-0217/0218): advance matching capabilities on native68000 Mac and native x86 PC OS/managers; interpret legacy68k applications. This supersedes the earlier whole-Mac-completion prerequisite for native PC counterparts/original parity workloads. Detailed native ABI/SDK/loading still requires reviewed feature contracts; no broad third-party execution admission. Preserve ADR-0026 where unsuperseded.
- PC display: shared logical 512x342 scene, unscaled and centered in the reported framebuffer until the owner says otherwise. 640x480 is a checked case, not a required mode.
- One x86-64 PC distribution with BIOS and UEFI entries (ADR-0067). "Any 64-bit PC" is a goal, not a claim.
- 286 lane: PC/AT 5170, 6 MHz, 640 KiB, EGA, hard-disk install. It is a separate 16-bit build. The C16 toolchain, segmented views and masks are prerequisites. The 576 KiB cap is unmeasured.
- Fixtures must be independently generated and rights-cleared. Private references stay outside Git and outside ordinary CI.
- Preserve UART implementation/test role separation; coordinating source reviewers do not inspect, edit or forward the held bodies. A task name alone does not waive that boundary.

## Gates that remain open

- Preserve target-specific historical, physical/full128K, three-edition/parity, B2, human-input, provenance-review, native-app and Linux gates. A result closes only its stated scope; human provenance review stays separate. Original/native-PC parity and deliberate Linux divergence are tracked separately.
- System1 interoperability in both directions: original System1 disks on our ROM, and our System1 on an identified supported Apple ROM. Both are mandatory and unverified. References must be admitted separately.
- B2 criterion 6: never relabel a firmware container hash as an image hash.
- Release blocker: the paired Mac motion/drag and PC GUI drag/release blocker persists. Release2 is NOT ready.
- ROM and System have separate versions. System is unreleased. Published prereleases are immutable.
- Build probes are not boots; an observed emulator boot is not physical validation, complete OS acceptance or compatibility proof. Keep E1 hosted, E2 device/API injection, E3 host GUI delivery and E4 human observations distinct; no substitution.

## Trials (when unpaused)

- The owner's SRC-0402..0411 three-scope approval for isolated, verified original-candidate Snow and VirtualBox trials is retained. Publication, reference runs and physical hardware each remain separate.
- Preserve existing VMs/media, close owned sessions, restore preferences and bound active time. Check actual foreground, physical owned-client containment and readiness; capture metadata/API returns are not guest acceptance. Stop an inconclusive route until a new discriminating observation/method exists. Use windowed Display1 with instructions visible; leave GLKVM/other displays untouched. Never fullscreen over the instructions.

## Preservation

- Keep the 24 untracked research originals and their byte-identical private copies, the held native-outline brief, and the 286 originals.
- Keep UART bodies, old records, rows, assets, tags, releases, PNG/GIF evidence and frozen wiki files.
- Never reset, stash, clean or pull over them.
- Reviewed own-product screenshots go in Git under `docs/evidence/screenshots` with indexed identity/rights manifests. Only the two already pinned GIF identities are admitted by [storage policy](docs/development/screenshot-evidence.md); no arbitrary images, reference pixels, desktop or emulator chrome.

## Process

- OneNote AND its Git wiki queue are permanently retired (SRC-0227). Never contact OneNote. Do not add or maintain any wiki, queue, mirror or manifest.
- HQ is the private `C:\Repos\ClasSICK`. Consult its current instructions before cross-project changes. Keep private HQ content out of this public repository.
- Workstation paths: source in `C:\Repos\ClasSICK-0.97`, build output in `C:\ClasSICK`. Core code must not depend on them.
- Protect Codex usage. Do not create extra chats, subagents or automation. Create specialist roles only when the user asks.
- Use one canonical receipt and reference-only indexes under [recordkeeping](docs/development/recordkeeping.md); preserve required provenance/spec/test/implementation/ADR records. Run the indexed repository guard before publication; inspect staged paths/history, source eligibility and fixture rights. No experiment journal in this file.
