# Primary original artifact - fixed rectangle helper, 2026-10-16

Owner SRC-0202 continues to first loaded drawing APIs. Codex keeps coordination,
one repository writer, tests/compiler/CI and final decisions. Standing SRC-0149
permits this reviewed-public/original transfer to Claude. No new trial or release.

Read only these original public files in C:/Repos/ClasSICK-0.97:
- This brief (under55 lines).
- personality/system1/quickdraw-rects.h (under30 lines).
- docs/specifications/SPEC-0036-loaded-rectangle-calls.md lines1..44 only.
At most3 Reads, <=110 lines each. No other files, commands, writes, tests, web,
MCP, extra agents/chats/automation/OneNote. No HQ, untracked drafts, manuals,
credentials, Apple/Microsoft implementation/binaries/ROM/assets/media, emulator
source, manual examples or private structure reconstruction. Read only.

Return only one C11 source block for personality/system1/quickdraw-rects.c,
<=150 lines, implementing the header. No test execution or file edit.
SPDX GPL-3.0-or-later and copyright Dean Howell2026. Strict -Wconversion,
-Wsign-conversion, -Wpedantic/-Werror. No libc/malloc/memset/memcpy imports,
mutable globals, guest address interpretation, caller callbacks or hidden state.

Rect bytes decode as signed16 BE top,left,bottom,right, promoted to int32_t
without implementation-defined signed narrowing. Port is fixed128x64,1024
MSB-first pixel bytes, stride16. Clip to its half-open extent. Inverted/empty/
off-screen calls succeed without pixel changes; all other pixels survive.
Canonical A8A2 sets covered bits, A8A3 clears them. Native paints/erases diagnostic
increments on success including empty calls; saturate at UINT32_MAX. Null/unknown
word returns0 with absolutely no state change. All native extents are live and
disjoint by contract. No historical structure/state beyond this selected profile.

Use an original bounded masked-byte algorithm, including correct first/last
byte intersections. Do not call the generic surface renderer. Codex supplies
an independent per-pixel/set oracle and integrates the separate safe trap bridge.
Do not propose a second implementation/review/architecture; just this artifact.
