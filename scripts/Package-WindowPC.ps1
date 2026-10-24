# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,
    [Parameter(Mandatory=$true)][string]$MediaWriter,
    [Parameter(Mandatory=$true)][string]$TwinWriter,[switch]$InputProbe)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$repo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$proof=Get-Content -Raw -LiteralPath (Join-Path $BuildRoot 'efi-results.json') | ConvertFrom-Json
if(-not $proof.window_drawing -or $proof.loaded){throw 'Require audited unloaded window drawing EFI.'}
$isProbe=($null -ne $proof.PSObject.Properties['window_input_probe'] -and [bool]$proof.window_input_probe)
if([bool]$InputProbe -ne [bool]$isProbe){throw 'Input-probe media requires its explicit matching packaging switch.'}
$probeMode=if($null -ne $proof.PSObject.Properties['window_input_probe_mode']){
    [string]$proof.window_input_probe_mode
}elseif($isProbe){'loss'}else{'none'}
if($probeMode -notin @('none','loss','motion') -or ($isProbe -eq ($probeMode -eq 'none'))){throw 'Invalid probe mode metadata.'}
$efi=Join-Path $BuildRoot 'payload/EFI/BOOT/BOOTX64.EFI'
$hash=(Get-FileHash -LiteralPath $efi -Algorithm SHA256).Hash.ToLowerInvariant()
if($hash -ne $proof.images[1].sha256){throw 'EFI payload differs from audited O2 candidate.'}
$disk=Join-Path $BuildRoot 'window-drawing.img'
$twin=Join-Path $BuildRoot 'window-drawing-twin.img'
& $MediaWriter $efi $disk
if($LASTEXITCODE -ne 0){throw 'Window boot media generation failed.'}
& $TwinWriter $efi $twin
if($LASTEXITCODE -ne 0){throw 'Window twin media generation failed.'}
if((Get-FileHash -LiteralPath $disk).Hash -ne (Get-FileHash -LiteralPath $twin).Hash){throw 'Window media writers disagree.'}
$check=@(& (Join-Path $PSScriptRoot 'Test-BootMedia.ps1') -Image $disk -Payload $efi -PassThru)[-1]
$inputs=[ordered]@{}
foreach($path in @('platform/uefi/window.c','platform/uefi/window.h','platform/uefi/entry.c','platform/pc/window-session.c','platform/pc/window-session.h','platform/pc/mono-present.c','platform/pc/mono-present.h','platform/pc/window-input.c','platform/pc/window-input.h','apps/window-scene/input.c','apps/window-scene/scene.c','core/graphics/cursor.c','core/input/pointer.c','core/ui/window.c','apps/product-scene/scene.c','scripts/Verify-UEFIImage.ps1','scripts/Package-WindowPC.ps1','docs/specifications/SPEC-0062-uefi-window-binding.md')){
    $bytes=[Text.Encoding]::UTF8.GetBytes([IO.File]::ReadAllText((Join-Path $repo $path)).Replace("`r`n","`n"))
    $digest=[Security.Cryptography.SHA256]::Create().ComputeHash($bytes)
    $inputs[$path]=($digest | ForEach-Object {$_.ToString('x2')}) -join ''
}
$head=(& git -C $repo rev-parse HEAD).Trim()
if($isProbe){
    foreach($path in @('platform/pc/input-probe.c','platform/pc/input-probe.h','platform/pc/x64/input-probe-halt.S')){
        $bytes=[Text.Encoding]::UTF8.GetBytes([IO.File]::ReadAllText((Join-Path $repo $path)).Replace("`r`n","`n"))
        $digest=[Security.Cryptography.SHA256]::Create().ComputeHash($bytes)
        $inputs[$path]=($digest | ForEach-Object {$_.ToString('x2')}) -join ''
    }
}
$dirty=[bool]@(& git -C $repo diff --name-only HEAD).Count
[ordered]@{specification='SPEC-0062';session_specification='SPEC-0061';diagnostic_input_probe=[bool]$isProbe;input_probe_mode=$probeMode;continuous_preview=$true;source_revision=$head;tracked_source_dirty=$dirty;inputs=$inputs;input_scope='window binding subset;source revision identifies remaining inputs';
    efi_sha256=$hash;media=$check;writer_twins_equal=$true;target_executed=$false;release=$false} |
    ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $BuildRoot 'window-media.json') -Encoding UTF8
Write-Output 'Window native PC media PASS: matching writer images, independent filesystem/payload check; no image executed.'
