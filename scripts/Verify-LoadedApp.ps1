# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,[switch]$Sanitizers,[string]$Python='python')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
& (Join-Path $PSScriptRoot 'Verify-GuardedBus.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers -Python $Python
& (Join-Path $PSScriptRoot 'Verify-LoadedAppObjects.ps1') -BuildRoot $BuildRoot
Write-Output 'First loaded application full verification PASS; actual boot trial separate.'
