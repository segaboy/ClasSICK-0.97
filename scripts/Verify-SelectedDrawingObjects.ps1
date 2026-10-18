# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$repo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if(Test-Path -LiteralPath $BuildRoot){throw 'Fresh selected drawing object directory required.'}
[void][IO.Directory]::CreateDirectory($BuildRoot)
. (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
$clang=(Get-Command clang).Source;$nm=(Get-Command llvm-nm).Source
$gcc=Join-Path $repo '.tools/gcc/w64devkit/bin/gcc.exe'
$modules=@(
    @('apps/selected-drawing/workload.c','workload','cs_qd_select_get,cs_qd_select_set,cs_qd_view_init,cs_qd_view_offset,cs_qd_view_clip,cs_qd_view_rect'),
    @('personality/system1/quickdraw-select.c','selection',''),
    @('apps/boot-scene/scene.c','boot','cs_surface_init,cs_arena_alloc,cs_fb_init,cs_fb_present,cs_product_scene,cs_service_probe,cs_native_draw,cs_selected_draw'))
$checks=0
foreach($compiler in @($clang,$gcc)){foreach($bits in @(32,64)){foreach($opt in @(0,2)){foreach($module in $modules){
    $name=if($compiler -eq $clang){'clang'}else{'gcc'}
    $object=Join-Path $BuildRoot "$name-$bits-O$opt-$($module[1]).o"
    & $compiler "-m$bits" "-O$opt" -DCLASSICK_SELECTED_DRAWING=1 -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo $module[0]) -o $object
    if($LASTEXITCODE -ne 0){throw 'Selected drawing object compile failed.'}
    & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=$($module[2])" -P (Join-Path $PSScriptRoot 'Check-ObjectImports.cmake')
    if($LASTEXITCODE -ne 0){throw 'Selected drawing object imports/data failed.'};++$checks
}}}}
foreach($target in @('aarch64-none-elf','aarch64_be-none-elf')){foreach($module in $modules){
    $object=Join-Path $BuildRoot "$target-$($module[1]).o"
    & $clang "--target=$target" -O2 -DCLASSICK_SELECTED_DRAWING=1 -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo $module[0]) -o $object
    if($LASTEXITCODE -ne 0){throw 'Selected drawing endian compile failed.'}
    & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=$($module[2])" -P (Join-Path $PSScriptRoot 'Check-ObjectImports.cmake')
    if($LASTEXITCODE -ne 0){throw 'Selected drawing endian imports/data failed.'};++$checks
}}
@{specification='SPEC-0047';object_audits=$checks;target_executed=$false} | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'manifest.json') -Encoding UTF8
Write-Output "$checks selected drawing object audits PASS."
