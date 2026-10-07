# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot, [switch]$Sanitizers)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$BuildRoot = [IO.Path]::GetFullPath($BuildRoot)
# Reuse the established build matrix and validated sanitizer controls. CTest now
# includes the conversion/GDI/hidden-window suites alongside unchanged core tests.
& (Join-Path $PSScriptRoot 'Verify-Surfaces.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers
$hashes = @{}
foreach ($artifact in @('classick_presentation_tests.exe','classick_surface_demo.exe')) {
    $first = (Get-FileHash -LiteralPath (Join-Path $BuildRoot ('debug-a/' + $artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
    $second = (Get-FileHash -LiteralPath (Join-Path $BuildRoot ('debug-b/' + $artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($first -ne $second) { throw "Fresh presentation hashes differ: $artifact" }
    $hashes[$artifact] = $first
    Write-Output ("Matching SHA-256 {0}: {1}" -f $artifact, $first)
}
$hashes | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $BuildRoot 'presentation-hashes.json') -Encoding UTF8
Write-Output 'SPEC-0002 verification PASS. Windows conversion/GDI/hidden-window contracts; manual visible review remains separate. No B1 or OS boot claim.'
