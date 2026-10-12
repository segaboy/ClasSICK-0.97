# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,[switch]$Sanitizers)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
& (Join-Path $PSScriptRoot 'Test-ServiceObservation.ps1')
& (Join-Path $PSScriptRoot 'Verify-FaultCheckpoint.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers
& (Join-Path $PSScriptRoot 'Verify-ServiceObjects.ps1') -BuildRoot $BuildRoot
& (Join-Path $PSScriptRoot 'Verify-MacROM.ps1') -BuildRoot (Join-Path $BuildRoot 'mac-hosted')
Write-Output 'Shared services verification PASS: hosted, original EFI/ROM dependencies and unloaded audits; no native trial.'
