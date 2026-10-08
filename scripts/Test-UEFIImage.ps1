# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$Image,
    [Parameter(Mandatory=$true)][string]$Map,[switch]$PassThru)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
if((Get-Item -LiteralPath $Image).Length -gt 4194304){throw 'Image size rejected.'}
$bytes=[IO.File]::ReadAllBytes([IO.Path]::GetFullPath($Image))
function Bounds([long]$at,[long]$size) {
    if($at -lt 0 -or $size -lt 0 -or $at -gt $bytes.Length -or $size -gt $bytes.Length-$at){throw 'PE bounds rejected.'}
}
function U16([long]$at){Bounds $at 2; [BitConverter]::ToUInt16($bytes,[int]$at)}
function U32([long]$at){Bounds $at 4; [BitConverter]::ToUInt32($bytes,[int]$at)}
function U64([long]$at){Bounds $at 8; [BitConverter]::ToUInt64($bytes,[int]$at)}
if((U16 0) -ne 0x5A4D){throw 'DOS signature rejected.'}
$pe=[long](U32 60)
if((U32 $pe) -ne 0x4550){throw 'PE signature rejected.'}
if((U16 ($pe+4)) -ne 0x8664){throw 'Machine rejected.'}
if((U32 ($pe+8)) -ne 0){throw 'Timestamp rejected.'}
if((U32 ($pe+12)) -ne 0 -or (U32 ($pe+16)) -ne 0){throw 'Unstripped symbol table rejected.'}
$characteristics=U16 ($pe+22)
if(($characteristics -band 2) -eq 0 -or ($characteristics -band 0x2001) -ne 0){throw 'Executable/relocation characteristics rejected.'}
$count=U16 ($pe+6); $optional=$pe+24; $optionalSize=U16 ($pe+20)
if($count -lt 1 -or $count -gt 16 -or $optionalSize -ne 240){throw 'Header profile rejected.'}
Bounds $optional $optionalSize
if((U16 $optional) -ne 0x20B){throw 'PE32+ rejected.'}
if((U16 ($optional+68)) -ne 10){throw 'EFI application subsystem rejected.'}
if(((U16 ($optional+70)) -band 0x140) -ne 0x140){throw 'Relocatable/NX characteristics rejected.'}
$entry=[long](U32 ($optional+16)); $imageBase=U64 ($optional+24)
$sectionAlignment=U32 ($optional+32); $fileAlignment=U32 ($optional+36)
$imageSize=[long](U32 ($optional+56)); $headers=[long](U32 ($optional+60))
if($sectionAlignment -ne 4096 -or $fileAlignment -ne 512 -or $imageBase -ne 0x10000000 -or
    $imageSize -gt 4194304 -or $imageSize -lt 8192 -or $imageSize%4096 -ne 0 -or
    $headers%512 -ne 0 -or $headers -lt $optional+$optionalSize+$count*40){throw 'Alignment/extent profile rejected.'}
