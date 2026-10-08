# IMPL-0008 — Combined hosted startup and operator validation

Date 2026-10-08. Owner authority SRC-0028, host interfaces SRC-0029, existing
SPEC-0001–0005 and B1/M0.1 acceptance contract. TEST-0010. Author/reviewer:
Codex, self-review; human provenance review pending. Original AI-assisted C,
CMake and documentation using only eligible project sources and declared Win32
interfaces. No new portable-core contract or implementation.

Paths: apps/surface-demo/windows-main.c, CMakeLists.txt,
scripts/Check-Hosted-EarlyExit.cmake and development/provenance documentation.

`--verify-start` uses the normal GetMessage/TranslateMessage/DispatchMessage loop.
It checks initial arena/pixels, sends one normalized synthetic Space event, then
posts native Space press/repeat/release and a checkpoint to its own window. Checks
cover FIFO sequence/source, scene changes, actual timer/QPC deadline clearing and
posted Escape shutdown. It checks window destruction, removed timer, empty input,
retired arena and successful class unregistration before reporting PASS.

`--verify-start-incomplete` posts early Escape and must return 1 despite orderly
cleanup. Its independent CMake test checks exit status and the observed partial
event trace. This prevents a clean exit from being mistaken for acceptance.

`--validate-start` displays the original viewer and injects no input. Two Space
presses/releases and Escape must traverse the normal host input path; the timed
strip must have cleared. It records the bounded counts and shutdown state. Input
origin cannot be attested by counters alone; the actual operator session is
identified separately in TEST-0010. This option is never an unattended CI test.

Host cleanup now checks KillTimer and UnregisterClass failures. Core allocation,
surface/input/time behavior is unchanged. No Apple code, ROM/source/disassembly,
copied assets, private reference inputs, new tools or runtime adoption. Published
Win32 examples were visible but not copied/adopted. GPL-3.0-or-later for original
project work; existing external tool/runtime terms remain separate. No OS boot,
historical compatibility, full keyboard or independent human provenance approval.
