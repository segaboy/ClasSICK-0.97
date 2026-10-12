# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,[switch]$Sanitizers)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
& (Join-Path $PSScriptRoot 'Verify-ALine.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers
& (Join-Path $PSScriptRoot 'Verify-GuestObjects.ps1') -BuildRoot $BuildRoot
Write-Output 'Checked guest memory PASS: hosted behavior and standalone object audits; new native program execution is not claimed.'
