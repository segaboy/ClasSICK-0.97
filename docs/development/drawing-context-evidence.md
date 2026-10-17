# Paired native drawing context - 2026-10-17

SPEC-0041 / ADR-0047 / IMPL-0045 / TEST-0050; SRC-0240..0243.
The same original native view now applies coordinate translation and a physical
rectangular clip on68000/x86. It borrows the unchanged1032-byte fixed port and
delegates rasterization to the unchanged rectangle helper. No global port,
guest GrafPort/InitGraf/GetPort/SetPort ABI, regions, cursor/events or SDK is added.
This is native engineering, not a new historical Macintosh compatibility claim.

The native workload uses offset(8,8), clips an extreme signed rectangle to the
old outer rectangle, then translates its erase rectangle. The existing2304
black pixels, counts, hash, result97, six phases and complete scene remain exact.
This exercises the new operations in the real Mac/PC/Windows native workload;
the old40-byte68k fixture and guest binding stay independent and unchanged.
No scene redesign or synthetic substitute for native drawing is involved.

## Reviewed worker artifacts

Claude supplied22 header lines,144 core lines,35 native-workload lines and a65-line
spec in64.95s, one call. Code/header/workload adopted unchanged; lead finalized
spec metadata and boundaries. Copilot supplied351 independent test lines in86.05s,
one call, from API/contract only. It exceeded the260-line budget by91 and included
two assertions that could never fail, a self-comparison and one wrong known-pixel
expectation. Those were removed/corrected; fixture comparison no longer assumes
padding, and one unused local was removed. The independent inverse-mapping oracle
and substantive cases were adopted. No additional provider call was needed.

Each successful worker read only its one frozen packet and executed no commands,
edits, tests, web/MCP, discovery or subagents. Captures remain outside Git; no HQ,
manual, untracked original, credential, prohibited implementation or UART body was
supplied. Existing Grok/Antigravity qualifications were reused. Codex alone reviewed,
integrated and verified. Provider counters are in [JSON](drawing-context-evidence.json);
they are not subscription bills or measured ChatGPT quota equivalences.

## Actual verification

All804 local CTests pass: Clang debug283, ASan/UBSan283 and GCC32-bit238. Independent
per-pixel int64 inverse-coordinate checks cover256 random cases, byte boundaries,
32,400 offset/endpoint combinations,625 clip combinations, directed empty/inverted
cases, invalid-state/null/unknown refusal, saturation, rebind and two-view/port
isolation. Full old native/legacy image equality and failure-retention suites pass.

70 Clang/GCC32/64/O0/O2 and ARM little/big-endian object audits pass, including the
native workload and view. The view imports only the old rasterizer; the workload
imports the four view APIs. Original EFI O0/O2 twins match;27 corruption and19
omitted-object controls reject, including missing view. No image was executed.
[Candidate identities](drawing-context-builds.json). CI at `0563837809cd5201498a175b28f99af0721ce61e` passes both jobs: all2991
CTests across11 profiles,18 mutation tests,70 drawing and34 mouse object audits
with2 expected native-ABI rejections. The PC EFI matches the local candidate.
[CI receipt](https://github.com/segaboy/ClasSICK-0.97/actions/runs/39028795556).

The pinned CI build produces matching native68000 ROM twins, SHA-256
`48c0792bf3d9e01a59c33eccface86b89c88ee8905a3d153032dc005df70e6a2`. ROM used8456 bytes; data/BSS2160;
workspace1032; conservative stack10456 within24320, maximum
frame604. The complete64-input manifest matches current sources.
Native workload/view/rasterizer/helpers link without undefined imports or interpreter
symbols. No local68000 ROM build is claimed; these are partial-profile bounds. These new native candidates do not inherit old boot observations. The
previous PC mouse trial remains a closed observation of its exact earlier EFI.

No new VM/emulator/hardware/reference trial or release. Preserve all24 originals/
copies, UART bodies, old helper/guest fixture, prior rows/releases/tags and frozen
wiki. OneNote/wiki remain retired. Full guest port state, drawing-manager install,
regions, native pointer/events, human input, historical/physical/full128K/edition/B2
and both System1/ROM requirements remain open. Next is a bounded guest port/manager
contract using this native foundation; future target trials need fresh approval.
