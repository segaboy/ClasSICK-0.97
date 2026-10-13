# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,[switch]$Sanitizers,[string]$Python='python')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$oldPython=$env:CLASSICK_TEST_PYTHON
try {
    $taskPython=(Get-Command $Python -ErrorAction Stop).Source
    $env:CLASSICK_TEST_PYTHON=$taskPython
    & (Join-Path $PSScriptRoot 'Verify-GuestPatches.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers -Python $taskPython
} finally {$env:CLASSICK_TEST_PYTHON=$oldPython}
& (Join-Path $PSScriptRoot 'Verify-ResourceObjects.ps1') -BuildRoot $BuildRoot
