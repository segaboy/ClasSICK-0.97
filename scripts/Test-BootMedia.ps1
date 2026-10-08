# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
# SPEC-0014 independent image check. This reader shares no code with the C writer:
# it re-derives every field from UEFI 2.11 chapter 5/13.3 and the FAT32 tables.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$Image,[Parameter(Mandatory=$true)][string]$Payload,[switch]$PassThru)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
if(-not [BitConverter]::IsLittleEndian){throw 'Checker requires a little-endian host.'}
$bytes=[IO.File]::ReadAllBytes([IO.Path]::GetFullPath($Image))
$expected=[IO.File]::ReadAllBytes([IO.Path]::GetFullPath($Payload))
function Fail([string]$reason){throw "Boot media rejected: $reason"}
function U16([long]$at){[uint32][BitConverter]::ToUInt16($bytes,[int]$at)}
function U32([long]$at){[BitConverter]::ToUInt32($bytes,[int]$at)}
function U64([long]$at){[BitConverter]::ToUInt64($bytes,[int]$at)}
$sha=[Security.Cryptography.SHA256]::Create()
$zeroHash=@{}
function Hash([byte[]]$data,[long]$at,[long]$count){[BitConverter]::ToString($sha.ComputeHash($data,[int]$at,[int]$count))}
function Zero([long]$at,[long]$count,[string]$reason) {
    if($count -le 0){return}
    if(-not $zeroHash.ContainsKey($count)){$zeroHash[$count]=Hash (New-Object byte[] $count) 0 $count}
    if((Hash $bytes $at $count) -ne $zeroHash[$count]){Fail $reason}
}
function Same([long]$a,[long]$b,[long]$count,[string]$reason){if((Hash $bytes $a $count) -ne (Hash $bytes $b $count)){Fail $reason}}
$crcTable=New-Object 'uint32[]' 256
for($n=0;$n -lt 256;++$n){$c=[uint32]$n; for($k=0;$k -lt 8;++$k){if($c -band 1){$c=[uint32](($c -shr 1) -bxor 0xEDB88320L)}else{$c=[uint32]($c -shr 1)}}; $crcTable[$n]=$c}
function Crc([byte[]]$data,[long]$at,[long]$count) {
    $c=[uint32]0xFFFFFFFFL
    for($i=$at;$i -lt $at+$count;++$i){$c=[uint32]($crcTable[($c -bxor $data[$i]) -band 0xFF] -bxor ($c -shr 8))}
    [uint32]($c -bxor 0xFFFFFFFFL)
}
function Guid([long]$at){[BitConverter]::ToString($bytes,[int]$at,16)}
$S=512
if($bytes.Length -ne 67108864){Fail 'Image size'}
$total=[long]($bytes.Length/$S)
# Protective MBR (UEFI 5.2.3, Tables 5.3/5.4).
if($bytes[510] -ne 0x55 -or $bytes[511] -ne 0xAA){Fail 'MBR signature'}
Zero 0 446 'MBR boot code/signature fields'
$r=446
if($bytes[$r] -ne 0 -or $bytes[$r+1] -ne 0 -or $bytes[$r+2] -ne 2 -or $bytes[$r+3] -ne 0 -or $bytes[$r+4] -ne 0xEE -or
    $bytes[$r+5] -ne 0xFF -or $bytes[$r+6] -ne 0xFF -or $bytes[$r+7] -ne 0xFF -or (U32 ($r+8)) -ne 1 -or (U32 ($r+12)) -ne $total-1){Fail 'Protective record'}
