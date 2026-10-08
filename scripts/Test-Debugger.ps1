# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot, [string]$OutputDirectory = '')
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$BuildRoot = [IO.Path]::GetFullPath($BuildRoot)
if (-not $OutputDirectory) { $OutputDirectory = Join-Path $BuildRoot 'debugger' }
$OutputDirectory = [IO.Path]::GetFullPath($OutputDirectory)
if (Test-Path -LiteralPath $OutputDirectory) { throw 'Use a new debugger output directory; existing logs are preserved.' }
foreach ($build in @('debug-a','debug-b')) {
    foreach ($exe in @('classick_clock_tests.exe','classick_surface_demo.exe')) {
        if (-not (Test-Path -LiteralPath (Join-Path $BuildRoot "$build/$exe"))) { throw "Missing tested Debug executable: $build/$exe" }
    }
}
$originalPath = $env:PATH
$runs = @()
function Quote-Path([string]$path) {
    if ($path.Contains('"')) { throw 'Unsupported quotation mark in path' }
    '"' + $path.Replace('\','/') + '"'
}
function Require-Text([string]$body, [string[]]$patterns, [string]$name) {
    foreach ($pattern in $patterns) {
        if ($body -notmatch $pattern) { throw "Debugger evidence missing ($name): $pattern" }
    }
}
function Invoke-Debugger([string]$name, [string]$exe, [string[]]$commands, [int]$guestExit) {
    $commandPath = Join-Path $OutputDirectory "$name.commands"
    $logPath = Join-Path $OutputDirectory "$name.log"
    $map = 'settings set target.source-map . ' + (Quote-Path $repoRoot)
    [IO.File]::WriteAllLines($commandPath, @($map) + $commands, (New-Object Text.UTF8Encoding $false))
    $start = New-Object Diagnostics.ProcessStartInfo
    $start.FileName = $script:debugger
    $start.Arguments = '--no-lldbinit --no-use-colors --batch --file ' + (Quote-Path $exe) + ' --source ' + (Quote-Path $commandPath)
    $start.WorkingDirectory = $repoRoot
    $start.UseShellExecute = $false
    $start.CreateNoWindow = $true
    $start.WindowStyle = [Diagnostics.ProcessWindowStyle]::Hidden
    $start.RedirectStandardOutput = $true
    $start.RedirectStandardError = $true
    $process = New-Object Diagnostics.Process
    $process.StartInfo = $start
    try {
        if (-not $process.Start()) { throw 'Could not start pinned LLDB' }
        $stdout = $process.StandardOutput.ReadToEndAsync()
        $stderr = $process.StandardError.ReadToEndAsync()
        if (-not $process.WaitForExit(30000)) {
            # Only this newly launched debugger and its descendants are terminated.
            & (Join-Path $env:SystemRoot 'System32/taskkill.exe') /PID $process.Id /T /F | Out-Null
            $null = $process.WaitForExit(10000)
            throw "Debugger timeout: $name"
        }
        $body = $stdout.GetAwaiter().GetResult() + $stderr.GetAwaiter().GetResult()
        [IO.File]::WriteAllText($logPath, $body, (New-Object Text.UTF8Encoding $false))
        if ($process.ExitCode -ne 0 -or $body.Contains('error: ')) { throw "LLDB command failed: $name; inspect $logPath" }
        Require-Text $body @("Process [0-9]+ exited with status = $guestExit \(") $name
        return $body
    } finally { $process.Dispose() }
}
try {
    & (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
    $debugger = Join-Path (Split-Path -Parent (Get-Command clang).Source) 'lldb.exe'
    $version = & $debugger --version
    if ($LASTEXITCODE -ne 0 -or $version[0] -notmatch '^lldb version 23\.1\.1') { throw 'Pinned debugger unavailable or unexpected version' }
    $null = New-Item -ItemType Directory -Path $OutputDirectory
    # x64 hosted debugging is supported. The recorded WOW64 trial failed;
    # actual i686 conformance execution remains part of Verify-Clocks.
    foreach ($build in @('debug-a','debug-b')) {
        $clock = Join-Path $BuildRoot "$build/classick_clock_tests.exe"
        $viewer = Join-Path $BuildRoot "$build/classick_surface_demo.exe"
        $body = Invoke-Debugger "$build-core" $clock @(
            'breakpoint set --name cs_clock_advance', 'run deterministic',
            'frame variable clock->last duration', 'thread backtrace',
            'thread step-over', 'thread step-over', 'thread step-in',
            'frame variable base duration', 'thread step-out', 'thread step-over',
            'frame variable sum result', 'thread step-out', 'frame variable a.last b.last',
            'breakpoint disable', 'continue') 0
        Require-Text $body @(
            'stop reason = breakpoint', 'stop reason = step over', 'stop reason = step in', 'stop reason = step out',
            'cs_clock_advance.* at clock\.c:', 'cs_time_add.* at clock\.c:',
            'if \(clock == NULL\)', 'clocks\.c:',
            'clock->last = \(seconds = 7, nanoseconds = 900000000\)',
            'base = \(seconds = 7, nanoseconds = 900000000\)',
            'duration = \(seconds = 0, nanoseconds = 200000000\)',
            'sum = \(seconds = 8, nanoseconds = 100000000\)', 'result = CS_TIME_OK',
            'a.last = \(seconds = 8, nanoseconds = 100000000\)',
            'b.last = \(seconds = 1, nanoseconds = 0\)', 'SPEC-0005 portable clock: PASS') "$build-core"
        $runs += [ordered]@{name="$build-core";status='passed';sha256=(Get-FileHash $clock -Algorithm SHA256).Hash.ToLowerInvariant()}
        $body = Invoke-Debugger "$build-viewer-fake" $viewer @(
            'breakpoint set --name verify_print', 'run --verify-clock', 'thread backtrace',
            'frame variable state->arena.used state->input.storage[0] state->clock.last state->deadline state->flash',
            'continue', 'frame variable state->clock.last state->flash',
            'continue', 'frame variable state->clock.last state->flash',
            'breakpoint disable', 'continue') 0
        Require-Text $body @(
            'stop reason = breakpoint', 'verify_print.* at windows-main\.c:', 'verify_clock.* at windows-main\.c:',
            'command="--verify-clock "', 'state->arena.used = 1342744',
            'event = \(source = 1, key = 1, action = 1, repeat = 0\)', 'sequence = 1',
            'state->deadline = \(seconds = 0, nanoseconds = 250000000\)',
            '(?s)state->clock.last = \(seconds = 0, nanoseconds = 249999999\).*?state->flash = 1',
            '(?s)state->clock.last = \(seconds = 0, nanoseconds = 250000000\).*?state->flash = 0') "$build-viewer-fake"
        $runs += [ordered]@{name="$build-viewer-fake";status='passed';sha256=(Get-FileHash $viewer -Algorithm SHA256).Hash.ToLowerInvariant()}
        $body = Invoke-Debugger "$build-viewer-live" $viewer @(
            'breakpoint set --name verify_print', 'run --verify-clock-live',
            'frame variable state->clock_wakeups state->failed state->host_clock.frequency',
            'breakpoint disable', 'continue') 0
        Require-Text $body @('stop reason = breakpoint', 'state->failed = 0',
            'state->clock_wakeups = [1-9][0-9]*', 'state->host_clock.frequency = [1-9][0-9]*') "$build-viewer-live"
        $runs += [ordered]@{name="$build-viewer-live";status='passed';sha256=(Get-FileHash $viewer -Algorithm SHA256).Hash.ToLowerInvariant()}
        Write-Output "Debugger launch/source/state/step/resume PASS: $build"
    }
    $viewer = Join-Path $BuildRoot 'debug-a/classick_surface_demo.exe'
    foreach ($case in @(@('unknown','run --unknown'), @('extra','run --verify --verify-clock'), @('empty','run ""'))) {
        $null = Invoke-Debugger ("reject-" + $case[0]) $viewer @($case[1]) 2
        $runs += [ordered]@{name=("reject-" + $case[0]);status='passed'}
    }
    [ordered]@{debugger=$version[0];runs=$runs} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $OutputDirectory 'results.json') -Encoding UTF8
    Write-Output 'TEST-0009 debugger verification PASS: six valid x64 LLDB runs and three argument rejections. WOW64 debugging unsupported; interactive local review and B1 gate acceptance recorded separately. No OS boot claim.'
} finally { $env:PATH = $originalPath }
