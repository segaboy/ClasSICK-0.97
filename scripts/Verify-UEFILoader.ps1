# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,[switch]$Sanitizers)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$BuildRoot=[IO.Path]::GetFullPath($BuildRoot)
& (Join-Path $PSScriptRoot 'Verify-UEFIContracts.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers
$hashes=[ordered]@{}
foreach($pair in @(@('clang','clang-hosted/debug-a','clang-hosted/debug-b'),@('gcc','gcc-debug-a','gcc-debug-b'))) {
    $name='classick_uefi_loader_tests.exe'
    $a=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[1]+'/'+$name)) -Algorithm SHA256).Hash.ToLowerInvariant()
    $b=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[2]+'/'+$name)) -Algorithm SHA256).Hash.ToLowerInvariant()
    if($a -ne $b){throw 'Loader hosted twins differ.'}
    $hashes[$pair[0]]=$a; Write-Output "Matching loader SHA-256 $($pair[0]): $a"
}
$hashes | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'loader-hosted-hashes.json') -Encoding UTF8
& (Join-Path $PSScriptRoot 'Verify-UEFIImage.ps1') -BuildRoot (Join-Path $BuildRoot 'efi')
Write-Output 'UEFI loader PASS: original x64 ABI calls against hosted mocks and unloaded EFI inspection; firmware/native boot remains unverified.'
