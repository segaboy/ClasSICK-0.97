# Primary implementation artifact: first loaded ROM application - 2026-10-15

Owner SRC-0187: "OK, let's move on then." accepts an integration milestone:
our boot ROM loads an original CODE resource application, runs it under the
guarded interpreter, and renders its returned result. Codex authors the bridge,
integrates/builds/tests and remains the sole repository writer. Claude authors
the portable demo artifact; Grok later checks one actual boundary. Two planned
CLI calls, no overlapping general reviews. Existing source/native-app/physical/
128K/edition/human gates persist; no VM/emulator/reference trial is authorized.

Read only these public original project files in C:/Repos/ClasSICK-0.97:
- This brief (under100 lines).
- personality/system1/code-run.h (under40 lines), code-image.h (under110 lines).
- tests/code-image.c lines1..88: original resource encoder and manager/image setup.
- core/memory/manager.h lines1..90 if needed for configuration/error constants.
- core/memory/lifecycle.h lines1..46 if needed for inventory constants.
Each Read <=120 lines, at most7 Reads. No other files, HQ, untracked research,
manuals, credentials, Apple/Microsoft implementation/ROM/binaries/resources/disk
images/disassembly/emulator source, manual samples or private heap structures.
Read only; no commands, edits, web, MCP, extra agents/chats, automation or OneNote.
Standing SRC-0149 scoped original/public transfer applies. No native application.

Return ONLY two original C11 files, clearly separated by their repo paths:
apps/loaded-rom-app/probe.h and probe.c, <=300 lines combined. SPDX GPL-3.0-or-later
and copyright Dean Howell2026. Strict warning flags include -Wconversion and
-Wsign-conversion. No libc/malloc/memset/memcpy imports or mutable process globals.
Own byte copy/zero loops, buffers only in caller-supplied zeroed workspace.
No tests executed. Codex integrates; you do not write files.

Header defines cs_loaded_app_workspace with low[1024], ram[32768], staging[64],
regions[3], map, manager, lifecycle, image, image_workspace, description, image
request/report, run request/report, caller frame and one inventory entry. All
controls are disjoint sibling subobjects, not overlapping union storage. Source
resource fork and token are static CONST ROM arrays in probe.c, not workspace.
The source is original encoded bytes, not a borrowed manual/application sample.

Define cs_loaded_app_result with success, phase, value, pattern, steps, generation,
retired (all uint32_t), and function int cs_loaded_app_probe(workspace*,gateway*,result*).
All native inputs/outputs/buffers live/aligned/disjoint; gateway already initialized.
Only cold zeroed workspace supported; return0 and phase diagnostic on failures.
No CPU/frame/guest write if initial pointer arguments are null. Valid native
storage/protocol preconditions remain caller responsibilities. Never retry/reset
an unexpected image fault or scrub/hide its diagnostic state.

Map low at logical0..1024 RW; ram at0x1000..0x9000 RW; static four-NOP token at
logical0x500000..0x500008 RX, all present before image init/PREPARE. Config system
{identity0x400,begin0x1000,end0x1800}, application{0x402,0x2000,0x6000}.
Initialize manager then life then image (owner93, source capacity exact source
array length, max staging64, max world4096). One inventory FUTURE/REFUSE owner93,
native=&image, bytes=sizeof image. Ordinary context, no active/session/resource.
B=0x8000,S=512,H_min=0. PREPARE/LOAD use existing APIs, no second loader. Source
is immutable ROM data, source/staging request fields clear after PREPARE. Capture
generation/epoch from PREPARE. Check statuses; do not inspect private heap layout.

Original resource fork contains only CODE0 and CODE1, no names/duplicates/flags.
CODE0: above40, below16, table length8, table offset32; one unloaded row
{routine offset4,0x3F3C,segment1,0xA9F0}. CODE1 header{first entry0,count1}, then
the original twelve-byte program 20 4D 21 3C 5A 5A 5A 5A 70 61 4E 75:
MOVEA.L A5,A0; MOVE.L #0x5A5A5A5A,-(A0); MOVEQ #97,D0; RTS.
Its literal resource-start routine offset4 is the chosen engineering profile,
not a historical offset claim. Follow the existing original resource encoder's
format, verify data/map/resource offsets independently. Use a compact static
const byte array (designated initializers allowed) with no generated binary file.

After LOAD call cs_image_run with exact generation/epoch, token0x500000,budget32,
same context/gateway and zeroed caller frame. Run should return RETURNED, entered1,
steps5 (loaded JMP then four program instructions), guest.d[0]==97. Read the
BE32 at image.layout.a5-4 via cs_guest_read, require0x5A5A5A5A. Copy result scalar
before explicit retirement; then cs_image_run_retire with existing work/out/report.
Require image EMPTY and cs_image_external_admit OK, retired1. Set success1 last.
Use phase1 init,2 prepared,3 loaded,4 returned,5 result verified,6 retired; preserve
last completed phase on failure. Result fields must reflect observed results,
not unconditional success constants. No fake renderer or direct program execution.
