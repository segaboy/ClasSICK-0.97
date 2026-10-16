# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$repo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$objects=Join-Path $BuildRoot 'mouse-objects'
if(Test-Path -LiteralPath $objects){throw 'Fresh mouse object directory required.'}
[void][IO.Directory]::CreateDirectory($objects)
. (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
$clang=(Get-Command clang).Source;$nm=(Get-Command llvm-nm).Source
$gcc=Join-Path $repo '.tools/gcc/w64devkit/bin/gcc.exe';$checks=0
$modules=@(@('core/input/mouse.c','button'),@('platform/pc/ps2-mouse.c','packet'))
foreach($compiler in @($clang,$gcc)){foreach($bits in @(32,64)){foreach($opt in @(0,2)){
    foreach($module in $modules){
        $name=if($compiler -eq $clang){'clang'}else{'gcc'}
        $object=Join-Path $objects "$name-$bits-O$opt-$($module[1]).o"
        & $compiler "-m$bits" "-O$opt" -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo $module[0]) -o $object
        if($LASTEXITCODE -ne 0){throw 'Mouse object compile failed.'}
        & cmake "-DNM=$nm" "-DOBJECT=$object" -P (Join-Path $PSScriptRoot 'Check-Freestanding.cmake')
        if($LASTEXITCODE -ne 0){throw 'Mouse object imports failed.'};++$checks
    }
}}}
foreach($target in @('aarch64-none-elf','aarch64_be-none-elf')){foreach($module in $modules){
    $object=Join-Path $objects "$target-$($module[1]).o"
    & $clang "--target=$target" -O2 -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo $module[0]) -o $object
    if($LASTEXITCODE -ne 0){throw 'Mouse endian compile failed.'}
    & cmake "-DNM=$nm" "-DOBJECT=$object" -P (Join-Path $PSScriptRoot 'Check-Freestanding.cmake')
    if($LASTEXITCODE -ne 0){throw 'Mouse endian imports failed.'};++$checks
}}
@{specification='SPEC-0039';object_audits=$checks;emulator_run=$false} |
    ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'mouse-objects.json') -Encoding utf8
Write-Output "SPEC-0039 mouse object audits: $checks PASS"
