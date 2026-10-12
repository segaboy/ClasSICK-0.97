# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,[switch]$Sanitizers)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
& (Join-Path $PSScriptRoot 'Test-ALineObservation.ps1')
& (Join-Path $PSScriptRoot 'Verify-SharedServices.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers
& (Join-Path $PSScriptRoot 'Verify-M68KObjects.ps1') -BuildRoot $BuildRoot
Write-Output 'A-line boundary verification PASS: hosted and unloaded native image checks; native trial separate.'
