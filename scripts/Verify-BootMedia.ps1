# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
# SPEC-0014 retains the entire prior wrapper chain, then packages the audited O2
# payload with every hosted build of the writer and checks the image independently.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,[switch]$Sanitizers)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$BuildRoot=[IO.Path]::GetFullPath($BuildRoot)
& (Join-Path $PSScriptRoot 'Verify-UARTDiagnostics.ps1') -BuildRoot $BuildRoot -Sanitizers:$Sanitizers
$hashes=[ordered]@{}
foreach($artifact in @('classick_boot_media.exe','classick_boot_media_tests.exe')) {
    foreach($pair in @(@('clang','clang-hosted/debug-a','clang-hosted/debug-b'),@('gcc','gcc-debug-a','gcc-debug-b'))) {
        $a=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[1]+'/'+$artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
        $b=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ($pair[2]+'/'+$artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
        if($a -ne $b){throw "Boot-media hosted twins differ: $($pair[0]) $artifact"}
        $hashes["$($pair[0]) $artifact"]=$a; Write-Output "Matching boot-media SHA-256 $($pair[0]) ${artifact}: $a"
    }
}
$payload=Join-Path $BuildRoot 'efi/payload/EFI/BOOT/BOOTX64.EFI'
$payloadHash=(Get-FileHash -LiteralPath $payload -Algorithm SHA256).Hash.ToLowerInvariant()
$mediaRoot=Join-Path $BuildRoot 'media'
if(Test-Path -LiteralPath $mediaRoot){throw 'Use a fresh media output directory.'}
[void][IO.Directory]::CreateDirectory($mediaRoot)
$configs=@('clang-hosted/debug-a','clang-hosted/debug-b','clang-hosted/release','clang-hosted/i686','gcc-debug-a','gcc-debug-b','gcc-release','gcc-i686')
if($Sanitizers){$configs+='clang-hosted/sanitized'}
$images=[ordered]@{}; $reference=$null
# The sanitized writer needs the pinned toolchain's runtime DLLs on PATH.
$originalPath=$env:PATH
try {
    & (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
    foreach($config in $configs) {
        $tool=Join-Path $BuildRoot ($config+'/classick_boot_media.exe')
        $image=Join-Path $mediaRoot (($config -replace '[/\\]','-')+'.img')
        & $tool $payload $image
        if($LASTEXITCODE -ne 0){throw "Boot-media generation failed: $config"}
        $images[$config]=(Get-FileHash -LiteralPath $image -Algorithm SHA256).Hash.ToLowerInvariant()
        if($null -eq $reference){$reference=$image}
        elseif($images[$config] -ne $images[$configs[0]]){throw "Boot-media image differs across builds: $config"}
    }
} finally { $env:PATH=$originalPath }
Write-Output "Matching boot-media image SHA-256 across $($configs.Count) writer builds: $($images[$configs[0]])"
$check=@(& (Join-Path $PSScriptRoot 'Test-BootMedia.ps1') -Image $reference -Payload $payload -PassThru)[-1]
if($check.image_sha256 -ne $images[$configs[0]] -or $check.payload_sha256 -ne $payloadHash){throw 'Checker result mismatch.'}

# Tool refusals: existing output, non-MZ payload and oversize payload.
$tool=Join-Path $BuildRoot 'clang-hosted/debug-a/classick_boot_media.exe'
$previous=$ErrorActionPreference; $refusals=0
try {
    $ErrorActionPreference='Continue'
    $null=& $tool $payload $reference 2>&1
    if($LASTEXITCODE -eq 0 -or (Get-FileHash -LiteralPath $reference -Algorithm SHA256).Hash.ToLowerInvariant() -ne $images[$configs[0]]){throw 'Existing output was not preserved.'}
    ++$refusals
    $notPe=Join-Path $mediaRoot 'not-pe.bin'; [IO.File]::WriteAllBytes($notPe,[byte[]](0x4E,0x4F,0x50,0x45))
    $null=& $tool $notPe (Join-Path $mediaRoot 'not-pe.img') 2>&1
    if($LASTEXITCODE -eq 0 -or (Test-Path -LiteralPath (Join-Path $mediaRoot 'not-pe.img'))){throw 'Non-MZ payload was accepted.'}
    ++$refusals
    $huge=Join-Path $mediaRoot 'oversize.bin'; $bytes=New-Object byte[] (63961600+1); $bytes[0]=0x4D; $bytes[1]=0x5A
    [IO.File]::WriteAllBytes($huge,$bytes); $bytes=$null
    $null=& $tool $huge (Join-Path $mediaRoot 'oversize.img') 2>&1
    if($LASTEXITCODE -eq 0 -or (Test-Path -LiteralPath (Join-Path $mediaRoot 'oversize.img'))){throw 'Oversize payload was accepted.'}
    ++$refusals; Remove-Item -LiteralPath $huge
} finally { $ErrorActionPreference=$previous }
$global:LASTEXITCODE=0

# Corruption controls. GPT entry semantics are re-sealed so the checker's field
# rules, not only its CRCs, must reject them.
$original=[IO.File]::ReadAllBytes($reference); $S=512; $total=131072; $v=2048*$S
$fat1=$v+78*$S; $fat2=$fat1+985*$S; $data=$v+2048*$S
function ClusterAt([int]$n){$data+($n-2)*$S}
$crcTable=New-Object 'uint32[]' 256
for($n=0;$n -lt 256;++$n){$c=[uint32]$n; for($k=0;$k -lt 8;++$k){if($c -band 1){$c=[uint32](($c -shr 1) -bxor 0xEDB88320L)}else{$c=[uint32]($c -shr 1)}}; $crcTable[$n]=$c}
function Crc([byte[]]$b,[long]$at,[long]$count){$c=[uint32]0xFFFFFFFFL; for($i=$at;$i -lt $at+$count;++$i){$c=[uint32]($crcTable[($c -bxor $b[$i]) -band 0xFF] -bxor ($c -shr 8))}; [uint32]($c -bxor 0xFFFFFFFFL)}
function Reseal([byte[]]$b) {
    foreach($lba in @(1,($total-1))) {
        $h=$lba*$S; $array=[BitConverter]::ToUInt64($b,$h+72)
        [Array]::Copy([BitConverter]::GetBytes([uint32](Crc $b ($array*$S) 16384)),0,$b,$h+88,4)
        for($i=16;$i -lt 20;++$i){$b[$h+$i]=0}
        [Array]::Copy([BitConverter]::GetBytes([uint32](Crc $b $h 92)),0,$b,$h+16,4)
    }
}
$entries=@((2*$S),(131039*$S)); $freeEntry=5+[int]$check.payload_clusters+8
function M([string]$name,[long[]]$at,[byte[]]$value,[string]$reason,[switch]$Seal){[pscustomobject]@{name=$name;at=$at;value=$value;reason=$reason;seal=[bool]$Seal}}
$mutations=@(
    (M 'mbr-signature' @(510) @(0xFF) 'MBR signature'),
    (M 'mbr-code' @(0) @(0x90) 'MBR boot code'),
    (M 'protective-type' @(450) @(0x07) 'Protective record'),
    (M 'gpt-signature' @(512) @(0x58) 'GPT header identity'),
    (M 'gpt-header-crc' @(528) @(0xA5) 'GPT header CRC'),
    (M 'gpt-entry-crc' @($entries[0]+56) @(0x44) 'GPT entry CRC'),
    (M 'backup-header-crc' @((($total-1)*$S)+16) @(0xA5) 'GPT header CRC'),
    (M 'backup-entry-crc' @($entries[1]+56) @(0x44) 'GPT entry CRC'),
    (M 'esp-type' @($entries[0],$entries[1]) @(0xFF) 'ESP type' -Seal),
    (M 'partition-unaligned' @(($entries[0]+32),($entries[1]+32)) @(1) 'Partition bounds' -Seal),
    (M 'partition-attributes' @(($entries[0]+48),($entries[1]+48)) @(2) 'Partition attributes' -Seal),
    (M 'bpb-sector-size' @(($v+12)) @(4) 'Boot sector BPB'),
    (M 'bpb-fats' @(($v+16)) @(1) 'Boot sector BPB'),
    (M 'fat-type' @(($v+37)) @(0x79) 'FAT type'),
    (M 'data-alignment' @(($v+14)) @(79) 'Data alignment'),
    (M 'fs-type' @(($v+82)) @(0x58) 'Boot sector signature'),
    (M 'vbr-signature' @(($v+510)) @(0xFF) 'Boot sector signature'),
    (M 'backup-boot' @(($v+6*$S+3)) @(0x58) 'Backup boot sector'),
    (M 'fsinfo-signature' @(($v+$S)) @(0xFF) 'FSInfo signature'),
    (M 'fsinfo-count' @(($v+$S+488),($v+7*$S+488)) @(1) 'FSInfo counts'),
    (M 'fat-mirror' @(($fat2+20)) @(1) 'FAT mirror'),
    (M 'fat-reserved' @($fat1,$fat2) @(0xF0) 'Reserved FAT entries'),
    (M 'payload-chain' @(($fat1+24),($fat2+24)) @(0xFF) 'Payload chain'),
    (M 'fat-free' @(($fat1+4*$freeEntry),($fat2+4*$freeEntry)) @(1) 'FAT free region'),
    (M 'directory-name' @((ClusterAt 2)) @(0x46) 'Directory entry EFI'),
    (M 'payload-size' @(((ClusterAt 4)+64+28)) @(1) 'Payload size'),
    (M 'payload-byte' @(((ClusterAt 5)+4096)) @(0x5A) 'Payload bytes'),
    (M 'stray-gap' @((100*$S)) @(1) 'Unexpected nonzero data'),
    (M 'stray-data' @((($v+126976*$S)-1)) @(1) 'Unexpected nonzero data'))
$controls=Join-Path $mediaRoot 'controls'; [void][IO.Directory]::CreateDirectory($controls)
foreach($mutation in $mutations) {
    $b=[byte[]]$original.Clone()
    foreach($at in $mutation.at){ for($i=0;$i -lt $mutation.value.Length;++$i){ $b[$at+$i]=[byte]($b[$at+$i] -bxor $mutation.value[$i]) } }
    if($mutation.seal){Reseal $b}
    $path=Join-Path $controls ($mutation.name+'.img'); [IO.File]::WriteAllBytes($path,$b); $rejected=$false
    try{& (Join-Path $PSScriptRoot 'Test-BootMedia.ps1') -Image $path -Payload $payload | Out-Null}
    catch{if($_.Exception.Message -notmatch [regex]::Escape($mutation.reason)){throw "Control $($mutation.name) rejected for the wrong reason: $($_.Exception.Message)"}; $rejected=$true}
    if(-not $rejected){throw "Boot-media control accepted: $($mutation.name)"}
    Remove-Item -LiteralPath $path
}
$short=Join-Path $controls 'truncated.img'; $cut=New-Object byte[] ($original.Length-512); [Array]::Copy($original,$cut,$cut.Length)
[IO.File]::WriteAllBytes($short,$cut); $cut=$null; $rejected=$false
try{& (Join-Path $PSScriptRoot 'Test-BootMedia.ps1') -Image $short -Payload $payload | Out-Null}catch{if($_.Exception.Message -notmatch 'Image size'){throw}; $rejected=$true}
if(-not $rejected){throw 'Truncated image accepted.'}
Remove-Item -LiteralPath $short
[ordered]@{image=$check;writer_builds=$images;payload='efi/payload/EFI/BOOT/BOOTX64.EFI';payload_sha256=$payloadHash;
    corruption_rejections=$mutations.Count+1;tool_refusals=$refusals;hosted=$hashes;loaded=$false;firmware=$false} |
    ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $BuildRoot 'boot-media-results.json') -Encoding UTF8
Write-Output "Boot media PASS: $($configs.Count) writer builds produce one image $($images[$configs[0]]); independent check, $($mutations.Count+1) corruption rejections and $refusals tool refusals; no image booted."
