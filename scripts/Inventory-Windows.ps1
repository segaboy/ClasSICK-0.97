[CmdletBinding()]
param()
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$names = @('git','gh','winget','pwsh','powershell','cl','clang','clang-cl','gcc',
    'cmake','ninja','make','python','python3','py','rustc','cargo','qemu-system-x86_64',
    'qemu-system-m68k','nasm','m68k-elf-gcc','wsl','docker','node')
$tools = foreach ($name in $names) {
    $command = Get-Command $name -ErrorAction SilentlyContinue | Select-Object -First 1
    $state = if ($command) { 'found-on-PATH' } else { 'not-found-on-PATH' }
    if ($command -and $name -match '^python3?$' -and $command.Source -match '\\WindowsApps\\') {
        $state = 'Windows-launch-alias-not-a-verified-interpreter'
    }
    [ordered]@{ name = $name; state = $state }
}
$commonLocations = @{
    vswhere = 'C:\Program Files\Microsoft Visual Studio\Installer\vswhere.exe'
    llvm = 'C:\Program Files\LLVM\bin\clang.exe'
    cmake = 'C:\Program Files\CMake\bin\cmake.exe'
    msys2ucrt = 'C:\msys64\ucrt64\bin\gcc.exe'
    msys2mingw = 'C:\msys64\mingw64\bin\gcc.exe'
}
$locations = foreach ($key in $commonLocations.Keys | Sort-Object) {
    [ordered]@{ location = $key; present = [bool](Test-Path -LiteralPath $commonLocations[$key]) }
}
$windows = Get-ItemProperty -LiteralPath 'HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion'
[ordered]@{
    inventoryVersion = 1
    operatingSystemVersion = [Environment]::OSVersion.Version.ToString()
    registryProductName = $windows.ProductName
    displayVersion = $windows.DisplayVersion
    build = $windows.CurrentBuildNumber
    processIs64Bit = [Environment]::Is64BitProcess
    powershellVersion = $PSVersionTable.PSVersion.ToString()
    tools = @($tools)
    commonLocations = @($locations)
    caveat = 'PATH and common-location probes are not an exhaustive installed-software inventory. WSL launcher presence does not prove a distribution exists. No account names, tokens, device IDs, or absolute user paths are collected.'
} | ConvertTo-Json -Depth 5