Bounds 0 $headers
if((U32 ($optional+108)) -ne 16){throw 'Directory count rejected.'}
$sections=@(); $lastVirtual=4096; $lastRaw=$headers
for($i=0;$i -lt $count;++$i) {
    $at=$optional+$optionalSize+$i*40; Bounds $at 40
    $s=[pscustomobject]@{rva=[long](U32 ($at+12));virtual=[long](U32 ($at+8));
        raw=[long](U32 ($at+20));size=[long](U32 ($at+16));flags=[long](U32 ($at+36));header=$at}
    if($s.rva -ne $lastVirtual -or $s.rva%4096 -ne 0 -or $s.virtual -eq 0 -or
        $s.virtual -gt $imageSize-$s.rva -or $s.size -eq 0 -or $s.size%512 -ne 0 -or
        $s.raw%512 -ne 0 -or $s.raw -lt $lastRaw -or $s.size -lt $s.virtual -or
        ($s.flags -band 0x40000000) -eq 0 -or
        (($s.flags -band 2147483648) -ne 0 -and
            (($s.flags -band 0x20000000) -ne 0 -or $s.virtual -ne 8))){throw 'Section bounds/permissions rejected.'}
    Bounds $s.raw $s.size
    for($j=24;$j -lt 36;$j+=4){if((U32 ($at+$j)) -ne 0){throw 'Section metadata rejected.'}}
    $lastVirtual=$s.rva+[long]([Math]::Ceiling($s.virtual/4096.0)*4096)
    $lastRaw=$s.raw+$s.size; $sections += $s
}
if($lastVirtual -ne $imageSize -or $lastRaw -ne $bytes.Length){throw 'Trailing/virtual extent rejected.'}
function Raw([long]$rva,[long]$size) {
    foreach($s in $sections) {
        if($rva -ge $s.rva -and $rva-$s.rva -le $s.virtual -and $size -le $s.virtual-($rva-$s.rva) -and
            $size -le $s.size-($rva-$s.rva)){return $s.raw+$rva-$s.rva}
    }
    throw 'RVA bounds rejected.'
}
function Executable([long]$rva) {
    $null=Raw $rva 1
    foreach($s in $sections){if($rva -ge $s.rva -and $rva-$s.rva -lt $s.virtual){return ($s.flags -band 0x20000000) -ne 0}}
    return $false
}
if($entry -eq 0 -or -not (Executable $entry)){throw 'Entry rejected.'}
for($i=0;$i -lt 16;++$i) {
    if($i -eq 5){continue}
    if((U32 ($optional+112+$i*8)) -ne 0 -or (U32 ($optional+116+$i*8)) -ne 0){throw 'Imports/runtime/other directories rejected.'}
}
if((Get-Item -LiteralPath $Map).Length -gt 1048576){throw 'Map size rejected.'}
$mapText=[IO.File]::ReadAllText([IO.Path]::GetFullPath($Map))
if($mapText -match '(?i)\.(a|lib|dll)(?=[\s)\r\n]|$)'){throw 'Runtime library input rejected.'}
$names=@('cs_uefi_check_framebuffer','cs_uefi_check_map','cs_uefi_check_owned','cs_uefi_exit_init',
    'cs_uefi_exit_snapshot','cs_uefi_exit_observe','cs_uefi_exit_allowed','cs_uefi_table_crc',
    'cs_uefi_image_spans','cs_uefi_loader_run','cs_uefi_entry','cs_native_stop','cs_x64_enter','cs_native_halt','cs_entry_anchor',
    'cs_x64_tables_init','cs_x64_install','cs_x64_reload','cs_x64_vector_base','cs_x64_vector_end','cs_x64_fault','cs_x64_active_state',
    'cs_surface_init','cs_surface_fill','cs_surface_clear','cs_arena_init','cs_arena_alloc','cs_arena_reset',
    'cs_fb_init','cs_fb_present','cs_scene_render','cs_boot_scene_prepare','cs_boot_scene_draw','cs_native_present',
    'cs_uefi_acpi20_rsdp','cs_native_read','cs_native_timer_probe','cs_acpi_find_pm_timer',
    'cs_pmtimer_init','cs_pmtimer_sample','cs_pmtimer_time','cs_x64_inl','cs_native_progress_loop',
    'cs_ps2_begin','cs_ps2_poll','cs_acpi_8042','cs_native_keyboard_loop',
    'cs_input_init','cs_input_push','cs_input_pop','cs_input_reset','cs_x64_inb','cs_x64_outb',
    'cs_uart_begin','cs_uart_enqueue','cs_uart_finish','cs_uart_poll','cs_native_keyboard_observed','cs_native_uart_loop')
