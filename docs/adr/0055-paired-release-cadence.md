# ADR-0055 - Work toward paired native preview releases

Accepted2026-10-19 owner direction,SRC-0291. Continue current work while making
each substantive slice advance a named release candidate. In context "PC and
x86 versions" means the existing native Macintosh68000 and native x86 PC tracks.

Adopt paired-native-preview-1 as the next bounded deliverable: a native drawing
technology preview with matching outline/selection/clip/branding/counter behavior.
Proposed next artifact versions are ROM0.3.0-alpha.1 and PC0.1.0-alpha.1; these
are draft target numbers,not assigned published versions. The existing ROM/System
version authority remains; add a currently unversioned/unreleased PC preview entry.
PC preview is an early delivery of the native PC workstream,not a completed System
edition or a separate semantic OS fork. Keep shared C and one matching feature matrix.

Parity means capabilities and evidence,not equal version numbers. A feature is
complete for this candidate only when both native sides are integrated and their
shared acceptance checks pass. Compilation,hosted conformance,actual target boots,
human input,package verification and publication have separate explicit states.
Use one code checkpoint for the pair and independently pinned artifact identities.
Do not advance a shared release-feature checkbox from one platform alone.

Prefer finishing candidate blockers over starting unrelated features or research.
Every substantive task brief names the candidate,expected feature/blocker change,
both target paths and acceptance evidence. Foundational work stays valid when it
has a direct dependency into that candidate; integrate the result rather than
leaving an indefinite chain of disconnected primitives. Freeze the preview once
the agreed feature set is ready; defer optional scope to a later release.

The next slice implements SPEC-0049 using SPEC-0048; it does not implement a full
desktop,guest outline ABI,USB/motion stack or external-native application loader.
Existing modes/releases/assets remain immutable. Both new candidates must later
receive exact-byte target qualification and a documented ordinary-user input
walkthrough before readiness. The PC diagnostic lifecycle must be explained and
accepted for a preview or changed in a separately scoped step before freeze.

No target/emulator/VM/hardware/reference run,tag,release publication or standing
launch permission follows from this cadence. Prepare concrete reviewable bundles
and reuse unchanged evidence; request only the existing final trial/publication
approvals when required. Screenshots from authorized own-product trials go to the
public evidence index. No OneNote/wiki maintenance,extra chats/subagents/automation.
Source/role/historical/physical/full128K/B2/human/Linux/complete-edition and both
System1/ROM goals remain. [Candidate](../../releases/paired-native-preview-1.md).
