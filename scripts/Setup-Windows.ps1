# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
# Requires only Windows PowerShell 5.1+ and .NET supplied with Windows 11.
[CmdletBinding()]
param([switch]$Offline)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
# Windows PowerShell 5.1 progress rendering can dominate large archive downloads.
$ProgressPreference = 'SilentlyContinue'
if ([Environment]::OSVersion.Platform -ne [PlatformID]::Win32NT -or
    [Environment]::GetEnvironmentVariable('PROCESSOR_ARCHITECTURE') -ne 'AMD64') {
    throw 'This pinned bootstrap supports x64 Windows only.'
}
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$lock = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'toolchain-lock.json') -Raw | ConvertFrom-Json
$cacheRoot = Join-Path $repoRoot '.downloads'
$toolRoot = Join-Path $repoRoot '.tools'
[void][IO.Directory]::CreateDirectory($cacheRoot)
[void][IO.Directory]::CreateDirectory($toolRoot)
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
Add-Type -AssemblyName System.IO.Compression.FileSystem

foreach ($tool in $lock.tools) {
    $destination = Join-Path $toolRoot $tool.id
    $receipt = Join-Path $destination '.installed-sha256'
    $executable = Join-Path (Join-Path $destination $tool.bin) $tool.executable
    if ((Test-Path -LiteralPath $receipt) -and (Test-Path -LiteralPath $executable)) {
        if ((Get-Content -LiteralPath $receipt -Raw).Trim() -ne $tool.sha256) {
            throw "Installed $($tool.id) differs from the lock. Use a new checkout or review its tool directory."
        }
        Write-Output "$($tool.id) already prepared: $($tool.version)"
        continue
    }
    if (Test-Path -LiteralPath $destination) {
        throw "Incomplete tool directory: $destination. Inspect it before retrying; setup will not delete it."
    }
    $archive = Join-Path $cacheRoot $tool.archive
    if (-not (Test-Path -LiteralPath $archive)) {
        if ($Offline) { throw "Offline cache missing: $($tool.archive)" }
        Write-Output "Downloading $($tool.id): $($tool.purpose)"
        $partial = "$archive.partial"
        Invoke-WebRequest -UseBasicParsing -Uri $tool.url -OutFile $partial -TimeoutSec 300
        if ((Get-FileHash -LiteralPath $partial -Algorithm SHA256).Hash.ToLowerInvariant() -ne $tool.sha256) {
            throw "SHA-256 mismatch for $($tool.archive); nothing extracted."
        }
        Move-Item -LiteralPath $partial -Destination $archive
    }
    if ((Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant() -ne $tool.sha256) {
        throw "SHA-256 mismatch in cached $($tool.archive); nothing extracted."
    }
    $staging = Join-Path $toolRoot (".staging-" + $tool.id + '-' + [Guid]::NewGuid().ToString('N'))
    $stagingPrefix = [IO.Path]::GetFullPath($staging) + [IO.Path]::DirectorySeparatorChar
    $zip = [IO.Compression.ZipFile]::OpenRead($archive)
    try {
        foreach ($entry in $zip.Entries) {
            $entryPath = [IO.Path]::GetFullPath((Join-Path $staging $entry.FullName))
            if (-not $entryPath.StartsWith($stagingPrefix, [StringComparison]::OrdinalIgnoreCase)) {
                throw "Archive entry escapes the tool directory: $($tool.archive)"
            }
        }
    } finally { $zip.Dispose() }
    Write-Output "Extracting verified $($tool.id) $($tool.version)"
    [IO.Compression.ZipFile]::ExtractToDirectory($archive, $staging)
    $stagedExe = Join-Path (Join-Path $staging $tool.bin) $tool.executable
    if (-not (Test-Path -LiteralPath $stagedExe)) { throw "Expected tool missing in $($tool.archive)" }
    [IO.File]::WriteAllText((Join-Path $staging '.installed-sha256'), $tool.sha256)
    Move-Item -LiteralPath $staging -Destination $destination
}
Write-Output 'Portable tools prepared inside this checkout; no system PATH or registry changes.'
