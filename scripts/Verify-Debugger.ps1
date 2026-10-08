# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot, [switch]$Sanitizers)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$BuildRoot = [IO.Path]::GetFullPath($BuildRoot)
& (Join-Path $PSScriptRoot 'Verify-Clocks.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers
& (Join-Path $PSScriptRoot 'Test-Debugger.ps1') -BuildRoot $BuildRoot
