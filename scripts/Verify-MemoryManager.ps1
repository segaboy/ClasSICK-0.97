# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,[switch]$Sanitizers,[string]$Python='python')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
& $Python (Join-Path $PSScriptRoot 'Test-Interfaces.py')
if($LASTEXITCODE -ne 0){throw 'Guest interface registry failed.'}
& (Join-Path $PSScriptRoot 'Verify-GuestMemory.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers
& (Join-Path $PSScriptRoot 'Verify-MMObjects.ps1') -BuildRoot $BuildRoot
Write-Output 'Memory Manager PASS: hosted/standalone verification only; no application/boot image or emulator execution.'
