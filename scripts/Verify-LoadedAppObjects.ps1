# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,[switch]$ObjectsOnly)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$repo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'));$objects=Join-Path $BuildRoot 'loadedapp-objects'
if(Test-Path -LiteralPath $objects){throw 'Fresh loadedapp object directory required.'}
[void][IO.Directory]::CreateDirectory($objects);$oldPath=$env:PATH
try {
    & (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
    $clang=(Get-Command clang).Source;$nm=(Get-Command llvm-nm).Source
    $gcc=Join-Path $repo '.tools/gcc/w64devkit/bin/gcc.exe';$checks=0
    $modules=@(
        @('personality/system1/code-run.c','run','cs_guest_window,cs_guest_write,cs_image_execute,cs_image_execution_native,cs_image_execution_retire_native,cs_image_execution_integrity,cs_m68k_bus_guard_validate,cs_m68k_bus_step_admitted'),
        @('apps/loaded-rom-app/probe.c','probe','cs_guest_read,cs_image_execute,cs_image_external_admit,cs_image_init,cs_image_run,cs_image_run_retire,cs_life_init,cs_mm_init'))
    foreach($compiler in @($clang,$gcc)){foreach($bits in @(32,64)){foreach($opt in @(0,2)){
        foreach($module in $modules){
            $name=if($compiler -eq $clang){'clang'}else{'gcc'};$object=Join-Path $objects "$name-$bits-O$opt-$($module[1]).o"
            & $compiler "-m$bits" "-O$opt" -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo $module[0]) -o $object
            if($LASTEXITCODE -ne 0){throw 'Loaded-app object compile failed.'}
            & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=$($module[2])" -P (Join-Path $PSScriptRoot 'Check-ObjectImports.cmake')
            if($LASTEXITCODE -ne 0){throw 'Loaded-app object imports failed.'};++$checks
        }
    }}}
    foreach($target in @('aarch64-none-elf','aarch64_be-none-elf')){foreach($module in $modules){
        $object=Join-Path $objects "$target-$($module[1]).o"
        & $clang "--target=$target" -O2 -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo $module[0]) -o $object
        if($LASTEXITCODE -ne 0){throw 'Loaded-app endian compile failed.'}
        & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=$($module[2])" -P (Join-Path $PSScriptRoot 'Check-ObjectImports.cmake')
        if($LASTEXITCODE -ne 0){throw 'Loaded-app endian imports failed.'};++$checks
    }}
    if($ObjectsOnly){Write-Output 'Twenty loaded-app object audits PASS.';return}
    $hashes=[ordered]@{}
    foreach($pair in @(@('clang','clang-hosted/debug-a','clang-hosted/debug-b'),@('gcc','gcc-debug-a','gcc-debug-b'))){
        $a=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[1]+'/classick_loadedapp_tests.exe')) -Algorithm SHA256).Hash.ToLowerInvariant()
        $b=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[2]+'/classick_loadedapp_tests.exe')) -Algorithm SHA256).Hash.ToLowerInvariant()
        if($a -ne $b){throw 'Loaded-app executable twins differ.'};$hashes[$pair[0]]=$a
    }
    [ordered]@{specification='SPEC-0035';object_audits=$checks;hosted_twins=$hashes;emulator_run=$false} |
        ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'loadedapp-results.json') -Encoding UTF8
} finally {$env:PATH=$oldPath}
Write-Output 'Loaded application object audits and compiler twins PASS.'
