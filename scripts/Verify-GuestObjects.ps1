# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$repo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$objects=Join-Path $BuildRoot 'guest-objects'
if(Test-Path -LiteralPath $objects){throw 'Fresh guest object directory required.'}
[void][IO.Directory]::CreateDirectory($objects)
$originalPath=$env:PATH
try{
    & (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
    $clang=(Get-Command clang).Source;$nm=(Get-Command llvm-nm).Source
    $gcc=Join-Path $repo '.tools/gcc/w64devkit/bin/gcc.exe';$checks=0
    $modules=@(
        @('core/memory/guest.c','memory',''),
        @('core/cpu/m68k-bus.c','cpu','cs_guest_validate,cs_guest_window,cs_guest_read,cs_m68k_aline'),
        @('apps/memory-probe/probe.c','probe','cs_m68k_bus_step'))
    foreach($compiler in @($clang,$gcc)) {foreach($bits in @(32,64)) {foreach($opt in @(0,2)) {
        foreach($module in $modules){
            $tool=if($compiler -eq $clang){'clang'}else{'gcc'}
            $object=Join-Path $objects "$tool-$bits-O$opt-$($module[1]).o"
            & $compiler "-m$bits" -std=c11 "-O$opt" -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo $module[0]) -o $object
            if($LASTEXITCODE -ne 0){throw 'Guest object compile failed.'}
            & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=$($module[2])" -P (Join-Path $PSScriptRoot 'Check-ObjectImports.cmake')
            if($LASTEXITCODE -ne 0){throw 'Guest object boundary failed.'};++$checks
        }
    }}}
    foreach($target in @('aarch64-none-elf','aarch64_be-none-elf')){foreach($module in $modules){
        $object=Join-Path $objects "$target-$($module[1]).o"
        & $clang "--target=$target" -std=c11 -O2 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo $module[0]) -o $object
        if($LASTEXITCODE -ne 0){throw 'Guest endian object compile failed.'}
        & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=$($module[2])" -P (Join-Path $PSScriptRoot 'Check-ObjectImports.cmake')
        if($LASTEXITCODE -ne 0){throw 'Guest endian object boundary failed.'};++$checks
    }}
    $hashes=[ordered]@{}
    foreach($pair in @(@('clang','clang-hosted/debug-a','clang-hosted/debug-b'),@('gcc','gcc-debug-a','gcc-debug-b'))){
        foreach($name in @('guest','bus')){
            $a=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[1]+"/classick_$($name)_tests.exe")) -Algorithm SHA256).Hash.ToLowerInvariant()
            $b=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[2]+"/classick_$($name)_tests.exe")) -Algorithm SHA256).Hash.ToLowerInvariant()
            if($a -ne $b){throw 'Guest hosted twins differ.'};$hashes["$($pair[0])-$name"]=$a
        }
    }
    [ordered]@{specification='SPEC-0022';object_audits=$checks;hosted_twins=$hashes;native_executed=$false} |
        ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'guest-results.json') -Encoding UTF8
}finally{$env:PATH=$originalPath}
