# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,[switch]$Sanitizers)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$BuildRoot=[IO.Path]::GetFullPath($BuildRoot)
& (Join-Path $PSScriptRoot 'Verify-UEFILoader.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers
$hashes=[ordered]@{}
foreach($pair in @(@('clang','clang-hosted/debug-a','clang-hosted/debug-b'),@('gcc','gcc-debug-a','gcc-debug-b'))) {
    $artifact='classick_x64_state_tests.exe'
    $a=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[1]+'/'+$artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
    $b=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[2]+'/'+$artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
    if($a -ne $b){throw 'x64 state hosted twins differ.'}
    $hashes[$pair[0]]=$a; Write-Output "Matching x64 state SHA-256 $($pair[0]): $a"
}
$hashes | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'x64-state-hosted-hashes.json') -Encoding UTF8
Write-Output 'Owned x64 exception scaffold PASS: hosted descriptor bytes and unloaded native image only; hardware fault delivery/B2 remain unverified.'
