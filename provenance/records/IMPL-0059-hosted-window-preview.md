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

312 full Clang64 CTests plus1 sanitized/1 GCC32 focused test pass. Each adapter
run verifies38 complete frames,3 client paints and1,656,250 assertions. Directed
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