Zero 462 48 'Unused MBR records'
# Primary and backup GPT headers and entry arrays (UEFI 5.3.2/5.3.3).
$espType='28-73-2A-C1-1F-F8-D2-11-BA-4B-00-A0-C9-3E-C9-3B'
$headers=@()
foreach($lba in @(1,($total-1))) {
    $h=$lba*$S
    if([Text.Encoding]::ASCII.GetString($bytes,[int]$h,8) -ne 'EFI PART' -or (U32 ($h+8)) -ne 0x00010000 -or (U32 ($h+12)) -ne 92){Fail 'GPT header identity'}
    $copy=New-Object byte[] 92; [Array]::Copy($bytes,$h,$copy,0,92); for($i=16;$i -lt 20;++$i){$copy[$i]=0}
    if((Crc $copy 0 92) -ne (U32 ($h+16))){Fail 'GPT header CRC'}
    if((U32 ($h+20)) -ne 0){Fail 'GPT header reserved'}
    Zero ($h+92) ($S-92) 'GPT header reserved'
    $other=if($lba -eq 1){$total-1}else{1}
    if((U64 ($h+24)) -ne $lba -or (U64 ($h+32)) -ne $other){Fail 'GPT header location'}
    $count=U32 ($h+80); $size=U32 ($h+84); $array=[long](U64 ($h+72)); $arrayBytes=[long]$count*$size
    if($size -ne 128 -or $count -ne 128 -or $arrayBytes -lt 16384){Fail 'GPT entry geometry'}
    $first=U64 ($h+40); $last=U64 ($h+48); $arraySectors=[long][Math]::Ceiling($arrayBytes/$S)
    if($first -lt 34 -or $last -ge $total-1-$arraySectors -or $first -gt $last){Fail 'GPT usable range'}
    if(($lba -eq 1 -and $array -ne 2) -or ($lba -ne 1 -and $array -ne $last+1)){Fail 'GPT entry location'}
    if($lba -eq 1 -and $array+$arraySectors -gt $first){Fail 'GPT entry location'}
    if((Crc $bytes ($array*$S) $arrayBytes) -ne (U32 ($h+88))){Fail 'GPT entry CRC'}
    $headers += [pscustomobject]@{lba=$lba;array=$array;bytes=$arrayBytes;first=$first;last=$last;guid=(Guid ($h+56));crc=(U32 ($h+16))}
}
if($headers[0].guid -ne $headers[1].guid -or $headers[0].first -ne $headers[1].first -or $headers[0].last -ne $headers[1].last){Fail 'GPT twin fields'}
if($headers[0].guid -eq ('00-'*15+'00')){Fail 'GPT disk GUID'}
Same ($headers[0].array*$S) ($headers[1].array*$S) $headers[0].bytes 'Backup entries'
$e=$headers[0].array*$S
if((Guid $e) -ne $espType){Fail 'ESP type'}
if((Guid ($e+16)) -eq ('00-'*15+'00') -or (Guid ($e+16)) -eq $headers[0].guid){Fail 'Partition GUID'}
$start=[long](U64 ($e+32)); $end=[long](U64 ($e+40))
if($start % 2048 -ne 0 -or $start -lt $headers[0].first -or $end -gt $headers[0].last -or $end -lt $start){Fail 'Partition bounds'}
if((U64 ($e+48)) -ne 0){Fail 'Partition attributes'}
Zero ($e+128) ($headers[0].bytes-128) 'Unused entries'
$name=[Text.Encoding]::Unicode.GetString($bytes,[int]($e+56),72).TrimEnd([char]0)
# FAT32 boot sector/BPB (Microsoft FAT32 v1.03, sector 0 tables).
$v=$start*$S; $partSectors=$end-$start+1
if(-not (($bytes[$v] -eq 0xEB -and $bytes[$v+2] -eq 0x90) -or $bytes[$v] -eq 0xE9)){Fail 'Boot sector jump'}
if((U16 ($v+11)) -ne 512 -or $bytes[$v+13] -ne 1 -or (U16 ($v+14)) -eq 0 -or $bytes[$v+16] -ne 2 -or (U16 ($v+17)) -ne 0 -or
    (U16 ($v+19)) -ne 0 -or $bytes[$v+21] -ne 0xF8 -or (U16 ($v+22)) -ne 0 -or (U32 ($v+28)) -ne $start -or (U32 ($v+32)) -ne $partSectors){Fail 'Boot sector BPB'}
if((U16 ($v+40)) -ne 0 -or (U16 ($v+42)) -ne 0 -or (U32 ($v+44)) -ne 2 -or (U16 ($v+48)) -ne 1 -or (U16 ($v+50)) -ne 6){Fail 'Boot sector FAT32 fields'}
Zero ($v+52) 12 'Boot sector reserved'
if($bytes[$v+65] -ne 0 -or $bytes[$v+66] -ne 0x29 -or [Text.Encoding]::ASCII.GetString($bytes,[int]($v+82),8) -ne 'FAT32   ' -or
    $bytes[$v+510] -ne 0x55 -or $bytes[$v+511] -ne 0xAA){Fail 'Boot sector signature'}
$reserved=[long](U16 ($v+14)); $fatSize=[long](U32 ($v+36)); $data=$reserved+2*$fatSize
$clusters=[long]($partSectors-$data)
if($clusters -lt 65525+16){Fail 'FAT type'}
if(($clusters+2)*4 -gt $fatSize*$S){Fail 'FAT size'}
if(($start+$data) % 2048 -ne 0){Fail 'Data alignment'}
$fsi=$v+$S
if((U32 $fsi) -ne 0x41615252 -or (U32 ($fsi+484)) -ne 0x61417272 -or (U32 ($fsi+508)) -ne 0xAA550000L){Fail 'FSInfo signature'}
Zero ($fsi+4) 480 'FSInfo reserved'; Zero ($fsi+496) 12 'FSInfo reserved'
Same $v ($v+6*$S) $S 'Backup boot sector'; Same $fsi ($v+7*$S) $S 'Backup FSInfo'
$fat=$v+$reserved*$S
Same $fat ($fat+$fatSize*$S) ($fatSize*$S) 'FAT mirror'
function Fat([long]$n){[long]((U32 ($fat+4*$n)) -band 0x0FFFFFFF)}
if((U32 $fat) -ne 0x0FFFFFF8 -or (U32 ($fat+4)) -ne 0x0FFFFFFF){Fail 'Reserved FAT entries'}
function Cluster([long]$n){$v+($data+$n-2)*$S}
function Find([long]$dir,[string]$entry,[int]$attr) {
    for($i=0;$i -lt 16;++$i){
        $d=$dir+32*$i
        if($bytes[$d] -eq 0){break}
        if([Text.Encoding]::ASCII.GetString($bytes,[int]$d,11) -eq $entry -and $bytes[$d+11] -eq $attr){return $d}
    }
    Fail "Directory entry $($entry.Trim())"
}
function First([long]$d){[long](((U16 ($d+20)) -shl 16) -bor (U16 ($d+26)))}
function OneCluster([long]$n,[string]$what){if($n -lt 2 -or $n -ge $clusters+2 -or (Fat $n) -lt 0x0FFFFFF8){Fail "$what chain"}}
$root=[long](U32 ($v+44)); OneCluster $root 'Root'
$efi=Find (Cluster $root) 'EFI        ' 0x10; $efiCluster=First $efi; OneCluster $efiCluster 'EFI directory'
$efiDir=Cluster $efiCluster
if([Text.Encoding]::ASCII.GetString($bytes,[int]$efiDir,11) -ne '.          ' -or (First $efiDir) -ne $efiCluster -or
    [Text.Encoding]::ASCII.GetString($bytes,[int]($efiDir+32),11) -ne '..         ' -or (First ($efiDir+32)) -ne 0){Fail 'EFI dot entries'}
