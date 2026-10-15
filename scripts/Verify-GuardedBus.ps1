# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,[switch]$Sanitizers,[string]$Python='python')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
& (Join-Path $PSScriptRoot 'Verify-CodeImage.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers -Python $Python
$hashes=[ordered]@{}
foreach($pair in @(@('clang','clang-hosted/debug-a','clang-hosted/debug-b'),@('gcc','gcc-debug-a','gcc-debug-b'))){
    $a=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[1]+'/classick_guard_tests.exe')) -Algorithm SHA256).Hash.ToLowerInvariant()
    $b=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[2]+'/classick_guard_tests.exe')) -Algorithm SHA256).Hash.ToLowerInvariant()
    if($a -ne $b){throw 'Guarded bus executable twins differ.'};$hashes[$pair[0]]=$a
}
[ordered]@{specification='SPEC-0034';dependency='X-1';hosted_twins=$hashes;guest_object_audits_inherited=30;application_executed=$false} |
    ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'guarded-bus-results.json') -Encoding UTF8
Write-Output 'Guarded single-instruction verification PASS; no image/session execution authority.'
