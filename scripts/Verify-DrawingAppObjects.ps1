# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$repo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$objects=Join-Path $BuildRoot 'drawing-objects'
if(Test-Path -LiteralPath $objects){throw 'Fresh drawing object directory required.'}
[void][IO.Directory]::CreateDirectory($objects)
. (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
$clang=(Get-Command clang).Source;$nm=(Get-Command llvm-nm).Source
$gcc=Join-Path $repo '.tools/gcc/w64devkit/bin/gcc.exe';$checks=0
$modules=@(
    @('personality/system1/quickdraw-rects.c','rect',''),
    @('apps/native-drawing/workload.c','native','cs_qd_rect'),
    @('apps/drawing-rom-app/probe.c','probe','cs_loaded_app_prepare_source,cs_image_run_rectangles,cs_image_run_retire,cs_image_external_admit'),
    @('apps/drawing-rom-app/scene.c','scene','cs_product_scene'),
    @('apps/product-scene/scene.c','product','cs_surface_init,cs_surface_fill'),
    @('apps/boot-scene/scene.c','boot','cs_surface_init,cs_arena_alloc,cs_fb_init,cs_fb_present,cs_product_scene,cs_service_probe,cs_native_draw'))
foreach($compiler in @($clang,$gcc)){foreach($bits in @(32,64)){foreach($opt in @(0,2)){
    foreach($module in $modules){
        $name=if($compiler -eq $clang){'clang'}else{'gcc'}
        $object=Join-Path $objects "$name-$bits-O$opt-$($module[1]).o"
        & $compiler "-m$bits" "-O$opt" -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo $module[0]) -o $object
        if($LASTEXITCODE -ne 0){throw 'Drawing object compile failed.'}
        & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=$($module[2])" -P (Join-Path $PSScriptRoot 'Check-ObjectImports.cmake')
        if($LASTEXITCODE -ne 0){throw 'Drawing object imports failed.'};++$checks
    }
}}}
foreach($target in @('aarch64-none-elf','aarch64_be-none-elf')){foreach($module in $modules){
    $object=Join-Path $objects "$target-$($module[1]).o"
    & $clang "--target=$target" -O2 -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo $module[0]) -o $object
    if($LASTEXITCODE -ne 0){throw 'Drawing endian compile failed.'}
    & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=$($module[2])" -P (Join-Path $PSScriptRoot 'Check-ObjectImports.cmake')
    if($LASTEXITCODE -ne 0){throw 'Drawing endian imports failed.'};++$checks
}}
@{specification='SPEC-0037';object_audits=$checks;emulator_run=$false} |
    ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'drawing-object-results.json') -Encoding UTF8
Write-Output "$checks drawing object audits PASS."
