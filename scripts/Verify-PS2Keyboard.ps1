# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
# SPEC-0012 retains the entire prior wrapper chain; hosted ports are callbacks.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,[switch]$Sanitizers)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$repoRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$BuildRoot=[IO.Path]::GetFullPath($BuildRoot)
& (Join-Path $PSScriptRoot 'Verify-ProgressLoop.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers
$hashes=[ordered]@{}
foreach($artifact in @('classick_ps2_tests.exe','classick_acpi_keyboard_tests.exe','classick_native_keyboard_tests.exe')) {
    foreach($pair in @(@('clang','clang-hosted/debug-a','clang-hosted/debug-b'),@('gcc','gcc-debug-a','gcc-debug-b'))) {
        $a=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[1]+'/'+$artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
        $b=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[2]+'/'+$artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
        if($a -ne $b){throw "Keyboard hosted twins differ: $($pair[0]) $artifact"}
        $hashes["$($pair[0]) $artifact"]=$a; Write-Output "Matching keyboard SHA-256 $($pair[0]) ${artifact}: $a"
    }
}
$originalPath=$env:PATH
try {
    & (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
    $clang=(Get-Command clang).Source; $nm=(Get-Command llvm-nm).Source
    foreach($target in @('aarch64-none-elf','aarch64_be-none-elf')) {
        $object=Join-Path $BuildRoot ($target+'-ps2.o')
        & $clang "--target=$target" -std=c11 -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -ffreestanding -fno-builtin -fno-stack-protector -O2 -c (Join-Path $repoRoot 'platform/pc/ps2.c') -o $object
        if($LASTEXITCODE -ne 0){throw "Keyboard endian compile failed: $target"}
        & cmake "-DNM=$nm" '-DALLOWED=cs_input_push' "-DOBJECT=$object" -P (Join-Path $PSScriptRoot 'Check-ObjectImports.cmake')
        if($LASTEXITCODE -ne 0){throw "Keyboard endian audit failed: $target"}
    }
} finally { $env:PATH=$originalPath }
$hashes | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'ps2-keyboard-hashes.json') -Encoding UTF8
Write-Output 'PS/2 keyboard PASS: hosted command/scan/queue/gate/scene tests, portable endian objects and unloaded EFI audits; no native keyboard or boot observation.'
