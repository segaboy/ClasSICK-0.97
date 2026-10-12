# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$VBoxManage,[Parameter(Mandatory=$true)][string]$BuildRoot,
    [Parameter(Mandatory=$true)][string]$TrialRoot,[Parameter(Mandatory=$true)][string]$VMName,[switch]$OwnerApproved)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
if(-not $OwnerApproved){throw 'Require owner continuation go-ahead for SPEC-0021.'}
$result=Get-Content -LiteralPath (Join-Path $BuildRoot 'm68k-results.json') -Raw | ConvertFrom-Json
$efi=Get-Content -LiteralPath (Join-Path $BuildRoot 'fault-results.json') -Raw | ConvertFrom-Json
if($result.specification -ne 'SPEC-0021' -or $result.object_audits -ne 20 -or $result.native_executed -or $efi.loaded){throw 'A-line verification admission rejected.'}
if((Get-FileHash -LiteralPath $VBoxManage -Algorithm SHA256).Hash.ToLowerInvariant() -ne '7fda8e54157eb94a5ebd9cea079a9fb8c23ade92ae71e13399dc3431c021a228'){throw 'Exact cleared platform required.'}
$r=$efi.profiles.none
$audit=& (Join-Path $PSScriptRoot 'Test-UEFIImage.ps1') -Image $r.payload -Map $r.map -FaultProbe none -PassThru
if($audit.sha256 -ne '6e2ea128e819923e6f88fd933bf40c7a7c9c7871d18d59c08315976a610269a7'){throw 'A-line image fingerprint rejected.'}
& (Join-Path $PSScriptRoot 'Prepare-VMTrial.ps1') -Candidate SPEC-0021 -VBoxManage $VBoxManage -BuildRoot $BuildRoot -TrialRoot $TrialRoot -VMName $VMName -OwnerApproved
$p=Get-Content -LiteralPath (Join-Path $TrialRoot 'prepared.json') -Raw | ConvertFrom-Json
[ordered]@{contract='SPEC-0021';uuid=$p.uuid;source_revision=$p.source_revision;profiles=@{
    none=@{vdi=$p.vdi;vdi_sha256=$p.vdi_sha256;raw_sha256=$p.raw_sha256;efi_sha256=$p.payload_sha256;symbols=$audit.symbols}}} |
    ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $TrialRoot 'fault-admission.json') -Encoding UTF8
& (Join-Path $PSScriptRoot 'Review-FaultVM.ps1') -VBoxManage $VBoxManage -TrialRoot $TrialRoot -Disk $p.vdi -Serial $p.serial
Write-Output 'A-line image admitted; fresh isolated VM reviewed and powered off.'
