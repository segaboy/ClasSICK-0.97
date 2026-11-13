# Original PC286 probes

SRC-0386..0389;IMPL-0067/TEST-0079. The original toolchain probes are
standalone inputs for an optional Windows-hosted WCC16 qualification. They are
not part of CMake/CI's normal C11/runtime targets and have no target entry
point. The C11 and bad-width cases intentionally fail in that profile. The
separate BootHush diagnostics below do have target entry points.

Obtain the exact publisher package named in
[the tool lock](../../scripts/pc286-toolchain-lock.json),keep it outside Git,
and supply a new output directory outside the checkout:

```powershell
python -B scripts/Probe-PC286Toolchain.py --archive C:/YOUR-CACHE/open-watcom-2_0-c-win-x64.exe --output C:/YOUR-WORK/new-pc286-check
```

The tool verifies archive/member hashes before extraction,retains the license,
extracts only selected compiler/object tools and5 standard-interface headers,
compiles39 cases,inspects our own objects and writes report.json plus logs.
It does not download,run the installer,edit PATH/registry,link runtime helpers,
produce a boot disk or execute target code. Only host compiler/object tools run.
No emulator/VM/firmware permission follows. Compiler calls have20-second caps.
The selected ISA guard is deliberately nonexhaustive. Full logs stay outside Git.

See the [review/results](../../docs/development/pc286-toolchain-preparation.md).

## Headless 286 capability diagnostic

[Test-BootHushPC286Capability.py](../../scripts/Test-BootHushPC286Capability.py)
compiles the existing original capability model with the pinned WCC16 toolchain,
then runs a generated diagnostic floppy headlessly in BootHush. It verifies
cross-bank borrows, permission and lifetime refusals, and four fixed far-memory
windows; three wrong-expectation controls and a near-data compiler refusal are
included. Supply an already qualified toolchain, the BootHush executable and a
new output directory outside the checkout:

```powershell
python -B scripts/Test-BootHushPC286Capability.py `
  --tool-root C:/ClasSICK/pc286-toolchain-20261023-a/toolchain `
  --emulator C:/Repos/boothush/out/windows/boothush.exe `
  --output C:/ClasSICK/YOUR-NEW-CAPABILITY-TEST
```

The generated floppy is an original test fixture, not a 286 system disk.
[Evidence and limitations](../../docs/development/pc286-capability-boot-20261113.md).

The separate [span-construction diagnostic](../../scripts/Test-BootHushPC286SpanBuild.py)
executes the actual numeric planner and far-pointer binder at a 32,767-byte
split and a nonzero segment offset, then checks cross-bank access and refusals.
[Evidence](../../docs/development/boothush-developer-report-20261113-b.md).

Add `--capability-chain` to run planned spans through the actual capability
layer in one guest. The original two-track diagnostic loader is separately
checked by [Test-BootHushPC286Loader.py](../../scripts/Test-BootHushPC286Loader.py).
These remain original diagnostic media, not a 286 operating-system boot.
[Evidence](../../docs/development/boothush-developer-report-20261113-c.md).
