# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,[switch]$ObjectsOnly)
$ErrorActionPreference='Stop'
$repo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$objects=Join-Path $BuildRoot 'codepreflight-objects'
if(Test-Path -LiteralPath $objects){throw 'Fresh codepreflight object directory required.'}
[void][IO.Directory]::CreateDirectory($objects)
$oldPath=$env:PATH
try {
    & (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
    $clang=(Get-Command clang).Source;$nm=(Get-Command llvm-nm).Source
    $gcc=Join-Path $repo '.tools/gcc/w64devkit/bin/gcc.exe';$checks=0
    $allowed='cs_guest_window,cs_life_admit,cs_rf_open,cs_rf_find'
    function Audit-CodePreflight([string]$object){
        & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=$allowed" -P (Join-Path $repo 'scripts/Check-ObjectImports.cmake')
        if($LASTEXITCODE -ne 0){throw 'CodePreflight object imports failed.'}
        $defined=@(& $nm --defined-only --format=posix $object)
        if($LASTEXITCODE -ne 0 -or ($defined -join "`n") -match 'cs_code_test_fault'){
            throw 'Production codepreflight exposes injection or cannot be inspected.'
        }
    }
    foreach($compiler in @($clang,$gcc)){foreach($bits in @(32,64)){foreach($opt in @(0,2)){
        $name=if($compiler -eq $clang){'clang'}else{'gcc'}
        $object=Join-Path $objects "$name-$bits-O$opt.o"
        & $compiler "-m$bits" "-O$opt" -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo 'personality/system1/code-preflight.c') -o $object
        if($LASTEXITCODE -ne 0){throw 'CodePreflight object compilation failed.'}
        Audit-CodePreflight $object;++$checks
    }}}
    foreach($target in @('aarch64-none-elf','aarch64_be-none-elf')){
        $object=Join-Path $objects "$target.o"
        & $clang "--target=$target" -O2 -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo 'personality/system1/code-preflight.c') -o $object
        if($LASTEXITCODE -ne 0){throw 'CodePreflight endian compile failed.'}
        Audit-CodePreflight $object;++$checks
    }
    if($ObjectsOnly){Write-Output 'Ten codepreflight object audits PASS.';return}
    $hashes=[ordered]@{}
    foreach($pair in @(@('clang','clang-hosted/debug-a','clang-hosted/debug-b'),@('gcc','gcc-debug-a','gcc-debug-b'))){
        $a=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[1]+'/classick_codepreflight_tests.exe')) -Algorithm SHA256).Hash.ToLowerInvariant()
        $b=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[2]+'/classick_codepreflight_tests.exe')) -Algorithm SHA256).Hash.ToLowerInvariant()
        if($a -ne $b){throw 'CodePreflight executable twins differ.'};$hashes[$pair[0]]=$a
    }
    [ordered]@{specification='SPEC-0032';object_audits=$checks;hosted_twins=$hashes;native_executed=$false} |
        ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'codepreflight-results.json') -Encoding UTF8
} finally {$env:PATH=$oldPath}
Write-Output 'Hosted CODE preflight verification PASS; no application or emulator trial.'
