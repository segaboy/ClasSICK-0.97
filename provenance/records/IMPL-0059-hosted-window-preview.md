# IMPL-0059 / TEST-0067 - Windows hosted native window preview

2026-10-21;SRC-0332..0334;SPEC-0056;ADR-0064.
Separate opt-in native Windows adapter/executable binds the unchanged shared
input owner. Fixed640x480 monochrome frame centers unscaled512x342;larger clients
remain white,smaller clients clip. Original plus cursor,title drag,content toggle
and coherent click counter;capture/focus/cancel rules retain accepted input.

One Copilot primary returns305 complete header/adapter/main lines in64.22s,
one21,108-byte public/original prompt view,no edits/tests. Lead corrects its own
presenter briefing assumption,adds1:1 saved-DC wrapper,margin-release handling,
constructor live ordering and state-publication check;GUI/Unicode build wiring.
GCC64 exposes a test-only HGDI_ERROR signed-macro conversion;use object-type
validation. Independent adapter oracle predates worker output inspection;
subsequent directed cases cover clarified behavior. No second worker or retry.

313 full Clang64 CTests plus1 sanitized/1 GCC32 focused test pass. Each adapter
run verifies38 complete frames,3 client paints and1,656,254 assertions. Directed
traces cover signed outside coordinates,held input,press/release,drag/final
release,capture/focus/cancel loss,margins,resize,cleanup/reopen and fault stop.
GCC64-O2 passes;two compiled click-release/scaled-presentation defects fail.
Actual32/64 executables use the Windows GUI subsystem. Normal CI pending here.
[Exact evidence](../../docs/development/hosted-window-preview-evidence.json).

All observations are synthetic messages/hidden-window client painting. No real
GUI/human/boot/VM/emulator/hardware qualification or release. Old viewer/presenter,
shared cores,ROM/EFI source lists,24 originals/held brief/UART/old rows/releases/
tags/retired wiki preserved. Mac M1-M7,PC GUI/human and all source/role/historical/
edition/physical/full128K/B2/native-app/Linux/mini-vMac/both System1-ROM gates remain.


Owner additionally requests interesting stills and a drag animation. Two original
PNG client captures and a49-frame/4.2-second looping GIF are stored/indexed in Git.
All decoded pixels match capture;Clang64/GCC32 observers reproduce every pixel
stream and encoded asset. A GCC observer indentation warning is fixed without
semantic change. Narrow exact-path/exact-SHA GIF admission plus three tamper/
provenance unit cases;no arbitrary GIF policy expansion. No screenshots of a
reference system,desktop/chrome or firmware run. [Media](../../docs/evidence/screenshots/2026-10-21/README.md).


Initial Windows CI fails the hidden fixture's1280x960 size assertion. Lead
reproduces the monitor/tracking ceiling locally(800x600 ->784x561 client).
Test and capture observers explicitly suppress pre-change size validation;
product adapter is unchanged. Tests retain full large-client pixel assertions
under the forced ceiling. Three negative controls now include the reproduced
sizing failure;all315 final local CTests pass. Captures remain byte-identical.
[Failed CI retained](https://github.com/segaboy/ClasSICK-0.97/actions/runs/39597371865).


A later CI audit fails pointer/cursor object-twin equality after runtime
regressions pass. Failed remote objects were not retained. Lead reproduces the
same flags locally across time:only the COFF timestamp differs. SOURCE_DATE_EPOCH
alone is ineffective. One distinct-risk Grok review(22.38s,1,479-byte original
facts brief,zero tools/edits/tests) proposes incremental-linker control,unverified.
Lead checks compiler help and actual accepted -Xclang spelling. Five audit scripts
now generate stable headers with -mno-incremental-linker-compatible and deliberately
separate twins by1.1s. All72 object audits and6 full-file identical pairs pass.
No bytes are normalized/ignored,and runtime image compiler inputs are unchanged.
[Second failed CI retained](https://github.com/segaboy/ClasSICK-0.97/actions/runs/39599190731).
SRC-0335 records the bounded review;neither worker opinion nor object behavior
settles historical Macintosh behavior. Remaining provider balances unavailable.


The same five quick object audits now run immediately after fresh hosted builds,
before the long regression matrix. No check or gate is removed;all five scripts
were independently run against fresh object folders. This exposes compiler-audit
failures earlier instead of after the full matrix.
