# Original Windows test-input instrumentation

Host development tool only;IMPL-0078/TEST-0091,SRC-0456..0461.
The [review and actual tests](../../docs/development/windows-input-helper-evidence.md)
define its narrow qualification and unresolved native input results.

`helper.c` is C99 with an injected backend. ASCII rows are `dx dy delay button`
followed by LF or CRLF. dx/dy:-8..8;delay:10..1000ms;button:N,D,U.1..64 rows,
4096 bytes maximum,delay sum<=30000ms. One down/up pair maximum,released at end.
N with zero motion performs no input. The runner revalidates mutable plans.
Interrupted delivery never replays;cleanup is one guarded release,with an explicit
owed-release result when target ownership is lost.

`native.c` accepts `--target snow|vbox --plan PATH`;default is dry-run. `--live`
requires an independently verified,owned isolated session and compile-bound
`test-targets.h` generated outside Git. The exact schema is in native.c. No
real PID/path/hash header or active helper binary belongs in this repo. CI builds
fake guard tests with PID0 inactive bindings and compiles the read-only capture
module;it performs no desktop input. With the pinned development environment,
a manual host build uses C99,`WINVER=0x0A00` and `_WIN32_WINNT=0x0A00`
compiler definitions,helper.c,native.c,capture.c,an outside-Git binding
include directory,and user32/kernel32/bcrypt/ole32/oleaut32/uuid libraries.

`--physical-left` selects physical left down/up for a guest that consumes raw
button identity. The default remains the Windows primary button. Receipts and
dry-runs name the selected mode;neither mode changes the host's button setting.
All process/foreground/desktop/client/mapping guards and guarded cleanup still
apply. This selector does not establish that an emulator accepts injected input.

`--place-window` (with `--target` and `--plan`; dry-run by default) is a separate
positioning-only operation. The plan must be exactly one zero-motion N row, else
it is refused before target access. With `--live` it verifies the compile-bound
process, one visible owned window, Default desktop, button mapping
and no held physical button, then calls MoveWindow once on that window only:
screen position (100,100), size 1024x854, repaint. Base guards are rechecked
afterward. This positioning-only operation needs neither foreground nor cursor
containment, so a test window can leave a display occupied by another app without
interacting with that app. It never requests foreground, calls SendInput or runs
the plan;receipts report `mode` and `placement`. Normal input still requires
foreground and physical client/root containment. No host setting is changed.

An outside-Git target header may define `WH_TARGET_WINDOW` as the exact native
window ID returned for the owned guest canvas. Enumeration still checks PID,
visibility and nonzero client size;all input guards still apply. This separates
a fullscreen guest from its same-process floating toolbar. With the default0,
the original unique-visible-window rule applies. No live window ID enters Git.

An accepted API event is not guest input acceptance. A foreground refusal is a
valid stop. No focus hacks,desktop/security changes,preferences,hooks,services,
arbitrary keyboard,network or guest patching. `capture.c` reads only the exact
owned window's accessibility subtree and verifies the current captured indicator.
It performs no accessibility action. The existing source and trial gates apply.

SRC-0492 safety correction:read-only capture metadata never authorizes input.
Physical WindowFromPoint/root/client containment is mandatory even when the
owned UIA indicator says captured. A failed containment check may query that
indicator for the receipt,then rechecks the base guards and refuses.
[Verification and live limitations](../../docs/development/unlocked-input-evidence.md).
