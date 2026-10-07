[CmdletBinding()]
param()
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$lock = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'toolchain-lock.json') -Raw | ConvertFrom-Json
$bins = @()
foreach ($tool in $lock.tools) {
    $bin = Join-Path (Join-Path (Join-Path $repoRoot '.tools') $tool.id) $tool.bin
    if (-not (Test-Path -LiteralPath (Join-Path $bin $tool.executable))) {
        throw "Missing $($tool.id). Run scripts\Setup-Windows.ps1 first."
    }
    $bins += $bin
}
$env:PATH = ($bins -join [IO.Path]::PathSeparator) + [IO.Path]::PathSeparator + $env:PATH
Write-Output 'Pinned tools are active in this PowerShell process.'
