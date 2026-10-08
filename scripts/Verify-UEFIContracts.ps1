# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,[switch]$Sanitizers)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$repoRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$BuildRoot=[IO.Path]::GetFullPath($BuildRoot)
& (Join-Path $PSScriptRoot 'Verify-CoreLink.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers
$hashes=[ordered]@{}
foreach($pair in @(@('clang','clang-hosted/debug-a','clang-hosted/debug-b'),@('gcc','gcc-debug-a','gcc-debug-b'))) {
    $name='classick_uefi_contract_tests.exe'
    $a=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[1]+'/'+$name)) -Algorithm SHA256).Hash.ToLowerInvariant()
    $b=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[2]+'/'+$name)) -Algorithm SHA256).Hash.ToLowerInvariant()
    if($a -ne $b){throw 'UEFI contract hosted twins differ.'}
    $hashes[$pair[0]]=$a
    Write-Output "Matching UEFI SHA-256 $($pair[0]): $a"
}
$originalPath=$env:PATH
try {
    & (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
    $clang=(Get-Command clang).Source
    $nm=(Get-Command llvm-nm).Source
    foreach($target in @('aarch64-none-elf','aarch64_be-none-elf')) {
        $object=Join-Path $BuildRoot ($target+'-uefi-contract.o')
        & $clang "--target=$target" -std=c11 -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -ffreestanding -fno-builtin -fno-stack-protector -O2 -c (Join-Path $repoRoot 'platform/uefi/contract.c') -o $object
        if($LASTEXITCODE -ne 0){throw 'UEFI contract endian compile failed.'}
        & cmake "-DNM=$nm" "-DOBJECT=$object" -P (Join-Path $PSScriptRoot 'Check-Freestanding.cmake')
        if($LASTEXITCODE -ne 0){throw 'UEFI contract endian object audit failed.'}
    }
} finally { $env:PATH=$originalPath }
$hashes | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'uefi-contract-hashes.json') -Encoding UTF8
Write-Output 'UEFI preboot contracts PASS: hosted data/ownership/exit-model tests and endian compile audits only; no firmware callback, VM launch or native boot.'
