# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$repo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$BuildRoot=[IO.Path]::GetFullPath($BuildRoot)
if(Test-Path -LiteralPath $BuildRoot){throw 'Use a fresh mono presentation object directory.'}
[void][IO.Directory]::CreateDirectory($BuildRoot)
. (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
$clang=(Get-Command clang).Source;$nm=(Get-Command llvm-nm).Source
$gcc=Join-Path $repo '.tools/gcc/w64devkit/bin/gcc.exe'
$source=Join-Path $repo 'platform/pc/mono-present.c'
$allowed='cs_fb_init,cs_surface_init,cs_fb_present';$audits=@()
foreach($compiler in @($clang,$gcc)){foreach($bits in @(32,64)){foreach($opt in @(0,2)){
    $name=if($compiler -eq $clang){'clang'}else{'gcc'}
    $object=Join-Path $BuildRoot "$name-$bits-O$opt.o"
    & $compiler "-m$bits" "-O$opt" -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -fstack-usage -c $source -o $object
    if($LASTEXITCODE -ne 0){throw 'Mono presentation object compile failed.'}
    & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=$allowed" -P (Join-Path $PSScriptRoot 'Check-ObjectImports.cmake')
    if($LASTEXITCODE -ne 0){throw 'Mono presentation imports failed.'}
    $audits+=@{profile="$name-$bits-O$opt";object_bytes=(Get-Item -LiteralPath $object).Length}
}}}
foreach($target in @('aarch64-none-elf','aarch64_be-none-elf')){
    $object=Join-Path $BuildRoot "$target.o"
    & $clang "--target=$target" -O2 -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -fstack-usage -c $source -o $object
    if($LASTEXITCODE -ne 0){throw 'Mono presentation endian compile failed.'}
    & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=$allowed" -P (Join-Path $PSScriptRoot 'Check-ObjectImports.cmake')
    if($LASTEXITCODE -ne 0){throw 'Mono presentation endian imports failed.'}
    $audits+=@{profile=$target;object_bytes=(Get-Item -LiteralPath $object).Length}
}
@{specification='SPEC-0060';object_audits=$audits;target_run=$false;whole_stack_budget_certified=$false} |
    ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $BuildRoot 'mono-objects.json') -Encoding utf8
Write-Output "SPEC-0060 mono presentation: $($audits.Count) object audits PASS; no image executed."
