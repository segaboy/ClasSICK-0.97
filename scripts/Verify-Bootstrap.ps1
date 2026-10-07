# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([string]$BuildRoot = '')
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if (-not $BuildRoot) { $BuildRoot = Join-Path $repoRoot 'build\bootstrap-verification' }
$BuildRoot = [IO.Path]::GetFullPath($BuildRoot)
if (Test-Path -LiteralPath $BuildRoot) {
    throw "Use a new empty verification directory. Existing output is preserved: $BuildRoot"
}
$originalPath = $env:PATH
try {
    & (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
    & clang --version
    if ($LASTEXITCODE -ne 0) { throw 'Compiler version check failed' }
    & cmake --version
    if ($LASTEXITCODE -ne 0) { throw 'CMake version check failed' }
    & ninja --version
    if ($LASTEXITCODE -ne 0) { throw 'Ninja version check failed' }
    $hashes = @()
    foreach ($name in @('a','b')) {
        $build = Join-Path $BuildRoot $name
        & cmake --preset windows-hosted -S $repoRoot -B $build
        if ($LASTEXITCODE -ne 0) { throw "Configure failed: $name" }
        & cmake --build $build
        if ($LASTEXITCODE -ne 0) { throw "Build failed: $name" }
        & ctest --test-dir $build --output-on-failure --no-tests=error
        if ($LASTEXITCODE -ne 0) { throw "Smoke test failed: $name" }
        $exe = Join-Path $build 'classick_toolchain_smoke.exe'
        $hashes += (Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    if ($hashes[0] -ne $hashes[1]) { throw 'Fresh build executable hashes differ' }
    Write-Output "Two fresh native hosted builds passed; identical executable SHA-256: $($hashes[0])"
    Write-Output 'This verifies bootstrap tooling only, not an OS boot or Macintosh compatibility.'
} finally { $env:PATH = $originalPath }
