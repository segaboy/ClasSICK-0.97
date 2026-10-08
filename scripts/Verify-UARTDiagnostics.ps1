# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,[switch]$Sanitizers)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$repoRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$BuildRoot=[IO.Path]::GetFullPath($BuildRoot)
& (Join-Path $PSScriptRoot 'Verify-PS2Keyboard.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers
$hashes=[ordered]@{}
foreach($artifact in @('classick_uart_tests.exe','classick_native_uart_tests.exe')) {
    foreach($pair in @(@('clang','clang-hosted/debug-a','clang-hosted/debug-b'),@('gcc','gcc-debug-a','gcc-debug-b'))) {
        $a=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[1]+'/'+$artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
        $b=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[2]+'/'+$artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
        if($a -ne $b){throw "UART hosted twins differ: $($pair[0]) $artifact"}
        $hashes["$($pair[0]) $artifact"]=$a; Write-Output "Matching UART SHA-256 $($pair[0]) ${artifact}: $a"
    }
}
$originalPath=$env:PATH
try {
    & (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
    $clang=(Get-Command clang).Source; $nm=(Get-Command llvm-nm).Source
    $gccLock=Get-Content -LiteralPath (Join-Path $PSScriptRoot 'gcc-toolchain-lock.json') -Raw | ConvertFrom-Json
    $gcc=Join-Path $repoRoot ('.tools/gcc/'+$gccLock.bin+'/gcc.exe')
    $auditRoot=Join-Path $BuildRoot 'uart-objects'; [void][IO.Directory]::CreateDirectory($auditRoot)
    foreach($tool in @('clang','gcc')) {
        $compiler=if($tool -eq 'clang'){$clang}else{$gcc}
        foreach($bits in @(32,64)) { foreach($opt in @(0,2)) {
            $modules=if($bits -eq 64){@('platform/pc/uart.c','platform/uefi/uart.c')}else{@('platform/pc/uart.c')}
            foreach($module in $modules) {
                $native=$module -eq 'platform/uefi/uart.c'
                $name=if($native){'native'}else{'portable'}
                $object=Join-Path $auditRoot "$tool-$bits-O$opt-$name.o"
                & $compiler "-m$bits" -std=c11 -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -ffreestanding -fno-builtin -fno-stack-protector -Wframe-larger-than=2048 "-O$opt" -c (Join-Path $repoRoot $module) -o $object
                if($LASTEXITCODE -ne 0){throw "UART object compile failed: $tool $bits O$opt $name"}
                $allowed=if($native){'cs_uart_begin,cs_uart_enqueue,cs_uart_finish,cs_uart_poll,cs_native_keyboard_observed,cs_acpi_find_pm_timer,cs_native_read,cs_pmtimer_init,cs_pmtimer_sample'}else{''}
                & cmake "-DNM=$nm" "-DALLOWED=$allowed" "-DOBJECT=$object" -P (Join-Path $PSScriptRoot 'Check-ObjectImports.cmake')
                if($LASTEXITCODE -ne 0){throw "UART object import audit failed: $tool $bits O$opt $name"}
            }
        } }
    }
    foreach($target in @('aarch64-none-elf','aarch64_be-none-elf')) {
        $object=Join-Path $BuildRoot ($target+'-uart.o')
        & $clang "--target=$target" -std=c11 -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -ffreestanding -fno-builtin -fno-stack-protector -O2 -c (Join-Path $repoRoot 'platform/pc/uart.c') -o $object
        if($LASTEXITCODE -ne 0){throw "UART endian compile failed: $target"}
        & cmake "-DNM=$nm" "-DOBJECT=$object" -P (Join-Path $PSScriptRoot 'Check-Freestanding.cmake')
        if($LASTEXITCODE -ne 0){throw "UART endian audit failed: $target"}
    }
} finally { $env:PATH=$originalPath }
$hashes | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'uart-diagnostics-hashes.json') -Encoding UTF8
Write-Output 'UART diagnostics PASS: hosted register/queue/bounds/transport/keyboard tests, endian objects and unloaded EFI audits; no native serial delivery or boot observation.'
