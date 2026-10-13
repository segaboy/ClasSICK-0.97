# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,[switch]$Sanitizers,[string]$Python='python')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
& (Join-Path $PSScriptRoot 'Verify-MemoryManager.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers -Python $Python
$repo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$objects=Join-Path $BuildRoot 'memory-trap-objects'
if(Test-Path -LiteralPath $objects){throw 'Fresh memory-trap object directory required.'}
[void][IO.Directory]::CreateDirectory($objects)
$oldPath=$env:PATH
try {
    & (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
    $clang=(Get-Command clang).Source;$nm=(Get-Command llvm-nm).Source
    $gcc=Join-Path $repo '.tools/gcc/w64devkit/bin/gcc.exe'
    $allowed='cs_trap_get,cs_trap_decode,cs_trap_call,cs_trap_direct,cs_mm_execute,cs_mm_register_call,cs_m68k_bus_step_options'
    $checks=0
    foreach($compiler in @($clang,$gcc)){foreach($bits in @(32,64)){foreach($opt in @(0,2)){
        $name=if($compiler -eq $clang){'clang'}else{'gcc'}
        $obj=Join-Path $objects "$name-$bits-O$opt.o"
        & $compiler "-m$bits" "-O$opt" -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo 'personality/system1/memory-traps.c') -o $obj
        if($LASTEXITCODE -ne 0){throw 'Memory trap object compile failed.'}
        & cmake "-DNM=$nm" "-DOBJECT=$obj" "-DALLOWED=$allowed" -P (Join-Path $repo 'scripts/Check-ObjectImports.cmake')
        if($LASTEXITCODE -ne 0){throw 'Memory trap object imports failed.'};++$checks
    }}}
    foreach($target in @('aarch64-none-elf','aarch64_be-none-elf')){
        $obj=Join-Path $objects "$target.o"
        & $clang "--target=$target" -O2 -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo 'personality/system1/memory-traps.c') -o $obj
        if($LASTEXITCODE -ne 0){throw 'Memory trap endian compile failed.'}
        & cmake "-DNM=$nm" "-DOBJECT=$obj" "-DALLOWED=$allowed" -P (Join-Path $repo 'scripts/Check-ObjectImports.cmake')
        if($LASTEXITCODE -ne 0){throw 'Memory trap endian imports failed.'};++$checks
    }
    $hashes=[ordered]@{}
    foreach($pair in @(@('clang','clang-hosted/debug-a','clang-hosted/debug-b'),@('gcc','gcc-debug-a','gcc-debug-b'))){
        $a=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[1]+'/classick_memory_traps_tests.exe')) -Algorithm SHA256).Hash.ToLowerInvariant()
        $b=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[2]+'/classick_memory_traps_tests.exe')) -Algorithm SHA256).Hash.ToLowerInvariant()
        if($a -ne $b){throw 'Memory trap hosted twins differ.'};$hashes[$pair[0]]=$a
    }
    [ordered]@{specification='SPEC-0025';object_audits=$checks;hosted_twins=$hashes;native_executed=$false} |
        ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'memory-trap-results.json') -Encoding UTF8
}finally{$env:PATH=$oldPath}
Write-Output 'Guest memory traps PASS: original interpreted heap program and standalone audits; no application callback or native trial.'
