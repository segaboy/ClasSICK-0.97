# Native selection claude artifact prompt

Baseline `36abbcf741440b9229ca647a3acb8ad2ad9c45f2`, Windows11/PowerShell.

You are a supervised artifact worker for ClasSICK 0.97 on Windows11/PowerShell.
Codex is sole repository writer and final verifier. Return proposed content only;
NO tools, commands, filesystem writes, discovery, web, manuals, other agents or tests.
Do not access HQ, untracked drafts, credentials, Apple/MS implementation, ROMs,
binaries, disk images, disassembly, emulator source or manual sample code. Inputs
below are original/reviewed public contract/API, baseline36abbcf741440b9229ca647a3acb8ad2ad9c45f2,
frozen before either implementation or independent-test author sees the other's work.
No historical assertions. Native synchronous values only; no trap/guest binding.
Return ONE JSON object {"files":[{"path":"relative/path","content":"full UTF8 LF text"}],
"findings":[],"tests_executed":[],"handoff":"<=100 words"}. No markdown fences.
One artifact only, <=400 physical lines; report blockers rather than invent API changes.


ASSIGNMENT: Implement the entire original freestanding C core exactly to the frozen API and contract. Return only personality/system1/quickdraw-select.c. Carefully preflight native overlap/alignment before dereferences; all validation before plan writes. No other library dependency or hosted utility. Do not generate tests.

Frozen exact API/contract inputs (copied inline, no other files):

- `personality/system1/quickdraw-select.h` SHA256 `cd72e096f6a9365ede45fd859df755fec021eb1986002d5ca8269a6c8f838624`
- `docs/specifications/SPEC-0043-native-port-selection.md` SHA256 `ad1a9dc4a56ee6d05e88c4e423d3f4ebef914c930afbad2942c9a1fe5c481e72`
- `personality/system1/quickdraw-view.h` SHA256 `4a93567872049f5142a3980eba501c7a2e027c15de794dde11fbbd93cc3bbee7`
- `personality/system1/quickdraw-rects.h` SHA256 `db9facdb5ef3742c3c8935255e3f627c30d31b323a8223e470d9851a48c81e09`
