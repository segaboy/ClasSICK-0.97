# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$VBoxManage,[Parameter(Mandatory=$true)][string]$BuildRoot,
    [Parameter(Mandatory=$true)][string]$TrialRoot,[Parameter(Mandatory=$true)][string]$VMName,[switch]$OwnerApproved)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
if(-not $OwnerApproved){throw 'Require owner approval for SPEC-0019.'}
$results=Get-Content -LiteralPath (Join-Path $BuildRoot 'fault-results.json') -Raw | ConvertFrom-Json
if($results.contract -ne 'SPEC-0019' -or $results.loaded -or
    (Get-FileHash -LiteralPath $VBoxManage -Algorithm SHA256).Hash.ToLowerInvariant() -ne '7fda8e54157eb94a5ebd9cea079a9fb8c23ade92ae71e13399dc3431c021a228'){
    throw 'Require exact cleared platform and unloaded verification.'
}
foreach($profile in @('none','ud','gp')){
    $r=$results.profiles.$profile
    $pins=@{none='95da0042afb44743bf14307cca866ac26b381cf7435c466ee04231282abcc5fe';
        ud='fc06905f718d6ff6db06a3e37254c6fc3abc337e6de247c9fca9a28a911eb11a';
        gp='58248ad62ce298f404622f05edae0a4e1eb5ecd6be589812058d11b8ebe4df7e'}
    $audit=& (Join-Path $PSScriptRoot 'Test-UEFIImage.ps1') -Image $r.payload -Map $r.map -FaultProbe $profile -PassThru
    $media=@(& (Join-Path $PSScriptRoot 'Test-BootMedia.ps1') -Image $r.image -Payload $r.payload -PassThru)[-1]
    if($audit.sha256 -ne $pins[$profile] -or $audit.sha256 -ne $r.efi_sha256 -or $media.image_sha256 -ne $r.raw_sha256){throw 'Fault profile fingerprint rejected.'}
}
& (Join-Path $PSScriptRoot 'Prepare-VMTrial.ps1') -Candidate SPEC-0019 -VBoxManage $VBoxManage -BuildRoot $BuildRoot -TrialRoot $TrialRoot -VMName $VMName -OwnerApproved
$p=Get-Content -LiteralPath (Join-Path $TrialRoot 'prepared.json') -Raw | ConvertFrom-Json
$profiles=[ordered]@{}
foreach($profile in @('none','ud','gp')){
    $r=$results.profiles.$profile
    $vdi=if($profile -eq 'none'){$p.vdi}else{Join-Path $TrialRoot ($profile+'.vdi')}
    if($profile -ne 'none'){
        & $VBoxManage convertfromraw $r.image $vdi --format VDI --variant Fixed --uuid ([Guid]::NewGuid().ToString())
        if($LASTEXITCODE -ne 0){throw 'Owned fault disk conversion failed.'}
        $roundtrip=Join-Path $TrialRoot ($profile+'-roundtrip.img')
        & $VBoxManage clonemedium disk $vdi $roundtrip --format RAW
        if($LASTEXITCODE -ne 0 -or (Get-FileHash -LiteralPath $roundtrip -Algorithm SHA256).Hash.ToLowerInvariant() -ne $r.raw_sha256){throw 'Owned fault disk round trip failed.'}
    }
    $profiles[$profile]=[ordered]@{vdi=$vdi;vdi_sha256=(Get-FileHash -LiteralPath $vdi -Algorithm SHA256).Hash.ToLowerInvariant();
        raw_sha256=$r.raw_sha256;efi_sha256=$r.efi_sha256;symbols=$r.symbols}
}
[ordered]@{contract='SPEC-0019';uuid=$p.uuid;source_revision=$p.source_revision;profiles=$profiles} |
    ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $TrialRoot 'fault-admission.json') -Encoding UTF8
& (Join-Path $PSScriptRoot 'Review-FaultVM.ps1') -VBoxManage $VBoxManage -TrialRoot $TrialRoot -Disk $p.vdi -Serial $p.serial
Write-Output 'Three owned media admitted with raw round trips; fresh reviewed VM is powered off.'
