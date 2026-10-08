# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$repoRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$BuildRoot=[IO.Path]::GetFullPath($BuildRoot)
if(Test-Path -LiteralPath $BuildRoot){throw 'Use a fresh EFI output directory.'}
$originalPath=$env:PATH
try {
    & (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
    $compiler=(Get-Command clang).Source; $nm=(Get-Command llvm-nm).Source
    $reader=(Get-Command llvm-readobj).Source; $disassembler=(Get-Command llvm-objdump).Source
    $sources=@('platform/uefi/contract.c','platform/uefi/loader.c','platform/uefi/entry.c',
        'platform/uefi/transition.S','platform/pc/x64/state.c','platform/pc/x64/exceptions.S',
        'core/graphics/surface.c','core/memory/arena.c','platform/pc/framebuffer.c',
        'apps/boot-scene/scene.c','platform/uefi/native.c',
        'platform/pc/acpi.c','platform/pc/pmtimer.c','platform/pc/x64/io.S')
    # SPEC-0009 objects permit only these original project imports.
    $allowedImports=@{8='cs_surface_init';
        9='cs_surface_init,cs_surface_fill,cs_arena_alloc,cs_fb_init,cs_fb_present';
        10='cs_uefi_check_framebuffer,cs_fb_init,cs_arena_init,cs_boot_scene_prepare,cs_boot_scene_draw,cs_uefi_check_map,cs_acpi_find_pm_timer,cs_pmtimer_init,cs_pmtimer_sample,cs_pmtimer_time'}
    $common=@('-m64','-Wall','-Wextra','-Wpedantic','-Werror','-Wconversion','-Wsign-conversion',
        '-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-ident','-mno-red-zone','-mgeneral-regs-only',
        '-fno-asynchronous-unwind-tables','-fno-unwind-tables',"-ffile-prefix-map=$repoRoot=.")
    $link=@('-m64','-nostdlib','-Wl,--entry,cs_uefi_entry,--subsystem,efi_application,--no-insert-timestamp,--no-gc-sections,--strip-all,--build-id=none,--image-base,0x10000000')
    $results=@(); $reference=$null; $referenceMap=$null; $referenceObjects=@(); $release=$null
    foreach($opt in @(0,2)) {
        $twins=@()
        foreach($repeat in @('a','b')) {
            $directory=Join-Path $BuildRoot "clang-64-O$opt-$repeat"
            [void][IO.Directory]::CreateDirectory($directory); $objects=@()
            for($i=0;$i -lt $sources.Count;++$i) {
                $object=Join-Path $directory "$i.o"
                if($sources[$i].EndsWith('.S')){& $compiler -m64 -c (Join-Path $repoRoot $sources[$i]) -o $object}
                else{& $compiler @common -std=c11 "-O$opt" -c (Join-Path $repoRoot $sources[$i]) -o $object}
                if($LASTEXITCODE -ne 0){throw 'Original EFI compile failed.'}
                if($i -lt 2 -or $i -eq 4 -or $i -eq 6 -or $i -eq 7 -or $i -eq 11 -or $i -eq 12) {
                    $check=if($i -eq 1){'Check-LoaderObject.cmake'}else{'Check-Freestanding.cmake'}
                    & cmake "-DNM=$nm" "-DOBJECT=$object" -P (Join-Path $PSScriptRoot $check)
                    if($LASTEXITCODE -ne 0){throw 'EFI object boundary failed.'}
                }
                if($allowedImports.ContainsKey($i)) {
                    & cmake "-DNM=$nm" "-DALLOWED=$($allowedImports[$i])" "-DOBJECT=$object" -P (Join-Path $PSScriptRoot 'Check-ObjectImports.cmake')
                    if($LASTEXITCODE -ne 0){throw 'EFI presenter object boundary failed.'}
                }
                $objects += $object
            }
            $map=Join-Path $directory 'image.map'; $image=Join-Path $directory 'BOOTX64.EFI'
            & $compiler @link "-Wl,-Map,$map" @objects -o $image
            if($LASTEXITCODE -ne 0){throw 'Original EFI link failed.'}
            $audit=& (Join-Path $PSScriptRoot 'Test-UEFIImage.ps1') -Image $image -Map $map -PassThru
            $inspection=@(& $reader --file-headers --coff-imports --coff-basereloc $image)
            if($LASTEXITCODE -ne 0 -or ($inspection -join "`n") -match '(?m)^Import \{' -or
                ($inspection -join "`n") -notmatch 'IMAGE_SUBSYSTEM_EFI_APPLICATION'){throw 'Independent EFI inspection failed.'}
            $inspection | Set-Content -LiteralPath (Join-Path $directory 'headers.txt') -Encoding UTF8
            & $disassembler -d $image | Set-Content -LiteralPath (Join-Path $directory 'disassembly.txt') -Encoding UTF8
            if($LASTEXITCODE -ne 0){throw 'Original EFI disassembly failed.'}
            $twins += $audit
            if($opt -eq 0 -and $repeat -eq 'a'){$reference=$image; $referenceMap=$map; $referenceObjects=$objects; $referenceAudit=$audit}
            if($opt -eq 2 -and $repeat -eq 'a'){$release=$image}
        }
        if($twins[0].sha256 -ne $twins[1].sha256){throw 'EFI optimization-profile twins differ.'}
        $results += [ordered]@{case="clang-64-O$opt";sha256=$twins[0].sha256;size_bytes=$twins[0].size_bytes;
            image_size=$twins[0].image_size;relocations=$twins[0].relocations;status='passed-unloaded'}
        Write-Output "Matching EFI SHA-256 clang-64-O${opt}: $($twins[0].sha256)"
    }
    $controls=Join-Path $BuildRoot 'controls'; [void][IO.Directory]::CreateDirectory($controls)
    $original=[IO.File]::ReadAllBytes($reference); $pe=[BitConverter]::ToUInt32($original,60); $optional=$pe+24
    $mutations=@(
        [pscustomobject]@{name='machine';at=$pe+4;bytes=[byte[]]@(0x4C,1);reason='Machine rejected'},
        [pscustomobject]@{name='subsystem';at=$optional+68;bytes=[byte[]]@(1,0);reason='subsystem rejected'},
        [pscustomobject]@{name='entry';at=$optional+16;bytes=[BitConverter]::GetBytes([uint32]($referenceAudit.entry_rva+1));reason='Entry differs'},
        [pscustomobject]@{name='imports';at=$optional+120;bytes=[byte[]]@(1,0,0,0,20,0,0,0);reason='Imports/runtime'},
        [pscustomobject]@{name='reloc-absent';at=$optional+152;bytes=[byte[]]@(0,0,0,0,0,0,0,0);reason='Relocation directory'},
        [pscustomobject]@{name='reloc-type';at=$referenceAudit.reloc_raw+8;bytes=[byte[]]@(0,0x30);reason='Relocation type'},
        [pscustomobject]@{name='reloc-block';at=$referenceAudit.reloc_raw+4;bytes=[byte[]]@(255,255,255,255);reason='Relocation block'},
        [pscustomobject]@{name='reloc-duplicate';at=$referenceAudit.reloc_raw+10;bytes=[byte[]]@($original[$referenceAudit.reloc_raw+8],$original[$referenceAudit.reloc_raw+9]);reason='Relocation patch'},
        [pscustomobject]@{name='anchor';at=$referenceAudit.anchor_raw;bytes=[BitConverter]::GetBytes([uint64]0);reason='transition anchor'},
        [pscustomobject]@{name='stack';at=$referenceAudit.transition_raw;bytes=[byte[]]@(0x90);reason='Stack transition'},
        [pscustomobject]@{name='halt';at=$referenceAudit.halt_raw+1;bytes=[byte[]]@(0x90);reason='Halt bytes'},
        [pscustomobject]@{name='writable-code';at=$referenceAudit.first_section_header+36;bytes=[byte[]]@(0x20,0,0,0xE0);reason='Section bounds/permissions'},
        [pscustomobject]@{name='gdtr';at=$referenceAudit.install_raw+9;bytes=[byte[]]@(0x90);reason='Descriptor install bytes'},
        [pscustomobject]@{name='tss-selector';at=$referenceAudit.reload_raw+18;bytes=[byte[]]@(0x10);reason='Descriptor reload bytes'},
        [pscustomobject]@{name='vector-zero';at=$referenceAudit.vector_raw;bytes=[byte[]]@(0x90);reason='Vector error placeholder'},
        [pscustomobject]@{name='vector-error';at=$referenceAudit.vector_raw+8*32;bytes=[byte[]]@(0x6A);reason='Vector bytes'},
        [pscustomobject]@{name='vector-255';at=$referenceAudit.vector_raw+255*32+3;bytes=[byte[]]@(0xFE);reason='Vector bytes'},
        [pscustomobject]@{name='fault-claim';at=$referenceAudit.fault_raw+20;bytes=[byte[]]@(0x90);reason='Fault capture bytes'},
        [pscustomobject]@{name='fault-publish';at=$referenceAudit.fault_raw+128;bytes=[byte[]]@(1);reason='Fault publish bytes'},
        [pscustomobject]@{name='active-state';at=$referenceAudit.state_raw;bytes=[byte[]]@(1);reason='Active-state initial'},
        [pscustomobject]@{name='port-read';at=$referenceAudit.inl_raw+2;bytes=[byte[]]@(0x90);reason='Port read bytes'})
    foreach($mutation in $mutations) {
        $bytes=[byte[]]$original.Clone(); [Array]::Copy($mutation.bytes,0,$bytes,[int]$mutation.at,$mutation.bytes.Length)
        $image=Join-Path $controls ($mutation.name+'.EFI'); [IO.File]::WriteAllBytes($image,$bytes); $rejected=$false
        try{& (Join-Path $PSScriptRoot 'Test-UEFIImage.ps1') -Image $image -Map $referenceMap | Out-Null}
        catch{if($_.Exception.Message -notmatch $mutation.reason){throw}; $rejected=$true}
        if(-not $rejected){throw "EFI image control accepted: $($mutation.name)"}
    }
    $previousPreference=$ErrorActionPreference
    try {
        $ErrorActionPreference='Continue'
        $missing=@(& $compiler @link @($referenceObjects | Where-Object {$_ -ne $referenceObjects[3]}) -o (Join-Path $controls 'must-not-link.EFI') 2>&1)
    } finally{$ErrorActionPreference=$previousPreference}
    if($LASTEXITCODE -eq 0 -or ($missing -join "`n") -notmatch 'cs_native_halt|cs_entry_anchor'){throw 'Missing transition control did not reject.'}
    # This deliberate rejected link is a passing control. GitHub's PowerShell
    # launcher propagates LASTEXITCODE even after a successful script return.
    $global:LASTEXITCODE=0
    $missing | Out-File -LiteralPath (Join-Path $controls 'missing-transition.log') -Encoding UTF8
    try {
        $ErrorActionPreference='Continue'
        $missing=@(& $compiler @link @($referenceObjects | Where-Object {$_ -ne $referenceObjects[5]}) -o (Join-Path $controls 'must-not-link-exceptions.EFI') 2>&1)
    } finally{$ErrorActionPreference=$previousPreference}
    if($LASTEXITCODE -eq 0 -or ($missing -join "`n") -notmatch 'cs_x64_install|cs_x64_vector_base'){throw 'Missing exceptions control did not reject.'}
    $global:LASTEXITCODE=0
    $missing | Out-File -LiteralPath (Join-Path $controls 'missing-exceptions.log') -Encoding UTF8
    try {
        $ErrorActionPreference='Continue'
        $missing=@(& $compiler @link @($referenceObjects | Where-Object {$_ -ne $referenceObjects[8]}) -o (Join-Path $controls 'must-not-link-presenter.EFI') 2>&1)
    } finally{$ErrorActionPreference=$previousPreference}
    if($LASTEXITCODE -eq 0 -or ($missing -join "`n") -notmatch 'cs_fb_init|cs_fb_present'){throw 'Missing presenter control did not reject.'}
    $global:LASTEXITCODE=0
    $missing | Out-File -LiteralPath (Join-Path $controls 'missing-presenter.log') -Encoding UTF8
    try {
        $ErrorActionPreference='Continue'
        $missing=@(& $compiler @link @($referenceObjects | Where-Object {$_ -ne $referenceObjects[11]}) -o (Join-Path $controls 'must-not-link-acpi.EFI') 2>&1)
    } finally{$ErrorActionPreference=$previousPreference}
    if($LASTEXITCODE -eq 0 -or ($missing -join "`n") -notmatch 'cs_acpi_find_pm_timer'){throw 'Missing ACPI control did not reject.'}
    $global:LASTEXITCODE=0
    $missing | Out-File -LiteralPath (Join-Path $controls 'missing-acpi.log') -Encoding UTF8
    $payload=Join-Path $BuildRoot 'payload/EFI/BOOT'; [void][IO.Directory]::CreateDirectory($payload)
    $packaged=Join-Path $payload 'BOOTX64.EFI'; Copy-Item -LiteralPath $release -Destination $packaged
    if((Get-FileHash -LiteralPath $packaged -Algorithm SHA256).Hash.ToLowerInvariant() -ne $results[1].sha256){throw 'EFI payload copy differs.'}
    [ordered]@{images=$results;image_rejections=$mutations.Count;missing_transition_controls=1;missing_exceptions_controls=1;missing_presenter_controls=1;missing_acpi_controls=1;
        payload='payload/EFI/BOOT/BOOTX64.EFI';payload_kind='directory-tree-not-disk';loaded=$false} |
        ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $BuildRoot 'efi-results.json') -Encoding UTF8
    Write-Output "EFI inspection PASS: four twin images, $($mutations.Count) corruption rejections, missing transition/exception/presenter/ACPI rejections and original payload tree; no image executed."
} finally{$env:PATH=$originalPath}
