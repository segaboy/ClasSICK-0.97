# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot, [switch]$Sanitizers)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$BuildRoot = [IO.Path]::GetFullPath($BuildRoot)
& (Join-Path $PSScriptRoot 'Verify-Input.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers
$hashes = @{}
foreach ($artifact in @('classick_clock_tests.exe','classick_windows_clock_tests.exe')) {
    $first = (Get-FileHash -LiteralPath (Join-Path $BuildRoot ('debug-a/' + $artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
    $second = (Get-FileHash -LiteralPath (Join-Path $BuildRoot ('debug-b/' + $artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($first -ne $second) { throw "Fresh clock hashes differ: $artifact" }
    $hashes[$artifact] = $first
    Write-Output ("Matching SHA-256 {0}: {1}" -f $artifact, $first)
}
$hashes | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'clock-hashes.json') -Encoding UTF8
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$originalPath = $env:PATH
try {
    & (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
    $compilerBin = Split-Path -Parent (Get-Command clang).Source
    foreach ($target in @('aarch64-none-elf','aarch64_be-none-elf')) {
        $object = Join-Path $BuildRoot ($target + '-clock.o')
        & clang "--target=$target" -std=c11 -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -ffreestanding -fno-builtin -fno-stack-protector -O2 -c (Join-Path $repoRoot 'core/time/clock.c') -o $object
        if ($LASTEXITCODE -ne 0) { throw "Clock compile-only check failed: $target" }
        & cmake "-DNM=$(Join-Path $compilerBin 'llvm-nm.exe')" "-DOBJECT=$object" -P (Join-Path $PSScriptRoot 'Check-Freestanding.cmake')
        if ($LASTEXITCODE -ne 0) { throw "Clock import check failed: $target" }
    }
} finally { $env:PATH = $originalPath }
Write-Output 'SPEC-0005 verification PASS. Portable time, deterministic clock and hosted timed behavior; B1 gate acceptance is recorded separately, with no OS boot claim.'