$symbols=@{}
foreach($name in $names) {
    $matches=[regex]::Matches($mapText,'(?m)^([0-9a-fA-F]+)\s+[0-9a-fA-F]+\s+\d+\s+'+$name+'\s*$')
    $addresses=@($matches | ForEach-Object {[Convert]::ToInt64($_.Groups[1].Value,16)} | Select-Object -Unique)
    if($addresses.Count -ne 1){throw "Required original symbol absent/ambiguous: $name"}
    $symbols[$name]=$addresses[0]
    if($name -notin @('cs_entry_anchor','cs_x64_active_state') -and -not (Executable $addresses[0])){throw 'Original function permissions rejected.'}
}
if($entry -ne $symbols.cs_uefi_entry){throw 'Entry differs from original wrapper.'}
$anchorRaw=Raw $symbols.cs_entry_anchor 8
if(Executable $symbols.cs_entry_anchor){throw 'Anchor permissions rejected.'}
if((U64 $anchorRaw) -ne $imageBase+$symbols.cs_x64_enter){throw 'Original transition anchor rejected.'}
$relocRva=[long](U32 ($optional+152)); $relocSize=[long](U32 ($optional+156))
if($relocRva -eq 0 -or $relocSize -lt 12 -or $relocSize -gt 4096){throw 'Relocation directory rejected.'}
$relocRaw=Raw $relocRva $relocSize; $cursor=0; $patches=@(); $lastPage=-1
while($cursor -lt $relocSize) {
    if($relocSize-$cursor -lt 8){throw 'Relocation block rejected.'}
    $page=[long](U32 ($relocRaw+$cursor)); $size=[long](U32 ($relocRaw+$cursor+4))
    if($page%4096 -ne 0 -or $page -le $lastPage -or $size -lt 12 -or $size%4 -ne 0 -or $size -gt $relocSize-$cursor){throw 'Relocation block rejected.'}
    $lastPage=$page
    for($i=8;$i -lt $size;$i+=2) {
        $word=U16 ($relocRaw+$cursor+$i); $type=$word -shr 12
        if($type -eq 0){if($word -ne 0){throw 'Relocation padding rejected.'}; continue}
        if($type -ne 10){throw 'Relocation type rejected.'}
        $rva=$page+($word -band 4095)
        if($rva%8 -ne 0 -or $patches -contains $rva -or $patches.Count -ge 32){throw 'Relocation patch rejected.'}
        $raw=Raw $rva 8; $pointer=U64 $raw
        if($pointer -lt $imageBase -or $pointer -ge $imageBase+$imageSize){throw 'Relocation pointer rejected.'}
        $null=Raw ([long]($pointer-$imageBase)) 1
        $patches += $rva
    }
    $cursor+=$size
}
if($patches -notcontains $symbols.cs_entry_anchor){throw 'Anchor relocation absent.'}
# Audit the exact original assembly and its relative call destination, not just mnemonics.
$transitionRaw=Raw $symbols.cs_x64_enter 20
$expected=[byte[]]@(0xFA,0xFC,0x48,0x89,0xD4,0x48,0x83,0xE4,0xF0,0x31,0xED,0x48,0x83,0xEC,0x20,0xE8)
for($i=0;$i -lt $expected.Length;++$i){if($bytes[$transitionRaw+$i] -ne $expected[$i]){throw 'Stack transition bytes rejected.'}}
$target=$symbols.cs_x64_enter+20+[BitConverter]::ToInt32($bytes,[int]($transitionRaw+16))
if($target -ne $symbols.cs_native_stop -or $symbols.cs_native_halt -ne $symbols.cs_x64_enter+20){throw 'Transition destination rejected.'}
$haltRaw=Raw $symbols.cs_native_halt 4; $halt=[byte[]]@(0xFA,0xF4,0xEB,0xFC)
for($i=0;$i -lt 4;++$i){if($bytes[$haltRaw+$i] -ne $halt[$i]){throw 'Halt bytes rejected.'}}
# One original pointer slot is the only writable image data. Tables/stacks live
# in the separate loader bundle. LTR needs writable GDT busy-bit storage there.
$dataSections=@($sections | Where-Object {($_.flags -band 2147483648) -ne 0})
if($dataSections.Count -ne 1 -or $dataSections[0].rva -ne $symbols.cs_x64_active_state -or
    $symbols.cs_x64_active_state%8 -ne 0 -or (Executable $symbols.cs_x64_active_state)) {throw 'Active-state section rejected.'}
$stateRaw=Raw $symbols.cs_x64_active_state 8
if((U64 $stateRaw) -ne 0){throw 'Active-state initial value rejected.'}
function Exact([long]$rva,[byte[]]$expected,[string]$reason) {
    $at=Raw $rva $expected.Length
    for($i=0;$i -lt $expected.Length;++$i){if($bytes[$at+$i] -ne $expected[$i]){throw $reason}}
}
$install=$symbols.cs_x64_install; $installRaw=Raw $install 25
Exact $install ([byte[]]@(0xFA,0xFC,0x48,0x89,0x0D)) 'Descriptor install bytes rejected.'
if($install+9+[BitConverter]::ToInt32($bytes,[int]($installRaw+5)) -ne $symbols.cs_x64_active_state){throw 'Descriptor state pointer rejected.'}
Exact ($install+9) ([byte[]]@(0x0F,0x01,0x51,0x28,0x6A,8,0x48,0x8D,5)) 'Descriptor install bytes rejected.'
if($install+22+[BitConverter]::ToInt32($bytes,[int]($installRaw+18)) -ne $symbols.cs_x64_reload -or
    $symbols.cs_x64_reload -ne $install+25){throw 'Descriptor reload destination rejected.'}
Exact ($install+22) ([byte[]]@(0x50,0x48,0xCB)) 'Descriptor install bytes rejected.'
$reloadRaw=Raw $symbols.cs_x64_reload 38
Exact $symbols.cs_x64_reload ([byte[]]@(0x66,0xB8,0x10,0,0x8E,0xD8,0x8E,0xC0,0x8E,0xD0,
    0x66,0x31,0xC0,0x0F,0,0xD0,0x66,0xB8,0x18,0,0x0F,0,0xD8,0x0F,1,0x59,0x32,
    0xC7,0x81,0xF0,0,0,0,1,0,0,0,0xC3)) 'Descriptor reload bytes rejected.'
