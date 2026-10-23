# Optional original PC286 compile-only probes

SRC-0386..0389;IMPL-0067/TEST-0079. These are standalone original inputs for an
optional Windows-hosted WCC16 qualification. They are not part of CMake/CI's
normal C11/runtime targets and have no target entry point. The C11 and bad-width
cases intentionally fail in this pinned preparation profile.

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
