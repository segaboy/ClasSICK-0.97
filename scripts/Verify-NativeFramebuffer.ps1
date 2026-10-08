# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
# SPEC-0009: retains the complete x64 exception wrapper, then pins the new hosted
# presenter/scene/gate twins and portable AArch64 LE/BE object audits.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,[switch]$Sanitizers)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$repoRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$BuildRoot=[IO.Path]::GetFullPath($BuildRoot)
& (Join-Path $PSScriptRoot 'Verify-X64Exceptions.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers
$hashes=[ordered]@{}
foreach($artifact in @('classick_native_framebuffer_tests.exe','classick_boot_scene_tests.exe','classick_native_present_tests.exe')) {
    foreach($pair in @(@('clang','clang-hosted/debug-a','clang-hosted/debug-b'),@('gcc','gcc-debug-a','gcc-debug-b'))) {
        $a=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[1]+'/'+$artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
        $b=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[2]+'/'+$artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
        if($a -ne $b){throw "Native presentation hosted twins differ: $($pair[0]) $artifact"}
        $hashes["$($pair[0]) $artifact"]=$a; Write-Output "Matching native presentation SHA-256 $($pair[0]) ${artifact}: $a"
    }
}
$originalPath=$env:PATH
try {
    & (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
    $clang=(Get-Command clang).Source; $nm=(Get-Command llvm-nm).Source
    $portable=@(@('platform/pc/framebuffer.c','cs_surface_init'),
        @('apps/boot-scene/scene.c','cs_surface_init,cs_surface_fill,cs_arena_alloc,cs_fb_init,cs_fb_present'))
    foreach($target in @('aarch64-none-elf','aarch64_be-none-elf')) {
        foreach($item in $portable) {
            $object=Join-Path $BuildRoot ($target+'-'+[IO.Path]::GetFileNameWithoutExtension($item[0])+'.o')
            & $clang "--target=$target" -std=c11 -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -ffreestanding -fno-builtin -fno-stack-protector -O2 -c (Join-Path $repoRoot $item[0]) -o $object
            if($LASTEXITCODE -ne 0){throw "Portable presenter endian compile failed: $target $($item[0])"}
            & cmake "-DNM=$nm" "-DALLOWED=$($item[1])" "-DOBJECT=$object" -P (Join-Path $PSScriptRoot 'Check-ObjectImports.cmake')
            if($LASTEXITCODE -ne 0){throw "Portable presenter endian audit failed: $target $($item[0])"}
        }
    }
} finally { $env:PATH=$originalPath }
$hashes | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'native-framebuffer-hashes.json') -Encoding UTF8
Write-Output 'Native framebuffer presentation PASS: hosted conversion/scene/gate tests, AArch64 LE/BE compile audits and unloaded EFI linkage only; no firmware, VM, visible output or B2.'
