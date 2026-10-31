# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,
    [ValidateSet('none','ud','gp')][string]$FaultProbe='none',[switch]$SelectedDrawing,[switch]$OutlineDrawing,[switch]$WindowDrawing,[switch]$WindowInputProbe,
    [ValidateSet('loss','motion','motion-later','raw-motion','raw-motion-later','raw-report-later','raw-sequence','raw-sequence-first','timeline')][string]$WindowInputProbeMode='loss')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
if(([int][bool]$SelectedDrawing+[int][bool]$OutlineDrawing+[int][bool]$WindowDrawing) -gt 1){throw 'Select only one drawing profile.'}
if($WindowInputProbe -and (-not $WindowDrawing -or $FaultProbe -ne 'none')){throw 'Input probe requires WindowDrawing with no fault probe.'}
if($WindowInputProbeMode -ne 'loss' -and -not $WindowInputProbe){throw 'Motion mode requires the explicit input-probe switch.'}
$repoRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$BuildRoot=[IO.Path]::GetFullPath($BuildRoot)
if(Test-Path -LiteralPath $BuildRoot){throw 'Use a fresh EFI output directory.'}
$originalPath=$env:PATH
try {
    & (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
    $compiler=(Get-Command clang).Source; $nm=(Get-Command llvm-nm).Source
    $probeNumber=@{none=0;ud=6;gp=13}[$FaultProbe]
    $reader=(Get-Command llvm-readobj).Source; $disassembler=(Get-Command llvm-objdump).Source
    $sources=@('platform/uefi/contract.c','platform/uefi/loader.c','platform/uefi/entry.c',
        'platform/uefi/transition.S','platform/pc/x64/state.c','platform/pc/x64/exceptions.S',
        'core/graphics/surface.c','core/memory/arena.c','platform/pc/framebuffer.c',
        'apps/boot-scene/scene.c','platform/uefi/native.c',
        'platform/pc/acpi.c','platform/pc/pmtimer.c','platform/pc/x64/io.S',
        'platform/pc/ps2.c','platform/uefi/keyboard.c','core/input/input.c','platform/pc/x64/keyboard-io.S',
        'platform/pc/uart.c','platform/uefi/uart.c','core/services/traps.c','apps/service-probe/probe.c',
        'core/cpu/m68k.c','apps/aline-probe/probe.c',
        'apps/product-scene/scene.c','apps/drawing-rom-app/probe.c','apps/drawing-rom-app/scene.c',
        'apps/loaded-rom-app/probe.c','apps/loaded-rom-app/scene.c','apps/rom-scene/scene.c',
        'core/memory/guest.c','core/memory/manager.c','core/memory/lifecycle.c',
        'formats/resourcefork/reader.c','personality/system1/code-preflight.c',
        'personality/system1/resources.c','personality/system1/code-image.c',
        'personality/system1/code-run.c','personality/system1/quickdraw-rects.c','core/cpu/m68k-bus.c','core/input/mouse.c','platform/pc/ps2-mouse.c','platform/pc/ps2-bus.c','personality/system1/quickdraw-view.c','apps/native-drawing/workload.c')
    # SPEC-0009 objects permit only these original project imports.
    $allowedImports=@{8='cs_surface_init';
        9='cs_surface_init,cs_arena_alloc,cs_fb_init,cs_fb_present,cs_product_scene,cs_service_probe,cs_native_draw';
        10='cs_uefi_check_framebuffer,cs_fb_init,cs_arena_init,cs_boot_scene_prepare,cs_boot_scene_draw,cs_uefi_check_map,cs_acpi_find_pm_timer,cs_pmtimer_init,cs_pmtimer_sample,cs_pmtimer_time';
        14='cs_input_push';
        15='cs_ps2_bus_begin,cs_ps2_bus_poll,cs_native_read,cs_uefi_check_map,cs_acpi_find_pm_timer,cs_acpi_8042,cs_uefi_check_framebuffer,cs_fb_init,cs_arena_init,cs_arena_alloc,cs_boot_scene_prepare,cs_boot_scene_draw_input,cs_input_init,cs_input_pop,cs_pmtimer_init,cs_pmtimer_sample,cs_pmtimer_time';
        19='cs_uart_begin,cs_uart_enqueue,cs_uart_finish,cs_uart_poll,cs_native_keyboard_observed,cs_acpi_find_pm_timer,cs_native_read,cs_pmtimer_init,cs_pmtimer_sample';
        21='cs_trap_init,cs_trap_call,cs_trap_direct';22='cs_trap_call';23='cs_trap_get,cs_trap_set,cs_m68k_step'}
    $allowedImports[[Array]::IndexOf($sources,'apps/native-drawing/workload.c')]='cs_qd_view_init,cs_qd_view_offset,cs_qd_view_clip,cs_qd_view_rect'
    $allowedImports[[Array]::IndexOf($sources,'personality/system1/quickdraw-view.c')]='cs_qd_rect'
    $allowedImports[[Array]::IndexOf($sources,'platform/pc/ps2-bus.c')]='cs_ps2_begin,cs_ps2_poll,cs_ps2_receive,cs_ps2_mouse_begin,cs_ps2_mouse_feed,cs_mouse_button_begin,cs_mouse_button_sample'
    if($SelectedDrawing) {
        $sources+=@('apps/selected-drawing/workload.c','personality/system1/quickdraw-select.c')
        $allowedImports[9]+=',cs_selected_draw'
        $allowedImports[[Array]::IndexOf($sources,'apps/selected-drawing/workload.c')]='cs_qd_select_get,cs_qd_select_set,cs_qd_view_init,cs_qd_view_offset,cs_qd_view_clip,cs_qd_view_rect'
        $allowedImports[[Array]::IndexOf($sources,'personality/system1/quickdraw-select.c')]=''
    }
    if($OutlineDrawing) {
        $sources+=@('apps/outline-drawing/workload.c','personality/system1/quickdraw-select.c','personality/system1/quickdraw-outline.c')
        $allowedImports[9]+=',cs_outline_draw'
        $allowedImports[[Array]::IndexOf($sources,'apps/outline-drawing/workload.c')]='cs_qd_select_get,cs_qd_select_set,cs_qd_view_init,cs_qd_view_offset,cs_qd_view_clip,cs_qd_view_outline'
        $allowedImports[[Array]::IndexOf($sources,'personality/system1/quickdraw-select.c')]=''
        $allowedImports[[Array]::IndexOf($sources,'personality/system1/quickdraw-outline.c')]=''
    }
    if($WindowDrawing) {
        $sources+=@('platform/uefi/window.c','platform/pc/window-session.c','platform/pc/mono-present.c','platform/pc/window-input.c','core/input/pointer.c','core/graphics/cursor.c','core/ui/window.c','apps/window-scene/scene.c','apps/window-scene/input.c')
        $allowedImports[[Array]::IndexOf($sources,'platform/uefi/window.c')]='cs_uefi_check_framebuffer,cs_uefi_check_map,cs_uefi_check_owned,cs_native_read,cs_acpi_find_pm_timer,cs_acpi_8042,cs_pmtimer_init,cs_pmtimer_sample,cs_arena_init,cs_arena_alloc,cs_pc_window_session_start,cs_pc_window_session_poll,cs_pc_window_session_stop,cs_fb_init'
        $allowedImports[[Array]::IndexOf($sources,'platform/pc/window-session.c')]='cs_fb_init,cs_surface_init,cs_cursor_init,cs_cursor_bind,cs_input_init,cs_input_pop,cs_window_input_begin,cs_window_input_cancel,cs_pc_window_packet,cs_pc_mono_present,cs_pc_mono_present_rects,cs_ps2_bus_begin,cs_ps2_bus_poll_delivery'
        $allowedImports[[Array]::IndexOf($sources,'platform/pc/mono-present.c')]='cs_fb_init,cs_surface_init,cs_fb_present'
        $allowedImports[[Array]::IndexOf($sources,'platform/pc/window-input.c')]='cs_window_input_sample'
        $allowedImports[[Array]::IndexOf($sources,'core/input/pointer.c')]=''
        $allowedImports[[Array]::IndexOf($sources,'core/graphics/cursor.c')]='cs_surface_init'
        $allowedImports[[Array]::IndexOf($sources,'core/ui/window.c')]=''
        $allowedImports[[Array]::IndexOf($sources,'apps/window-scene/scene.c')]='cs_surface_init,cs_surface_fill,cs_product_scene,cs_cursor_query,cs_cursor_hide,cs_cursor_move,cs_cursor_advance,cs_cursor_show'
        $allowedImports[[Array]::IndexOf($sources,'apps/window-scene/input.c')]='memcpy,cs_surface_init,cs_cursor_query,cs_cursor_hide,cs_pointer_begin,cs_pointer_sample_apply,cs_pointer_cancel,cs_mouse_button_begin,cs_mouse_button_sample,cs_window_begin,cs_window_apply,cs_window_scene_compose'
    }
    if($WindowInputProbe) {
        $probeSource=if($WindowInputProbeMode -eq 'timeline'){'platform/pc/input-timeline.c'}else{'platform/pc/input-probe.c'}
        $sources+=@($probeSource,'platform/pc/x64/input-probe-halt.S')
        $allowedImports[[Array]::IndexOf($sources,$probeSource)]='cs_pc_input_probe_halt'
        $allowedImports[[Array]::IndexOf($sources,'platform/pc/x64/input-probe-halt.S')]=''
    }
    $common=@('-m64','-Wall','-Wextra','-Wpedantic','-Werror','-Wconversion','-Wsign-conversion',
        '-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-ident','-mno-red-zone','-mgeneral-regs-only','-Wframe-larger-than=2048',
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
                if($sources[$i].EndsWith('.S')){& $compiler -m64 "-DCS_X64_FAULT_PROBE=$probeNumber" -c (Join-Path $repoRoot $sources[$i]) -o $object}
                else{
                    $profileFlags=@()
                    if($WindowDrawing){$profileFlags+='-DCLASSICK_WINDOW_DRAWING=1'}
                    if($WindowInputProbe){$profileFlags+='-DCLASSICK_WINDOW_INPUT_PROBE=1'}
                    if($WindowInputProbe -and $WindowInputProbeMode -in @('motion','motion-later','raw-motion','raw-motion-later','raw-report-later')){$profileFlags+='-DCLASSICK_PC_INPUT_PROBE_MOTION=1'}
                    if($WindowInputProbe -and $WindowInputProbeMode -in @('raw-motion','raw-motion-later','raw-report-later')){$profileFlags+='-DCLASSICK_PC_RAW_PACKET_PROBE=1'}
                    if($WindowInputProbe -and $WindowInputProbeMode -in @('motion-later','raw-motion-later','raw-report-later')){$profileFlags+='-DCLASSICK_PC_INPUT_PROBE_SKIP_FIRST_PACKET=1'}
                    if($WindowInputProbe -and $WindowInputProbeMode -eq 'raw-report-later'){$profileFlags+='-DCLASSICK_PC_INPUT_PROBE_REPORT=1'}
                    if($WindowInputProbe -and $WindowInputProbeMode -in @('raw-sequence','raw-sequence-first')){$profileFlags+='-DCLASSICK_PC_INPUT_PROBE_SEQUENCE=1'}
                    if($WindowInputProbe -and $WindowInputProbeMode -eq 'raw-sequence-first'){$profileFlags+='-DCLASSICK_PC_INPUT_SEQUENCE_LENGTH=3'}
                    if($WindowInputProbe -and $WindowInputProbeMode -eq 'timeline'){$profileFlags+='-DCLASSICK_PC_INPUT_TIMELINE=1'}
                    if($sources[$i] -eq 'platform/uefi/keyboard.c'){$profileFlags=@('-DCLASSICK_PC_MOUSE=1')}
                    if($SelectedDrawing -or $OutlineDrawing){$profileFlags+='-DCLASSICK_SELECTED_DRAWING=1'}
                    if($OutlineDrawing){$profileFlags+='-DCLASSICK_OUTLINE_DRAWING=1'}
                    if($OutlineDrawing -and $sources[$i] -eq 'platform/uefi/keyboard.c'){$profileFlags+='-DCLASSICK_PC_PREVIEW_SESSION=1'}
                    & $compiler @common @profileFlags -std=c11 "-O$opt" -c (Join-Path $repoRoot $sources[$i]) -o $object
                }
                if($LASTEXITCODE -ne 0){throw 'Original EFI compile failed.'}
                if($i -lt 2 -or $i -eq 4 -or $i -eq 6 -or $i -eq 7 -or $i -eq 11 -or $i -eq 12 -or $i -eq 16 -or $i -eq 18 -or $i -eq 20) {
                    $check=if($i -eq 1){'Check-LoaderObject.cmake'}else{'Check-Freestanding.cmake'}
                    & cmake "-DNM=$nm" "-DOBJECT=$object" -P (Join-Path $PSScriptRoot $check)
                    if($LASTEXITCODE -ne 0){throw 'EFI object boundary failed.'}
                }
                if($sources[$i] -in @('core/input/mouse.c','platform/pc/ps2-mouse.c')) {
                    & cmake "-DNM=$nm" "-DOBJECT=$object" -P (Join-Path $PSScriptRoot 'Check-Freestanding.cmake')
                    if($LASTEXITCODE -ne 0){throw 'EFI pure mouse object boundary failed.'}
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
            $audit=& (Join-Path $PSScriptRoot 'Test-UEFIImage.ps1') -Image $image -Map $map -FaultProbe $FaultProbe -PassThru
            if($OutlineDrawing){
                $mapText=[IO.File]::ReadAllText($map)
                foreach($name in @('cs_outline_draw','cs_qd_view_outline','cs_qd_select_get','cs_qd_select_set')){if($mapText -notmatch ('\b'+$name+'\b')){throw 'Disconnected native outline preview.'}}
            }
            if($WindowDrawing){
                $mapText=[IO.File]::ReadAllText($map)
                foreach($name in @('cs_native_window_loop','cs_pc_window_session_start','cs_pc_window_session_poll','cs_window_input_sample','cs_pc_mono_present','cs_pc_mono_present_rects')){if($mapText -notmatch ('\b'+$name+'\b')){throw 'Disconnected window profile.'}}
            }
            if($WindowInputProbe){
                $probeObserver=if($WindowInputProbeMode -eq 'timeline'){'cs_pc_input_timeline_observe'}else{'cs_pc_input_probe_observe'}
                foreach($name in @($probeObserver,'cs_pc_input_probe_halt','cs_pc_input_probe_spin')){
                    if($mapText -notmatch ('\b'+$name+'\b')){throw 'Disconnected input probe.'}
                }
            }
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
        [pscustomobject]@{name='fault-export';at=$referenceAudit.fault_raw+135;bytes=[byte[]]@(0xB8);reason='Fault export bytes'},
        [pscustomobject]@{name='fault-marker';at=$referenceAudit.fault_raw+169;bytes=[byte[]]@(0);reason='Fault export bytes'},
        [pscustomobject]@{name='services-export';at=$referenceAudit.services_raw+5;bytes=[byte[]]@(0x90);reason='Service export bytes'},
        [pscustomobject]@{name='aline-export';at=$referenceAudit.aline_raw+5;bytes=[byte[]]@(0x90);reason='A-line export bytes'},
        [pscustomobject]@{name='active-state';at=$referenceAudit.state_raw;bytes=[byte[]]@(1);reason='Active-state initial'},
        [pscustomobject]@{name='port-read';at=$referenceAudit.inl_raw+2;bytes=[byte[]]@(0x90);reason='Port read bytes'},
        [pscustomobject]@{name='byte-read';at=$referenceAudit.inb_raw+4;bytes=[byte[]]@(0x90);reason='Byte port read bytes'},
        [pscustomobject]@{name='byte-write';at=$referenceAudit.outb_raw+4;bytes=[byte[]]@(0x90);reason='Byte port write bytes'})
    if($FaultProbe -ne 'none'){
        $mutations += [pscustomobject]@{name='probe-opcode';at=$referenceAudit.probe_raw;bytes=[byte[]]@(0x90);reason='Probe bytes'}
        $rejected=$false
        try{& (Join-Path $PSScriptRoot 'Test-UEFIImage.ps1') -Image $reference -Map $referenceMap -FaultProbe none | Out-Null}
        catch{if($_.Exception.Message -notmatch 'Probe profile'){throw};$rejected=$true}
        if(-not $rejected){throw 'Injected image accepted as normal.'}
    }
    foreach($mutation in $mutations) {
        $bytes=[byte[]]$original.Clone(); [Array]::Copy($mutation.bytes,0,$bytes,[int]$mutation.at,$mutation.bytes.Length)
        $image=Join-Path $controls ($mutation.name+'.EFI'); [IO.File]::WriteAllBytes($image,$bytes); $rejected=$false
        try{& (Join-Path $PSScriptRoot 'Test-UEFIImage.ps1') -Image $image -Map $referenceMap -FaultProbe $FaultProbe | Out-Null}
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
    $componentControls=@(@(14,'ps2','cs_ps2_poll'),@(15,'keyboard','cs_native_keyboard_observed'),@(16,'input','cs_input_push'),@(17,'byte-io','cs_x64_inb|cs_x64_outb'),@(18,'uart','cs_uart_poll'),@(19,'native-uart','cs_native_uart_loop'),@(20,'services','cs_trap_call'),@(21,'service-probe','cs_service_probe'),@(22,'m68k','cs_m68k_step'),@(23,'aline-probe','cs_aline_probe'),@(([Array]::IndexOf($sources,'apps/native-drawing/workload.c')),'native-drawing','cs_native_draw'),@([Array]::IndexOf($sources,'core/input/mouse.c'),'mouse-button','cs_mouse_button_'),@([Array]::IndexOf($sources,'platform/pc/ps2-mouse.c'),'mouse-packet','cs_ps2_mouse_'),@([Array]::IndexOf($sources,'platform/pc/ps2-bus.c'),'mouse-bus','cs_ps2_bus_'),@([Array]::IndexOf($sources,'personality/system1/quickdraw-view.c'),'drawing-view','cs_qd_view_'))
    if($SelectedDrawing){$componentControls+=@(@([Array]::IndexOf($sources,'apps/selected-drawing/workload.c'),'selected-workload','cs_selected_draw'),@([Array]::IndexOf($sources,'personality/system1/quickdraw-select.c'),'selected-core','cs_qd_select_'))}
    if($OutlineDrawing){$componentControls+=@(@([Array]::IndexOf($sources,'apps/outline-drawing/workload.c'),'outline-workload','cs_outline_draw'),@([Array]::IndexOf($sources,'personality/system1/quickdraw-select.c'),'outline-selection','cs_qd_select_'),@([Array]::IndexOf($sources,'personality/system1/quickdraw-outline.c'),'outline-core','cs_qd_view_outline'))}
    if($WindowDrawing){
        # These two exports are intentionally not imported by the new entry.
        $componentControls=@($componentControls | Where-Object {$_[1] -notin @('native-uart','aline-probe')})
        $componentControls+=,@([Array]::IndexOf($sources,'platform/uefi/window.c'),'window-binding','cs_native_window_loop')
        $componentControls+=,@([Array]::IndexOf($sources,'platform/pc/window-session.c'),'window-session','cs_pc_window_session_')
        $componentControls+=,@([Array]::IndexOf($sources,'platform/pc/mono-present.c'),'mono-present','cs_pc_mono_present')
        $componentControls+=,@([Array]::IndexOf($sources,'platform/pc/window-input.c'),'window-packet','cs_pc_window_packet')
        $componentControls+=,@([Array]::IndexOf($sources,'core/input/pointer.c'),'pointer','cs_pointer_')
        $componentControls+=,@([Array]::IndexOf($sources,'core/graphics/cursor.c'),'cursor','cs_cursor_')
        $componentControls+=,@([Array]::IndexOf($sources,'core/ui/window.c'),'window-state','cs_window_')
        $componentControls+=,@([Array]::IndexOf($sources,'apps/window-scene/scene.c'),'window-compose','cs_window_scene_compose')
        $componentControls+=,@([Array]::IndexOf($sources,'apps/window-scene/input.c'),'window-owner','cs_window_input_')
    }
    if($WindowInputProbe){
        $probeObserver=if($WindowInputProbeMode -eq 'timeline'){'cs_pc_input_timeline_observe'}else{'cs_pc_input_probe_observe'}
        $componentControls+=,@([Array]::IndexOf($sources,$probeSource),'input-probe',$probeObserver)
        $componentControls+=,@([Array]::IndexOf($sources,'platform/pc/x64/input-probe-halt.S'),'input-probe-halt','cs_pc_input_probe_halt')
    }
    foreach($control in $componentControls) {
        try {
            $ErrorActionPreference='Continue'
            $missing=@(& $compiler @link @($referenceObjects | Where-Object {$_ -ne $referenceObjects[$control[0]]}) -o (Join-Path $controls ('must-not-link-'+$control[1]+'.EFI')) 2>&1)
        } finally{$ErrorActionPreference=$previousPreference}
        if($LASTEXITCODE -eq 0 -or ($missing -join "`n") -notmatch $control[2]){throw "Missing object control did not reject: $($control[1])"}
        $global:LASTEXITCODE=0
        $missing | Out-File -LiteralPath (Join-Path $controls ('missing-'+$control[1]+'.log')) -Encoding UTF8
    }
    $payload=Join-Path $BuildRoot 'payload/EFI/BOOT'; [void][IO.Directory]::CreateDirectory($payload)
    $packaged=Join-Path $payload 'BOOTX64.EFI'; Copy-Item -LiteralPath $release -Destination $packaged
    if((Get-FileHash -LiteralPath $packaged -Algorithm SHA256).Hash.ToLowerInvariant() -ne $results[1].sha256){throw 'EFI payload copy differs.'}
    [ordered]@{diagnostic_specification=$(if($WindowInputProbeMode -in @('raw-sequence','raw-sequence-first')){'SPEC-0085'}elseif($WindowInputProbeMode -eq 'timeline'){'SPEC-0088'}else{$null});input_sequence_bytes=$(if($WindowInputProbeMode -eq 'raw-sequence'){48}elseif($WindowInputProbeMode -eq 'raw-sequence-first'){3}else{0});fault_probe=$FaultProbe;window_input_probe=[bool]$WindowInputProbe;window_input_probe_mode=$(if($WindowInputProbe){$WindowInputProbeMode}else{'none'});missing_input_probe_controls=(2*[int][bool]$WindowInputProbe);window_drawing=[bool]$WindowDrawing;window_trace_version=$(if($WindowDrawing){1}else{0});window_trace_offset=$(if($WindowDrawing){2048}else{0});missing_window_controls=(9*[int][bool]$WindowDrawing);outline_drawing=[bool]$OutlineDrawing;continuous_preview=([bool]$OutlineDrawing -or [bool]$WindowDrawing);missing_outline_controls=(3*[int][bool]$OutlineDrawing);selected_drawing=[bool]$SelectedDrawing;missing_selected_workload_controls=[int][bool]$SelectedDrawing;missing_selected_core_controls=[int][bool]$SelectedDrawing;images=$results;image_rejections=$mutations.Count;profile_rejections=[int]($FaultProbe -ne 'none');missing_transition_controls=1;missing_exceptions_controls=1;missing_presenter_controls=1;missing_acpi_controls=1;
        missing_ps2_controls=1;missing_keyboard_controls=1;missing_input_controls=1;missing_byte_io_controls=1;
        missing_uart_controls=1;missing_native_uart_controls=([int](-not $WindowDrawing));missing_service_controls=1;missing_service_probe_controls=1;
        missing_m68k_controls=1;missing_aline_probe_controls=([int](-not $WindowDrawing));missing_native_drawing_controls=1;missing_mouse_button_controls=1;missing_mouse_packet_controls=1;missing_mouse_bus_controls=1;missing_drawing_view_controls=1;drawing_context_specification='SPEC-0041';pc_mouse_profile=1;keyboard_trace_version=$(if($WindowDrawing){0}else{2});
        payload='payload/EFI/BOOT/BOOTX64.EFI';payload_kind='directory-tree-not-disk';loaded=$false} |
        ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $BuildRoot 'efi-results.json') -Encoding UTF8
    Write-Output "EFI inspection PASS: four twin images, $($mutations.Count) corruption rejections, $(19+2*[int][bool]$SelectedDrawing+3*[int][bool]$OutlineDrawing+7*[int][bool]$WindowDrawing+2*[int][bool]$WindowInputProbe) omitted-object rejections and original payload tree; no image executed."
} finally{$env:PATH=$originalPath}
