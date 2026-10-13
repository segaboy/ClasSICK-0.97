# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$repo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$objects=Join-Path $BuildRoot 'mm-objects'
if(Test-Path -LiteralPath $objects){throw 'Fresh manager object directory required.'}
[void][IO.Directory]::CreateDirectory($objects)
$originalPath=$env:PATH
try{
    & (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
    $clang=(Get-Command clang).Source;$nm=(Get-Command llvm-nm).Source
    $gcc=Join-Path $repo '.tools/gcc/w64devkit/bin/gcc.exe';$checks=0
    $modules=@(
        @('core/memory/manager.c','core','cs_guest_validate,cs_guest_window'),
        @('personality/system1/memory-registers.c','register','cs_trap_decode,cs_mm_execute'),
        @('apps/heap-probe/probe.c','probe','cs_mm_register_call,cs_guest_read,cs_guest_write'))
    foreach($compiler in @($clang,$gcc)) {foreach($bits in @(32,64)) {foreach($opt in @(0,2)) {
        foreach($module in $modules){
            $tool=if($compiler -eq $clang){'clang'}else{'gcc'}
            $object=Join-Path $objects "$tool-$bits-O$opt-$($module[1]).o"
            & $compiler "-m$bits" -std=c11 "-O$opt" -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo $module[0]) -o $object
            if($LASTEXITCODE -ne 0){throw 'Manager object compile failed.'}
            & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=$($module[2])" -P (Join-Path $PSScriptRoot 'Check-ObjectImports.cmake')
            if($LASTEXITCODE -ne 0){throw 'Manager object boundary failed.'};++$checks
        }
    }}}
    foreach($target in @('aarch64-none-elf','aarch64_be-none-elf')){foreach($module in $modules){
        $object=Join-Path $objects "$target-$($module[1]).o"
        & $clang "--target=$target" -std=c11 -O2 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo $module[0]) -o $object
        if($LASTEXITCODE -ne 0){throw 'Manager endian object compile failed.'}
        & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=$($module[2])" -P (Join-Path $PSScriptRoot 'Check-ObjectImports.cmake')
        if($LASTEXITCODE -ne 0){throw 'Manager endian object boundary failed.'};++$checks
    }}
    $hashes=[ordered]@{}
    foreach($pair in @(@('clang','clang-hosted/debug-a','clang-hosted/debug-b'),@('gcc','gcc-debug-a','gcc-debug-b'))){
        $a=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[1]+'/classick_mm_tests.exe')) -Algorithm SHA256).Hash.ToLowerInvariant()
        $b=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[2]+'/classick_mm_tests.exe')) -Algorithm SHA256).Hash.ToLowerInvariant()
        if($a -ne $b){throw 'Manager hosted twins differ.'};$hashes[$pair[0]]=$a
        Write-Output "Matching manager executable SHA-256 $($pair[0]): $a"
    }
    [ordered]@{specification='SPEC-0023';object_audits=$checks;hosted_twins=$hashes;native_executed=$false} |
        ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'mm-results.json') -Encoding UTF8
}finally{$env:PATH=$originalPath}