$vector=$symbols.cs_x64_vector_base; $vectorRaw=Raw $vector 8192
if($vector%32 -ne 0 -or $symbols.cs_x64_vector_end -ne $vector+8192 -or
    $symbols.cs_x64_fault -ne $symbols.cs_x64_vector_end){throw 'Vector extent rejected.'}
for($v=0;$v -lt 256;++$v) {
    $at=$vectorRaw+$v*32; $prefix=if($v -in @(8,10,11,12,13,14,17,21)){0}else{2}
    if($prefix -eq 2 -and ($bytes[$at] -ne 0x6A -or $bytes[$at+1] -ne 0)){throw 'Vector error placeholder rejected.'}
    if($bytes[$at+$prefix] -ne 0x68 -or (U32 ($at+$prefix+1)) -ne $v -or
        $bytes[$at+$prefix+5] -ne 0xE9 -or
        $vector+$v*32+$prefix+10+[BitConverter]::ToInt32($bytes,[int]($at+$prefix+6)) -ne $symbols.cs_x64_fault){throw 'Vector bytes/destination rejected.'}
    for($i=$prefix+10;$i -lt 32;++$i){if($bytes[$at+$i] -ne 0x90){throw 'Vector padding rejected.'}}
}
$fault=$symbols.cs_x64_fault; $faultRaw=Raw $fault 137
Exact $fault ([byte[]]@(0xFA,0xFC,0x41,0x0F,0x20,0xD0,0x48,0x8B,0x0D)) 'Fault capture bytes rejected.'
if($fault+13+[BitConverter]::ToInt32($bytes,[int]($faultRaw+9)) -ne $symbols.cs_x64_active_state){throw 'Fault state pointer rejected.'}
Exact ($fault+13) ([byte[]]@(0x31,0xC0,0xBA,1,0,0,0,0xF0,0x0F,0xB1,0x91,0xB0,0,0,0,0x0F,0x85)) 'Fault capture bytes rejected.'
if($fault+34+[BitConverter]::ToInt32($bytes,[int]($faultRaw+30)) -ne $symbols.cs_native_halt){throw 'Nested fault destination rejected.'}
Exact ($fault+34) ([byte[]]@(0x8B,4,0x24,0x89,0x81,0xB4,0,0,0)) 'Fault capture bytes rejected.'
for($field=0;$field -lt 6;++$field) {
    Exact ($fault+43+$field*12) ([byte[]]@(0x48,0x8B,0x44,0x24,(8+$field*8),0x48,0x89,0x81,(184+$field*8),0,0,0)) 'Fault frame field rejected.'
}
Exact ($fault+115) ([byte[]]@(0x4C,0x89,0x81,0xE8,0,0,0,0xC7,0x81,0xB0,0,0,0,2,0,0,0,0xE9)) 'Fault publish bytes rejected.'
if($fault+137+[BitConverter]::ToInt32($bytes,[int]($faultRaw+133)) -ne $symbols.cs_native_halt){throw 'Fault halt destination rejected.'}
# SPEC-0010/0012: original 32-bit timer IN and byte keyboard IN/OUT through DX.
$inlRaw=Raw $symbols.cs_x64_inl 4
Exact $symbols.cs_x64_inl ([byte[]]@(0x89,0xCA,0xED,0xC3)) 'Port read bytes rejected.'
$inbRaw=Raw $symbols.cs_x64_inb 6; $outbRaw=Raw $symbols.cs_x64_outb 6
Exact $symbols.cs_x64_inb ([byte[]]@(0x89,0xCA,0x31,0xC0,0xEC,0xC3)) 'Byte port read bytes rejected.'
Exact $symbols.cs_x64_outb ([byte[]]@(0x89,0xD0,0x89,0xCA,0xEE,0xC3)) 'Byte port write bytes rejected.'
$result=[pscustomobject]@{sha256=(Get-FileHash -LiteralPath $Image -Algorithm SHA256).Hash.ToLowerInvariant();
    size_bytes=$bytes.Length;image_size=$imageSize;entry_rva=$entry;relocations=$patches.Count;
    original_symbols=$names.Count;imports=0;runtime_libraries=0;reloc_raw=$relocRaw;
    anchor_raw=$anchorRaw;transition_raw=$transitionRaw;halt_raw=$haltRaw;first_section_header=$sections[0].header;
    install_raw=$installRaw;reload_raw=$reloadRaw;vector_raw=$vectorRaw;fault_raw=$faultRaw;state_raw=$stateRaw;inl_raw=$inlRaw;inb_raw=$inbRaw;outb_raw=$outbRaw;vectors=256}
if($PassThru){$result}else{Write-Output 'EFI image audit PASS: original entry/stack/halt, bounded relocations, zero imports/libraries; unloaded.'}