$boot=Find $efiDir 'BOOT       ' 0x10; $bootCluster=First $boot; OneCluster $bootCluster 'BOOT directory'
$bootDir=Cluster $bootCluster
if((First $bootDir) -ne $bootCluster -or (First ($bootDir+32)) -ne $efiCluster){Fail 'BOOT dot entries'}
$file=Find $bootDir 'BOOTX64 EFI' 0x20
if((U32 ($file+28)) -ne $expected.Length){Fail 'Payload size'}
$firstCluster=First $file; $used=[long][Math]::Ceiling($expected.Length/$S)
for($i=0;$i -lt $used;++$i){
    $n=$firstCluster+$i
    if($n -lt 2 -or $n -ge $clusters+2){Fail 'Payload chain'}
    $link=Fat $n
    if(($i -lt $used-1 -and $link -ne $n+1) -or ($i -eq $used-1 -and $link -lt 0x0FFFFFF8)){Fail 'Payload chain'}
}
$fileAt=Cluster $firstCluster
if((Hash $bytes $fileAt $expected.Length) -ne (Hash $expected 0 $expected.Length)){Fail 'Payload bytes'}
Zero ($fileAt+$expected.Length) ($used*$S-$expected.Length) 'Payload slack'
# Exactly root, EFI, BOOT and the payload are allocated, in that order.
if($root -ne 2 -or $efiCluster -ne 3 -or $bootCluster -ne 4 -or $firstCluster -ne 5){Fail 'Allocation order'}
$allocatedEnd=5+$used
Zero ($fat+4*$allocatedEnd) ($fatSize*$S-4*$allocatedEnd) 'FAT free region'
if((U32 ($fsi+488)) -ne $clusters-3-$used -or (U32 ($fsi+492)) -ne $allocatedEnd){Fail 'FSInfo counts'}
# Every byte outside the described structures is zero.
Zero (3*$S) (($headers[0].first-3)*$S) 'Unexpected nonzero data'
Zero ($e+$S) ($headers[0].bytes-$S) 'Unexpected nonzero data'
Zero ($headers[0].first*$S) (($start-$headers[0].first)*$S) 'Unexpected nonzero data'
Zero ($v+2*$S) (4*$S) 'Unexpected nonzero data'
Zero ($v+8*$S) (($reserved-8)*$S) 'Unexpected nonzero data'
Zero ((Cluster 2)+32) ($S-32) 'Unexpected nonzero data'; Zero ((Cluster 3)+96) ($S-96) 'Unexpected nonzero data'
Zero ((Cluster 4)+96) ($S-96) 'Unexpected nonzero data'
Zero (Cluster $allocatedEnd) (($v+$partSectors*$S)-(Cluster $allocatedEnd)) 'Unexpected nonzero data'
Zero (($end+1)*$S) (($headers[1].array-$end-1)*$S) 'Unexpected nonzero data'
Zero ($headers[1].array*$S+$S) (($total-1-$headers[1].array-1)*$S) 'Unexpected nonzero data'
Zero ($headers[1].array*$S+128) ($S-128) 'Unexpected nonzero data'
$result=[ordered]@{image_sha256=(Get-FileHash -LiteralPath $Image -Algorithm SHA256).Hash.ToLowerInvariant();
    payload_sha256=(Get-FileHash -LiteralPath $Payload -Algorithm SHA256).Hash.ToLowerInvariant();
    image_bytes=$bytes.Length;partition_first=$start;partition_sectors=$partSectors;partition_name=$name;
    reserved_sectors=$reserved;fat_sectors=$fatSize;clusters=$clusters;payload_clusters=$used;
    free_clusters=$clusters-3-$used;header_crc=('{0:X8}' -f $headers[0].crc);backup_crc=('{0:X8}' -f $headers[1].crc);loaded=$false}
Write-Output "Boot media PASS: protective MBR, twin GPT, FAT32 ESP and \EFI\BOOT\BOOTX64.EFI ($($expected.Length) bytes) match; not booted."
if($PassThru){[pscustomobject]$result}
