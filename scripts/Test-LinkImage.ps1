# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$Image,
    [Parameter(Mandatory=$true)][string]$Map,
    [Parameter(Mandatory=$true)][ValidateSet('clang','gcc')][string]$Compiler,
    [Parameter(Mandatory=$true)][ValidateSet(32,64)][int]$Bits,
    [switch]$PassThru)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$bytes=[IO.File]::ReadAllBytes([IO.Path]::GetFullPath($Image))
function Bounds([long]$offset,[long]$length) {
    if($offset -lt 0 -or $length -lt 0 -or $offset -gt $bytes.Length -or $length -gt $bytes.Length-$offset) { throw 'PE bounds rejected.' }
}
function U16([long]$offset) { Bounds $offset 2; [BitConverter]::ToUInt16($bytes,[int]$offset) }
function U32([long]$offset) { Bounds $offset 4; [BitConverter]::ToUInt32($bytes,[int]$offset) }
function U64([long]$offset) { Bounds $offset 8; [BitConverter]::ToUInt64($bytes,[int]$offset) }
if((U16 0) -ne 0x5A4D) { throw 'DOS signature rejected.' }
$pe=U32 60
if((U32 $pe) -ne 0x4550) { throw 'PE signature rejected.' }
$machine=U16 ($pe+4)
$expectedMachine=if($Bits -eq 64){0x8664}else{0x14C}
if($machine -ne $expectedMachine) { throw 'Machine rejected.' }
if((U32 ($pe+8)) -ne 0) { throw 'Image timestamp must be zero.' }
$sectionCount=U16 ($pe+6)
if($sectionCount -eq 0 -or $sectionCount -gt 32) { throw 'Section count rejected.' }
$optional=$pe+24
$optionalSize=U16 ($pe+20)
Bounds $optional $optionalSize
$magic=U16 $optional
if($magic -ne $(if($Bits -eq 64){0x20B}else{0x10B})) { throw 'Optional header rejected.' }
$entry=U32 ($optional+16)
$imageBase=if($Bits -eq 64){U64 ($optional+24)}else{U32 ($optional+28)}
if((U16 ($optional+68)) -ne 1) { throw 'Fixture must use the unloaded native subsystem.' }
$directoryOffset=if($Bits -eq 64){112}else{96}
$directoryCount=U32 ($optional+$directoryOffset-4)
if($directoryCount -lt 14 -or $directoryCount -gt 16 -or $directoryOffset+$directoryCount*8 -gt $optionalSize) { throw 'Directory bounds rejected.' }
$sections=@()
for($i=0;$i -lt $sectionCount;++$i) {
    $at=$optional+$optionalSize+$i*40
    Bounds $at 40
    $sections += [pscustomobject]@{rva=(U32 ($at+12));raw=(U32 ($at+20));size=(U32 ($at+16));flags=(U32 ($at+36))}
    Bounds $sections[-1].raw $sections[-1].size
}
function Raw([uint32]$rva,[uint32]$size) {
    foreach($section in $sections) {
        if($rva -ge $section.rva -and [long]$rva-$section.rva -le $section.size -and
                $size -le $section.size-([long]$rva-$section.rva)) {
            return [long]$section.raw+$rva-$section.rva
        }
    }
    throw 'Directory RVA rejected.'
}
$entrySection=@($sections | Where-Object { $entry -ge $_.rva -and [long]$entry-$_.rva -lt $_.size -and ($_.flags -band 0x20000000) -ne 0 })
if($entry -eq 0 -or $entrySection.Count -ne 1) { throw 'Entry point rejected.' }
$emptyImportBytes=0
foreach($index in @(1,9,12,13)) {
    $at=$optional+$directoryOffset+$index*8
    $rva=U32 $at; $size=U32 ($at+4)
    if($rva -eq 0 -and $size -eq 0) { continue }
    # GNU ld emits a bounded all-zero import terminator even with no DLL imports.
    if($index -ne 1 -or $rva -eq 0 -or $size -lt 20 -or $size -gt 64) { throw 'Imports/TLS rejected.' }
    $raw=Raw $rva $size
    for($j=0;$j -lt $size;++$j) { if($bytes[$raw+$j] -ne 0) { throw 'Imports/TLS rejected.' } }
    $emptyImportBytes=$size
}
$mapText=[IO.File]::ReadAllText([IO.Path]::GetFullPath($Map))
if($mapText -match '(?i)\.(a|lib|dll)(?=[\s)\r\n]|$)') { throw 'Library input rejected.' }
$symbols=@('cs_surface_init','cs_surface_fill','cs_surface_clear','cs_arena_init','cs_arena_alloc','cs_arena_reset',
    'cs_input_init','cs_input_push','cs_input_pop','cs_input_reset','cs_time_add','cs_time_elapsed','cs_time_reached',
    'cs_clock_init','cs_clock_observe','cs_clock_advance','cs_core_link_probe','cs_link_entry')
$entryAddress=$null
foreach($symbol in $symbols) {
    $pattern=if($Compiler -eq 'clang'){ '(?m)^([0-9a-fA-F]+)\s+[0-9a-fA-F]+\s+\d+\s+_?'+$symbol+'\s*$' }
        else { '(?m)^\s+0x([0-9a-fA-F]+)\s+_?'+$symbol+'\s*$' }
    $match=[regex]::Match($mapText,$pattern)
    if(-not $match.Success) { throw "Required core symbol absent: $symbol" }
    if($symbol -eq 'cs_link_entry') { $entryAddress=[Convert]::ToUInt64($match.Groups[1].Value,16) }
}
$expectedEntry=if($Compiler -eq 'clang'){$entryAddress}else{$entryAddress-$imageBase}
if($entry -ne $expectedEntry) { throw 'Entry point differs from the declared original entry symbol.' }
$result=[pscustomobject]@{machine=$machine;bits=$Bits;entry_rva=$entry;size_bytes=$bytes.Length;empty_import_bytes=$emptyImportBytes;
    sha256=(Get-FileHash -LiteralPath $Image -Algorithm SHA256).Hash.ToLowerInvariant();imports=0;runtime_libraries=0;required_symbols=$symbols.Count}
if($PassThru) { $result } else { Write-Output "Complete link audit PASS: $Compiler $Bits-bit; 18 original symbols; no runtime libraries or imports." }
