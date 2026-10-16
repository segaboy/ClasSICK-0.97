# Native mouse foundations - 2026-10-16

SPEC-0039 / ADR-0045 / IMPL-0043 / TEST-0047; SRC-0227..0232.
One shared original C button rule now serves the native Mac drawing ROM and
Windows/x86 viewer: count a down edge once, preserve held baseline, saturate,
and clear held state on capture cancellation. Windows clicks update both native
surfaces without format/flash changes; Space retains its earlier behavior.
The displayed counter is the saturated sum. Existing arena budget is unchanged.

The pure PS/2 decoder consumes three report bytes, uses header sign bits, reports
three buttons and refuses partial/corrupt/overflow output. It contains no port
I/O or controller routing. The existing keyboard controller driver still owns
the8042 and discards AUX. Freestanding PC mouse and full pointer/event parity
remain incomplete; this foundation does not close that milestone.

## Reviewed worker artifacts

Claude supplied four original C/header files,70 primary test lines, a46-line
contract draft and exact native binding edits. One240s call timed out without
a deliverable; one retry with supported low effort returned the substantial
artifact in184.91s. A requested thinking-token environment limit did not bound
actual thinking (11,198 reported); only the process deadline is claimed as a
hard bound. No further retries. Copilot returned199 independent C test lines
in51.52s, seeing the API contract alone. Both successful calls read one packet,
with no commands, writes, test execution, web, MCP or subagents. First failed
call also read only its packet. All raw captures remain outside Git.

Codex integrated the artifacts, normalized the spec path/status/source IDs,
selected native fault9/10, added an independent fixed counter-bound pixel check,
authored2048 Mac register traces, updated the viewer title and wired builds.
Core/decoder/worker tests required no semantic correction. Copilot's root include
paths are provided by CMake. Existing pixel/oracle/legacy qualification is reused,
with no new Antigravity/Grok call. Provider usage and the reported USD0.6088062
Claude list estimate are in [the receipt](mouse-input-evidence.json); they do
not identify subscription billing or translate to ChatGPT quota savings.

## Actual local verification

All773 CTests pass across Clang debug272, Clang ASan/UBSan272 and GCC i686229.
The final independent pixel-bound assertion was then rerun in both Windows
profiles,2 additional passes. Independent tests enumerate8,388,608 reports
plus untagged headers, malformed state/arguments, corruption and button edges.
Actual Mac loop/shared core agree on2048 original ten-sample transcripts.
Hidden Windows messages exercise capture/repeat/outside release/loss/cancel/
focus/Space sum/saturation and presentation. These are synthetic hosted tests.

Twenty strict Clang/GCC32/64-bit/O0/O2 and AArch64 little/big-endian object
audits pass with no imports or mutable globals. Two GNU68000 compile/import
checks pass. Initial sandbox WSL access was denied; ordinary approved development
compiler invocation succeeded, without running ClasSICK. Native ROM twins match:
65,536 bytes, SHA-256 `5e415b9036802fcbb217fc6b98872b7e620a9603dbf4b478055e23f5ad68fdd7`,7,732 used ROM bytes,2,160 data/BSS bytes,
1,032 workspace bytes,10,332 conservative stack bound within24,320 reserved;
largest individual frame604. Sixty-one source/header/contract inputs identified.
The decoder is not linked into the ROM; the shared button core is.
[Sanitized compiled identity record](mouse-input-builds.json).

CI at `8ba444410723616d9ed31a63904d0d6c1461e3fd` passes both Windows/68000 jobs, all2874
CTests across11 profiles,18 mutation tests and20 mouse object audits. The
native ROM and complete61-input manifest are byte-identical to local builds;
the optimized native PC EFI hash is unchanged. No PC boot mouse binding is
claimed. [CI receipt](https://github.com/segaboy/ClasSICK-0.97/actions/runs/38981277688).

## Current limits and next slice

No emulator, VM target, hardware/reference/human input, release or historical
claim. Older Mac loop/ROM entries, keyboard driver, UART bodies,24 research
originals/copies and previous releases/records remain intact. Next is a reviewed
single-owner8042 mouse startup/AUX-routing contract preserving keyboard deadlines,
then the corresponding native boot binding and fresh explicitly approved trials.

The owner permanently retired OneNote and its Git wiki queue/mirrors/manifest.
Existing wiki files are frozen history and unchanged; no OneNote service contact.
Ordinary development/provenance/HQ records continue. This supersedes every older
automatic/"update wiki" instruction; no further wiki records are maintained.
