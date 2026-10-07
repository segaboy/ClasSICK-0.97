# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot, [switch]$Sanitizers)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$BuildRoot = [IO.Path]::GetFullPath($BuildRoot)
& (Join-Path $PSScriptRoot 'Verify-Presentation.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers
$artifact = 'classick_arena_tests.exe'
$first = (Get-FileHash -LiteralPath (Join-Path $BuildRoot ('debug-a/' + $artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
$second = (Get-FileHash -LiteralPath (Join-Path $BuildRoot ('debug-b/' + $artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
if ($first -ne $second) { throw "Fresh arena hashes differ: $artifact" }
Write-Output ("Matching SHA-256 {0}: {1}" -f $artifact, $first)
@{ $artifact = $first } | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'arena-hashes.json') -Encoding UTF8
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$originalPath = $env:PATH
try {
    & (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
    $compilerBin = Split-Path -Parent (Get-Command clang).Source
    foreach ($target in @('aarch64-none-elf','aarch64_be-none-elf')) {
        $object = Join-Path $BuildRoot ($target + '-arena.o')
        & clang "--target=$target" -std=c11 -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -ffreestanding -fno-builtin -fno-stack-protector -O2 -c (Join-Path $repoRoot 'core/memory/arena.c') -o $object
        if ($LASTEXITCODE -ne 0) { throw "Arena compile-only check failed: $target" }
        & cmake "-DNM=$(Join-Path $compilerBin 'llvm-nm.exe')" "-DOBJECT=$object" -P (Join-Path $PSScriptRoot 'Check-Freestanding.cmake')
        if ($LASTEXITCODE -ne 0) { throw "Arena import check failed: $target" }
    }
} finally { $env:PATH = $originalPath }
Write-Output 'SPEC-0003 verification PASS. Bounded arena/ownership contracts and viewer integration; B1 and all OS boots remain open.'
