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
        ($s.flags -band 2147483648) -ne 0 -or ($s.flags -band 0x40000000) -eq 0){throw 'Section bounds/permissions rejected.'}
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
    'cs_uefi_image_spans','cs_uefi_loader_run','cs_uefi_entry','cs_native_stop','cs_x64_enter','cs_native_halt','cs_entry_anchor')
$symbols=@{}
foreach($name in $names) {
    $matches=[regex]::Matches($mapText,'(?m)^([0-9a-fA-F]+)\s+[0-9a-fA-F]+\s+\d+\s+'+$name+'\s*$')
    $addresses=@($matches | ForEach-Object {[Convert]::ToInt64($_.Groups[1].Value,16)} | Select-Object -Unique)
    if($addresses.Count -ne 1){throw "Required original symbol absent/ambiguous: $name"}
    $symbols[$name]=$addresses[0]
    if($name -ne 'cs_entry_anchor' -and -not (Executable $addresses[0])){throw 'Original function permissions rejected.'}
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
$result=[pscustomobject]@{sha256=(Get-FileHash -LiteralPath $Image -Algorithm SHA256).Hash.ToLowerInvariant();
    size_bytes=$bytes.Length;image_size=$imageSize;entry_rva=$entry;relocations=$patches.Count;
    original_symbols=$names.Count;imports=0;runtime_libraries=0;reloc_raw=$relocRaw;
    anchor_raw=$anchorRaw;transition_raw=$transitionRaw;halt_raw=$haltRaw;first_section_header=$sections[0].header}
if($PassThru){$result}else{Write-Output 'EFI image audit PASS: original entry/stack/halt, bounded relocations, zero imports/libraries; unloaded.'}
