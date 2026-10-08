# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
# SPEC-0011: retains the complete PM timer wrapper, then pins the timed progress
# loop hosted twins. The loop is hosted with synthetic counters only.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,[switch]$Sanitizers)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$BuildRoot=[IO.Path]::GetFullPath($BuildRoot)
& (Join-Path $PSScriptRoot 'Verify-PMTimer.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers
$hashes=[ordered]@{}
foreach($artifact in @('classick_native_loop_tests.exe','classick_native_timer_tests.exe','classick_native_present_tests.exe')) {
    foreach($pair in @(@('clang','clang-hosted/debug-a','clang-hosted/debug-b'),@('gcc','gcc-debug-a','gcc-debug-b'))) {
        $a=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[1]+'/'+$artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
        $b=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[2]+'/'+$artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
        if($a -ne $b){throw "Progress loop hosted twins differ: $($pair[0]) $artifact"}
        $hashes["$($pair[0]) $artifact"]=$a; Write-Output "Matching progress loop SHA-256 $($pair[0]) ${artifact}: $a"
    }
}
$hashes | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'progress-loop-hashes.json') -Encoding UTF8
Write-Output 'Timed progress loop PASS: hosted gate/progress/stall/value tests and unloaded EFI linkage only; no firmware, VM, real timer, visible output or B2.'
