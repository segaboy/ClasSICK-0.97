# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,
    [Parameter(Mandatory=$true)][string]$MediaWriter,
    [Parameter(Mandatory=$true)][string]$TwinWriter)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$repo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$proof=Get-Content -Raw -LiteralPath (Join-Path $BuildRoot 'efi-results.json') | ConvertFrom-Json
if(-not $proof.outline_drawing -or $proof.loaded){throw 'Require audited unloaded outline drawing EFI.'}
$efi=Join-Path $BuildRoot 'payload/EFI/BOOT/BOOTX64.EFI'
$hash=(Get-FileHash -LiteralPath $efi -Algorithm SHA256).Hash.ToLowerInvariant()
if($hash -ne $proof.images[1].sha256){throw 'EFI payload differs from audited O2 candidate.'}
$disk=Join-Path $BuildRoot 'outline-drawing.img'
$twin=Join-Path $BuildRoot 'outline-drawing-twin.img'
& $MediaWriter $efi $disk
if($LASTEXITCODE -ne 0){throw 'Outline boot media generation failed.'}
& $TwinWriter $efi $twin
if($LASTEXITCODE -ne 0){throw 'Outline twin media generation failed.'}
if((Get-FileHash -LiteralPath $disk).Hash -ne (Get-FileHash -LiteralPath $twin).Hash){throw 'Outline media writers disagree.'}
$check=@(& (Join-Path $PSScriptRoot 'Test-BootMedia.ps1') -Image $disk -Payload $efi -PassThru)[-1]
$inputs=[ordered]@{}
foreach($path in @('apps/outline-drawing/workload.c','apps/outline-drawing/workload.h','apps/boot-scene/scene.c','apps/boot-scene/scene.h',
    'personality/system1/quickdraw-outline.c','personality/system1/quickdraw-outline.h','personality/system1/quickdraw-select.c','personality/system1/quickdraw-select.h','personality/system1/quickdraw-view.c','personality/system1/quickdraw-view.h','personality/system1/quickdraw-rects.c','personality/system1/quickdraw-rects.h',
    'apps/product-scene/scene.c','apps/product-scene/scene.h','scripts/Verify-UEFIImage.ps1','scripts/Package-OutlineDrawingPC.ps1','docs/specifications/SPEC-0049-paired-native-outline.md')){
    $bytes=[Text.Encoding]::UTF8.GetBytes([IO.File]::ReadAllText((Join-Path $repo $path)).Replace("`r`n","`n"))
    $digest=[Security.Cryptography.SHA256]::Create().ComputeHash($bytes)
    $inputs[$path]=($digest | ForEach-Object {$_.ToString('x2')}) -join ''
}
$head=(& git -C $repo rev-parse HEAD).Trim()
$dirty=[bool]@(& git -C $repo diff --name-only HEAD).Count
[ordered]@{specification='SPEC-0049';source_revision=$head;tracked_source_dirty=$dirty;inputs=$inputs;input_scope='outline-workload subset; source revision identifies remaining inputs';
    efi_sha256=$hash;media=$check;writer_twins_equal=$true;target_executed=$false;release=$false} |
    ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $BuildRoot 'outline-media.json') -Encoding UTF8
Write-Output 'Outline native PC media PASS: matching writer images, independent filesystem/payload check; no image executed.'
