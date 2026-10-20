# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$repo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if(Test-Path -LiteralPath $BuildRoot){throw 'Fresh window object directory required.'}
[void][IO.Directory]::CreateDirectory($BuildRoot)
. (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
$clang=(Get-Command clang).Source
$nm=(Get-Command llvm-nm).Source
$gcc=Join-Path $repo '.tools/gcc/w64devkit/bin/gcc.exe'
$checks=0
foreach($source in @('core/ui/window.c')) {
$prefix='window'
foreach($compiler in @($clang,$gcc)){foreach($bits in @(32,64)){foreach($opt in @(0,2)){
    $name=if($compiler -eq $clang){'clang'}else{'gcc'}
    $object=Join-Path $BuildRoot "$prefix-$name-$bits-O$opt.o"
    & $compiler "-m$bits" "-O$opt" -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo $source) -o $object
    if($LASTEXITCODE -ne 0){throw 'Window object compile failed.'}
    & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=" -P (Join-Path $repo 'scripts/Check-ObjectImports.cmake')
    if($LASTEXITCODE -ne 0){throw 'Window imports/data audit failed.'}
    ++$checks
}}}
foreach($target in @('aarch64-none-elf','aarch64_be-none-elf')){
    $object=Join-Path $BuildRoot "$prefix-$target.o"
    & $clang "--target=$target" -O2 -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo $source) -o $object
    if($LASTEXITCODE -ne 0){throw 'Window endian compile failed.'}
    & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=" -P (Join-Path $repo 'scripts/Check-ObjectImports.cmake')
    if($LASTEXITCODE -ne 0){throw 'Window endian imports/data audit failed.'}
    ++$checks
}
$twins=@()
foreach($twin in @('a','b')){
    $object=Join-Path $BuildRoot "$prefix-efi-$twin.o"
    & $clang -m64 -O2 -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -fno-ident -mno-red-zone -mgeneral-regs-only -Wframe-larger-than=2048 -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -c (Join-Path $repo $source) -o $object
    if($LASTEXITCODE -ne 0){throw 'Window EFI-ABI compile failed.'}
    & cmake "-DNM=$nm" "-DOBJECT=$object" "-DALLOWED=" -P (Join-Path $PSScriptRoot 'Check-ObjectImports.cmake')
    if($LASTEXITCODE -ne 0){throw 'Window EFI-ABI audit failed.'}
    $twins+=((Get-FileHash -LiteralPath $object -Algorithm SHA256).Hash.ToLowerInvariant())
    ++$checks
}
if($twins[0] -ne $twins[1]){throw 'Window EFI-ABI object twins differ.'}
}
@{efi_abi_object_twins_equal=$true;efi_image_changed=$false;specifications=@('SPEC-0053');object_audits=$checks;target_executed=$false} |
    ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'manifest.json') -Encoding UTF8
Write-Output "$checks window object audits PASS."
