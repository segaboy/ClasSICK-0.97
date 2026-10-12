# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,[switch]$Sanitizers)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
& (Join-Path $PSScriptRoot 'Test-FaultObservation.ps1')
& (Join-Path $PSScriptRoot 'Verify-BootMedia.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers
$profiles=[ordered]@{}
foreach($profile in @('none','ud','gp')){
    $efi=if($profile -eq 'none'){Join-Path $BuildRoot 'efi'}else{Join-Path $BuildRoot ('fault-'+$profile)}
    if($profile -ne 'none'){
        & (Join-Path $PSScriptRoot 'Verify-UEFIImage.ps1') -BuildRoot $efi -FaultProbe $profile
    }
    $payload=Join-Path $efi 'payload/EFI/BOOT/BOOTX64.EFI'
    $map=Join-Path $efi 'clang-64-O2-a/image.map'
    $audit=& (Join-Path $PSScriptRoot 'Test-UEFIImage.ps1') -Image $payload -Map $map -FaultProbe $profile -PassThru
    $image=if($profile -eq 'none'){Join-Path $BuildRoot 'media/clang-hosted-debug-a.img'}else{Join-Path $efi 'disk.img'}
    if($profile -ne 'none'){
        & (Join-Path $BuildRoot 'clang-hosted/debug-a/classick_boot_media.exe') $payload $image
        if($LASTEXITCODE -ne 0){throw 'Fault media writer failed.'}
    }
    $media=@(& (Join-Path $PSScriptRoot 'Test-BootMedia.ps1') -Image $image -Payload $payload -PassThru)[-1]
    $profiles[$profile]=[ordered]@{efi_sha256=$audit.sha256;efi_bytes=$audit.size_bytes;
        image_size=$audit.image_size;raw_sha256=$media.image_sha256;symbols=$audit.symbols;
        image=$image;payload=$payload;map=$map}
}
[ordered]@{contract='SPEC-0019';profiles=$profiles;loaded=$false} |
    ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $BuildRoot 'fault-results.json') -Encoding UTF8
Write-Output 'Fault checkpoint build PASS: normal/UD/GP twins, image controls and own media; unloaded.'
