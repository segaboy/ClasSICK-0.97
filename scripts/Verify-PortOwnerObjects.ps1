# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$repo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if(Test-Path -LiteralPath $BuildRoot){throw 'Fresh port owner object directory required.'}
[void][IO.Directory]::CreateDirectory($BuildRoot)
. (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
$clang=(Get-Command clang).Source
$nm=(Get-Command llvm-nm).Source
$gcc=Join-Path $repo '.tools/gcc/w64devkit/bin/gcc.exe'
$allowed='cs_guest_read,cs_guest_window,cs_image_execution_integrity,cs_image_select_integrity,cs_image_select_native,cs_image_select_release,cs_image_select_retire_native,cs_m68k_bus_guard_validate,cs_m68k_bus_step_admitted,cs_qd_select_get,cs_qd_select_set,cs_qd_view_rect,cs_trap_get'
$modules=@(
    @('code-select.c',$allowed),
    @('code-image.c','cs_code_check_layout,cs_code_inspect,cs_guest_read,cs_guest_validate,cs_guest_window,cs_life_admit,cs_life_manager_call,cs_res_external_admit'))
$checks=0
foreach($compiler in @($clang,$gcc)){foreach($bits in @(32,64)){foreach($opt in @(0,2)){foreach($module in $modules){
    $name=if($compiler -eq $clang){'clang'}else{'gcc'}
    $object=Join-Path $BuildRoot "$name-$bits-O$opt-$($module[0]).o"
    & $compiler "-m$bits" "-O$opt" -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo "personality/system1/$($module[0])") -o $object
    if($LASTEXITCODE -ne 0){throw 'Port owner object compile failed.'}
    & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=$($module[1])" -P (Join-Path $repo 'scripts/Check-ObjectImports.cmake')
    if($LASTEXITCODE -ne 0){throw 'Port owner imports/data audit failed.'}
    ++$checks
}}}}
foreach($target in @('aarch64-none-elf','aarch64_be-none-elf')){foreach($module in $modules){
    $object=Join-Path $BuildRoot "$target-$($module[0]).o"
    & $clang "--target=$target" -O2 -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo "personality/system1/$($module[0])") -o $object
    if($LASTEXITCODE -ne 0){throw 'Port owner endian compile failed.'}
    & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=$($module[1])" -P (Join-Path $repo 'scripts/Check-ObjectImports.cmake')
    if($LASTEXITCODE -ne 0){throw 'Port owner endian imports/data audit failed.'}
    ++$checks
}}
@{specification='SPEC-0046';object_audits=$checks;target_executed=$false} |
    ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'manifest.json') -Encoding UTF8
Write-Output "$checks port owner object audits PASS."
