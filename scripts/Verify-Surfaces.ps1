# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot, [switch]$Sanitizers)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$BuildRoot = [IO.Path]::GetFullPath($BuildRoot)
if (Test-Path -LiteralPath $BuildRoot) { throw 'Use a fresh output directory; existing output is preserved.' }
$originalPath = $env:PATH
function Build-And-Test([string]$name, [string[]]$extra) {
    $build = Join-Path $BuildRoot $name
    & cmake --preset windows-hosted -S $repoRoot -B $build @extra
    if ($LASTEXITCODE -ne 0) { throw "Configure failed: $name" }
    & cmake --build $build
    if ($LASTEXITCODE -ne 0) { throw "Build failed: $name" }
    & ctest --test-dir $build --output-on-failure --no-tests=error
    if ($LASTEXITCODE -ne 0) { throw "Conformance failed: $name" }
}
try {
    & (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
    $compilerBin = Split-Path -Parent (Get-Command clang).Source
    & clang --version
    if ($LASTEXITCODE -ne 0) { throw 'Compiler unavailable' }
    foreach ($name in @('debug-a','debug-b')) { Build-And-Test $name @() }
    $hashes = @{}
    foreach ($artifact in @('classick_surface_tests.exe')) {
        $first = (Get-FileHash -LiteralPath (Join-Path $BuildRoot ('debug-a/' + $artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
        $second = (Get-FileHash -LiteralPath (Join-Path $BuildRoot ('debug-b/' + $artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($first -ne $second) { throw "Fresh surface hashes differ: $artifact" }
        $hashes[$artifact] = $first
        Write-Output ("Matching SHA-256 {0}: {1}" -f $artifact, $first)
    }
    foreach ($name in @('debug-a','debug-b')) {
        # COFF object timestamps vary; do not claim reproducibility for the archive.
        $hashes["$name/libclassick_surfaces.a"] = (Get-FileHash -LiteralPath (Join-Path $BuildRoot ("$name/libclassick_surfaces.a")) -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    Build-And-Test 'release' @('-DCMAKE_BUILD_TYPE=Release')
    $compiler32 = Join-Path $compilerBin 'i686-w64-mingw32-clang.exe'
    Build-And-Test 'i686' @("-DCMAKE_C_COMPILER=$compiler32")
    # Supplemental compile-only endian/architecture checks; no runner claim.
    foreach ($target in @('aarch64-none-elf','aarch64_be-none-elf')) {
        $object = Join-Path $BuildRoot ($target + '.o')
        & clang "--target=$target" -std=c11 -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -ffreestanding -fno-builtin -fno-stack-protector -O2 -c (Join-Path $repoRoot 'core/graphics/surface.c') -o $object
        if ($LASTEXITCODE -ne 0) { throw "Compile-only check failed: $target" }
        & cmake "-DNM=$(Join-Path $compilerBin 'llvm-nm.exe')" "-DOBJECT=$object" -P (Join-Path $PSScriptRoot 'Check-Freestanding.cmake')
        if ($LASTEXITCODE -ne 0) { throw "Import check failed: $target" }
    }
    if ($Sanitizers) {
        $probe = Join-Path $BuildRoot 'sanitizer-probe.exe'
        & clang -std=c11 -Wall -Wextra -Wpedantic -Werror '-fsanitize=address,undefined' -fno-sanitize-recover=all (Join-Path $repoRoot 'tests/sanitizer-probe.c') -o $probe
        if ($LASTEXITCODE -ne 0) { throw 'Sanitizer support not validated: probe build failed' }
        foreach ($kind in @('address','undefined')) {
            # Windows PowerShell represents redirected native stderr as error records.
            $ErrorActionPreference = 'Continue'
            $diagnostic = (& $probe $kind 2>&1 | Out-String)
            $probeExit = $LASTEXITCODE
            $ErrorActionPreference = 'Stop'
            $diagnostic | Set-Content -LiteralPath (Join-Path $BuildRoot ($kind + '-probe.log')) -Encoding UTF8
            $signature = if ($kind -eq 'address') { 'AddressSanitizer: heap-buffer-overflow' } else { 'runtime error: signed integer overflow' }
            if ($probeExit -eq 0 -or -not $diagnostic.Contains($signature)) {
                throw "Sanitizer support not validated: $kind did not detect its intentional defect"
            }
            Write-Output "Sanitizer detection validated: $kind"
        }
        Build-And-Test 'sanitized' @('-DCLASSICK_SURFACE_SANITIZERS=ON')
    }
    $hashes | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'surface-hashes.json') -Encoding UTF8
    Write-Output 'SPEC-0001 verification PASS. Windows headless contracts and compile/import checks only; no OS boot or historical compatibility claim.'
} finally {
    $env:PATH = $originalPath
    $ErrorActionPreference = 'Stop'
}
