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
$gcc=Join-Path $repo '.tools/gcc/w64devkit/bin/gcc.exe';$checks=0;$abiRejections=0
$busImports='cs_ps2_begin,cs_ps2_poll,cs_ps2_receive,cs_ps2_mouse_begin,cs_ps2_mouse_feed,cs_mouse_button_begin,cs_mouse_button_sample'
$nativeImports='cs_ps2_bus_begin,cs_ps2_bus_poll,cs_native_read,cs_uefi_check_map,cs_acpi_find_pm_timer,cs_acpi_8042,cs_uefi_check_framebuffer,cs_fb_init,cs_arena_init,cs_arena_alloc,cs_boot_scene_prepare,cs_boot_scene_draw_input,cs_input_init,cs_input_pop,cs_pmtimer_init,cs_pmtimer_sample,cs_pmtimer_time'
$modules=@(@('core/input/mouse.c','button',''),@('platform/pc/ps2-mouse.c','packet',''),
    @('platform/pc/ps2-bus.c','bus',$busImports),@('platform/uefi/keyboard.c','pointer',$nativeImports))
foreach($compiler in @($clang,$gcc)){foreach($bits in @(32,64)){foreach($opt in @(0,2)){
    foreach($module in $modules){
        if($bits -eq 32 -and $module[1] -eq 'pointer'){continue}
        $profileFlags=@()
        if($module[1] -eq 'pointer'){$profileFlags=@('-DCLASSICK_PC_MOUSE=1')}
        $name=if($compiler -eq $clang){'clang'}else{'gcc'}
        $object=Join-Path $objects "$name-$bits-O$opt-$($module[1]).o"
        & $compiler @profileFlags "-m$bits" "-O$opt" -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo $module[0]) -o $object
        if($LASTEXITCODE -ne 0){throw 'Mouse object compile failed.'}
        $check=if($module[2] -eq ''){'Check-Freestanding.cmake'}else{'Check-ObjectImports.cmake'}
        & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=$($module[2])" -P (Join-Path $PSScriptRoot $check)
        if($LASTEXITCODE -ne 0){throw 'Mouse object imports failed.'};++$checks
    }
}}}
foreach($target in @('aarch64-none-elf','aarch64_be-none-elf')){foreach($module in $modules){
    if($module[1] -eq 'pointer'){
        $priorPreference=$ErrorActionPreference
        try{
            $ErrorActionPreference='Continue'
            $rejected=@(& $clang "--target=$target" -DCLASSICK_PC_MOUSE=1 -std=c11 -ffreestanding -c (Join-Path $repo $module[0]) -o (Join-Path $objects "$target-must-not-build.o") 2>&1)
        }finally{$ErrorActionPreference=$priorPreference}
        if($LASTEXITCODE -eq 0 -or ($rejected -join "`n") -notmatch 'requires x64 native pointer widths'){throw 'Native pointer ABI did not reject unsupported target.'}
        $global:LASTEXITCODE=0;++$abiRejections;continue
    }
    $profileFlags=@()
    if($module[1] -eq 'pointer'){$profileFlags=@('-DCLASSICK_PC_MOUSE=1')}
    $object=Join-Path $objects "$target-$($module[1]).o"
    & $clang @profileFlags "--target=$target" -O2 -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo $module[0]) -o $object
    if($LASTEXITCODE -ne 0){throw 'Mouse endian compile failed.'}
    $check=if($module[2] -eq ''){'Check-Freestanding.cmake'}else{'Check-ObjectImports.cmake'}
    & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=$($module[2])" -P (Join-Path $PSScriptRoot $check)
    if($LASTEXITCODE -ne 0){throw 'Mouse endian imports failed.'};++$checks
}}
@{specification='SPEC-0040';object_audits=$checks;abi_rejections=$abiRejections;emulator_run=$false} |
    ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'mouse-objects.json') -Encoding utf8
Write-Output "SPEC-0040 mouse object audits: $checks PASS; $abiRejections expected ABI rejections"
