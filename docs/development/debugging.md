# Debug the hosted core and viewer

Verified scope: bundled LLDB 23.1.1, unoptimized x64 Debug executables and mapped
source. Six replayed core/viewer sessions across fresh Debug twins plus three
argument rejections pass. A local interactive core session also passed. Current
core/adapter conformance remains 34 checks per configuration.

Prepare the pinned tools, then build/verify into a fresh directory:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Verify-Debugger.ps1 -BuildRoot C:\ClasSICK\debug-local -Sanitizers
```

For a local interactive session, activate tools and launch LLDB from the source
checkout. Substitute your own source/build paths; settings apply only to this
debugger process. Startup uses --no-lldbinit so personal startup scripts are not
executed. No Python script or separate runtime installation is required.

```powershell
.\scripts\Enter-DevEnvironment.ps1
lldb --no-lldbinit --no-use-colors --file C:\ClasSICK\debug-local\debug-a\classick_clock_tests.exe
```

Enter these commands at the debugger prompt, waiting for each stop:

```text
settings set target.source-map . C:/Repos/ClasSICK-0.97
breakpoint set --name cs_clock_advance
run deterministic
frame variable clock->last duration
thread backtrace
thread step-over
thread step-over
thread step-in
frame variable base duration
thread step-out
thread step-over
frame variable sum result
thread step-out
frame variable a.last b.last
breakpoint disable
continue
quit
```

The first clock is 7s/900000000ns, duration 0s/200000000ns. Step into our addition,
then back to the caller: sum and the first clock become 8s/100000000ns; the second
clock remains 1s/0ns. Wait until the result assignment completes before inspecting
the result variable. Continue must print the portable-clock PASS and exit zero.
Source remapping restores the checkout path removed for reproducible executables.

To inspect the viewer, open classick_surface_demo.exe instead, set a breakpoint
on verify_print and run --verify-clock. Inspect state->arena.used (1342744),
state->input.storage[0], state->clock.last, state->deadline and state->flash.
Repeated continue/inspection shows the strip on at 249999999ns and off at exactly
250000000ns. Disable breakpoints and continue to exit zero. The --verify-clock-live
mode also samples a real Windows timer/QPC wakeup and closes; pauses affect wall
time, so it makes no exact-delivery or physical-accuracy promise.

LLDB adds a trailing delimiter to raw WinMain arguments. The viewer uses the
host runtime's parsed argument vector so this launch succeeds. Unknown options,
multiple options and an empty option reject with exit 2 before UI initialization.

For replay against already verified Debug twins, preserve existing logs by choosing
a fresh output directory:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Test-Debugger.ps1 -BuildRoot C:\ClasSICK\debug-local -OutputDirectory C:\ClasSICK\debug-replay
```

Replay checks actual source stops, typed values, call stacks, stepping and debuggee
exit, not just LLDB's exit code. Each launch is bounded to 30 seconds; timeout
cleanup targets only the launched debugger process tree. Commands/logs/results stay
local and may contain native addresses/paths; publish sanitized evidence only.

An extra i686/WOW64 trial stopped at exception 0x4000001f in the compatibility
layer instead of providing the original frame. It failed inspection. This resembles
the [reported LLVM symptom](https://github.com/llvm/llvm-project/issues/58065), with
no proven root cause or workaround. Actual i686 conformance still passes. Use x64
for this verified debugging workflow; 32-bit, optimized/sanitized debug sessions,
remote/guest/m68k/ARM64 debugging, IDE integration and other tools remain unverified.
B1 hosted-start acceptance and all OS boot gates remain separate.
