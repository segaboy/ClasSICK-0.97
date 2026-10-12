# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$repo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$objects=Join-Path $BuildRoot 'service-objects'
if(Test-Path -LiteralPath $objects){throw 'Fresh service object directory required.'}
[void][IO.Directory]::CreateDirectory($objects)
$originalPath=$env:PATH
try{
    & (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
    $clang=(Get-Command clang).Source;$nm=(Get-Command llvm-nm).Source
    $gcc=Join-Path $repo '.tools/gcc/w64devkit/bin/gcc.exe';$checks=0
    foreach($compiler in @($clang,$gcc)) { foreach($bits in @(32,64)) { foreach($opt in @(0,2)) {
        foreach($module in @('core/services/traps.c','apps/service-probe/probe.c')){
            $name=if($module -match 'traps'){'core'}else{'probe'}
            $tool=if($compiler -eq $clang){'clang'}else{'gcc'}
            $object=Join-Path $objects "$tool-$bits-O$opt-$name.o"
            & $compiler "-m$bits" -std=c11 "-O$opt" -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo $module) -o $object
            if($LASTEXITCODE -ne 0){throw 'Service object compile failed.'}
            $allowed=if($name -eq 'core'){''}else{'cs_trap_init,cs_trap_call,cs_trap_direct'}
            & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=$allowed" -P (Join-Path $PSScriptRoot 'Check-ObjectImports.cmake')
            if($LASTEXITCODE -ne 0){throw 'Service object boundary failed.'};++$checks
        }
    } } }
    foreach($target in @('aarch64-none-elf','aarch64_be-none-elf')){
        foreach($module in @('core/services/traps.c','apps/service-probe/probe.c')){
            $name=if($module -match 'traps'){'core'}else{'probe'}
            $object=Join-Path $objects "$target-$name.o"
            & $clang "--target=$target" -std=c11 -O2 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo $module) -o $object
            if($LASTEXITCODE -ne 0){throw 'Service endian object compile failed.'}
            $allowed=if($name -eq 'core'){''}else{'cs_trap_init,cs_trap_call,cs_trap_direct'}
            & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=$allowed" -P (Join-Path $PSScriptRoot 'Check-ObjectImports.cmake')
            if($LASTEXITCODE -ne 0){throw 'Service endian object boundary failed.'};++$checks
        }
    }
    $hashes=[ordered]@{}
    foreach($pair in @(@('clang','clang-hosted/debug-a','clang-hosted/debug-b'),@('gcc','gcc-debug-a','gcc-debug-b'))){
        $a=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[1]+'/classick_service_tests.exe')) -Algorithm SHA256).Hash.ToLowerInvariant()
        $b=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[2]+'/classick_service_tests.exe')) -Algorithm SHA256).Hash.ToLowerInvariant()
        if($a -ne $b){throw 'Shared-service hosted twins differ.'};$hashes[$pair[0]]=$a
        Write-Output "Matching service executable SHA-256 $($pair[0]): $a"
    }
    [ordered]@{specification='SPEC-0020';object_audits=$checks;hosted_twins=$hashes;native_executed=$false} |
        ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'service-results.json') -Encoding UTF8
}finally{$env:PATH=$originalPath}
