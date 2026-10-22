# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$repo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if(Test-Path -LiteralPath $BuildRoot){throw 'Fresh window boot object directory required.'}
[void][IO.Directory]::CreateDirectory($BuildRoot)
. (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
$clang=(Get-Command clang).Source;$nm=(Get-Command llvm-nm).Source
$gcc=Join-Path $repo '.tools/gcc/w64devkit/bin/gcc.exe'
$audits=@()
$modules=@(@('platform/uefi/window.c','window','cs_uefi_check_framebuffer,cs_uefi_check_map,cs_uefi_check_owned,cs_native_read,cs_acpi_find_pm_timer,cs_acpi_8042,cs_pmtimer_init,cs_pmtimer_sample,cs_arena_init,cs_arena_alloc,cs_pc_window_session_start,cs_pc_window_session_poll,cs_pc_window_session_stop,cs_fb_init'),
    @('platform/uefi/entry.c','entry','cs_x64_inb,cs_x64_outb,cs_x64_inl,cs_x64_tables_init,cs_x64_install,cs_x64_vector_base,cs_native_window_loop,cs_native_halt,cs_entry_anchor,cs_uefi_loader_run'))
foreach($compiler in @($clang,$gcc)){foreach($opt in @(0,2)){foreach($module in $modules){
    $name=if($compiler -eq $clang){'clang'}else{'gcc'}
    $object=Join-Path $BuildRoot "$name-64-O$opt-$($module[1]).o"
    & $compiler -m64 "-O$opt" -DCLASSICK_WINDOW_DRAWING=1 -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -Wframe-larger-than=2048 -fstack-usage -c (Join-Path $repo $module[0]) -o $object
    if($LASTEXITCODE -ne 0){throw 'Window boot object compile failed.'}
    & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=$($module[2])" -P (Join-Path $PSScriptRoot 'Check-ObjectImports.cmake')
    if($LASTEXITCODE -ne 0){throw 'Window boot imports/globals audit failed.'}
    $audits+=@{profile="$name-64-O$opt-$($module[1])";object_bytes=(Get-Item -LiteralPath $object).Length}
}}}
@{specification='SPEC-0062';object_audits=$audits;target_executed=$false;whole_stack_budget_certified=$false} |
    ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $BuildRoot 'window-objects.json') -Encoding utf8
Write-Output "$($audits.Count) window boot object audits PASS;no image executed."
