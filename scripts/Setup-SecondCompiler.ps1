# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([switch]$Offline)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$ProgressPreference='SilentlyContinue'
$repoRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$lock=Get-Content -LiteralPath (Join-Path $PSScriptRoot 'gcc-toolchain-lock.json') -Raw | ConvertFrom-Json
$destination=Join-Path $repoRoot '.tools/gcc'
$archive=Join-Path $repoRoot ('.downloads/'+$lock.archive)
function Check-Tools([string]$root) {
    foreach($item in @(@('gcc.exe',$lock.compilerSha256),@('ld.exe',$lock.linkerSha256))) {
        $file=Join-Path (Join-Path $root $lock.bin) $item[0]
        if(-not (Test-Path -LiteralPath $file) -or
                (Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash.ToLowerInvariant() -ne $item[1]) {
            throw "Unexpected second compiler tool: $($item[0])"
        }
    }
}
if(Test-Path -LiteralPath $destination) {
    Check-Tools $destination
    Write-Output 'Pinned GCC and GNU ld already prepared; system settings unchanged.'
    return
}
[void][IO.Directory]::CreateDirectory((Split-Path -Parent $archive))
if(-not (Test-Path -LiteralPath $archive)) {
    if($Offline) { throw 'Second compiler archive absent from offline cache.' }
    [Net.ServicePointManager]::SecurityProtocol=[Net.SecurityProtocolType]::Tls12
    Invoke-WebRequest -UseBasicParsing -Uri $lock.url -OutFile ($archive+'.partial') -TimeoutSec 300
    if((Get-FileHash -LiteralPath ($archive+'.partial') -Algorithm SHA256).Hash.ToLowerInvariant() -ne $lock.sha256) { throw 'Second compiler download hash mismatch.' }
    Move-Item -LiteralPath ($archive+'.partial') -Destination $archive
}
if((Get-Item -LiteralPath $archive).Length -ne $lock.sizeBytes -or
        (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant() -ne $lock.sha256) { throw 'Second compiler cache mismatch.' }
$tar=Join-Path $env:SystemRoot 'System32/tar.exe'
if(-not (Test-Path -LiteralPath $tar)) { throw 'Windows archive extractor unavailable.' }
$staging=Join-Path $repoRoot ('.tools/.staging-gcc-'+[Guid]::NewGuid().ToString('N'))
[void][IO.Directory]::CreateDirectory($staging)
$prefix=[IO.Path]::GetFullPath($staging)+[IO.Path]::DirectorySeparatorChar
$entries=@(& $tar -tf $archive)
if($LASTEXITCODE -ne 0) { throw 'Cannot inventory compiler archive.' }
foreach($entry in $entries) {
    $resolved=[IO.Path]::GetFullPath((Join-Path $staging $entry))
    if(-not $resolved.StartsWith($prefix,[StringComparison]::OrdinalIgnoreCase)) { throw 'Compiler archive path escapes staging.' }
}
& $tar -xf $archive -C $staging
if($LASTEXITCODE -ne 0) { throw 'Compiler extraction failed; staging preserved.' }
Check-Tools $staging
Move-Item -LiteralPath $staging -Destination $destination
Write-Output 'Pinned portable GCC prepared in this checkout; no system PATH/registry changes.'
