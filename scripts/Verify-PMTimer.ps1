# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
# SPEC-0010: retains the complete native framebuffer wrapper, then pins the ACPI,
# PM timer and native probe hosted twins and portable AArch64 LE/BE object audits.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,[switch]$Sanitizers)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$repoRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$BuildRoot=[IO.Path]::GetFullPath($BuildRoot)
& (Join-Path $PSScriptRoot 'Verify-NativeFramebuffer.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers
$hashes=[ordered]@{}
foreach($artifact in @('classick_acpi_tests.exe','classick_pmtimer_tests.exe','classick_native_timer_tests.exe','classick_uefi_loader_tests.exe')) {
    foreach($pair in @(@('clang','clang-hosted/debug-a','clang-hosted/debug-b'),@('gcc','gcc-debug-a','gcc-debug-b'))) {
        $a=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[1]+'/'+$artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
        $b=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[2]+'/'+$artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
        if($a -ne $b){throw "PM timer hosted twins differ: $($pair[0]) $artifact"}
        $hashes["$($pair[0]) $artifact"]=$a; Write-Output "Matching PM timer SHA-256 $($pair[0]) ${artifact}: $a"
    }
}
$originalPath=$env:PATH
try {
    & (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
    $clang=(Get-Command clang).Source; $nm=(Get-Command llvm-nm).Source
    foreach($target in @('aarch64-none-elf','aarch64_be-none-elf')) {
        foreach($source in @('platform/pc/acpi.c','platform/pc/pmtimer.c')) {
            $object=Join-Path $BuildRoot ($target+'-'+[IO.Path]::GetFileNameWithoutExtension($source)+'.o')
            & $clang "--target=$target" -std=c11 -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -ffreestanding -fno-builtin -fno-stack-protector -O2 -c (Join-Path $repoRoot $source) -o $object
            if($LASTEXITCODE -ne 0){throw "PM timer endian compile failed: $target $source"}
            & cmake "-DNM=$nm" "-DOBJECT=$object" -P (Join-Path $PSScriptRoot 'Check-Freestanding.cmake')
            if($LASTEXITCODE -ne 0){throw "PM timer endian audit failed: $target $source"}
        }
    }
} finally { $env:PATH=$originalPath }
$hashes | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'pm-timer-hashes.json') -Encoding UTF8
Write-Output 'ACPI PM timer PASS: hosted ACPI walk, counter extension, map-checked reader and probe tests, AArch64 LE/BE audits and unloaded EFI linkage only; no firmware, VM, port access or B2.'
