# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$taskRepo=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$taskBuild=[IO.Path]::GetFullPath($BuildRoot)
if(Test-Path -LiteralPath $taskBuild) {throw 'Choose a fresh Mac verification directory'}
$null=New-Item -ItemType Directory -Path $taskBuild
$taskClang=Join-Path $taskRepo '.tools/llvm-mingw/llvm-mingw-20260908-ucrt-x86_64/bin/clang.exe'
$taskGcc=Join-Path $taskRepo '.tools/gcc/w64devkit/bin/gcc.exe'
$taskSources=@('tests/mac-rom.c','arch/m68k/runtime.c','platform/mac128k/loop.c',
    'apps/rom-scene/scene.c','core/graphics/surface.c') | ForEach-Object {Join-Path $taskRepo $_}
$taskCommon=@('-std=c11','-ffreestanding','-fno-builtin','-Wall','-Wextra','-Werror',
    '-Wconversion','-Wsign-conversion',('-ffile-prefix-map='+$taskRepo+'=.'),
    ('-ffile-prefix-map='+$taskBuild+'=.'))
$taskProfiles=@(
    @{name='clang-x64-o0';compiler=$taskClang;flags=@('--target=x86_64-w64-mingw32','-O0')},
    @{name='clang-x64-o2';compiler=$taskClang;flags=@('--target=x86_64-w64-mingw32','-O2')},
    @{name='clang-x86-o2';compiler=$taskClang;flags=@('--target=i686-w64-mingw32','-O2')},
    @{name='gcc-x64-o2';compiler=$taskGcc;flags=@('-O2','-fno-tree-loop-distribute-patterns')}
)
$taskRows=@()
foreach($taskProfile in $taskProfiles) {
    $taskExe=Join-Path $taskBuild ($taskProfile.name+'.exe')
    $taskArgs=$taskCommon+$taskProfile.flags+$taskSources+@('-Wl,--no-insert-timestamp','-o',$taskExe)
    & $taskProfile.compiler @taskArgs
    if($LASTEXITCODE -ne 0) {throw ('Compiler failed: '+$taskProfile.name)}
    foreach($taskSuite in @('helpers','controller','mouse','clock','pixels','crc')) {
        & $taskExe $taskSuite
        if($LASTEXITCODE -ne 0) {throw ($taskProfile.name+' failed '+$taskSuite)}
        Write-Output ($taskProfile.name+' '+$taskSuite+' PASS')
    }
    & $taskExe zero
    if($LASTEXITCODE -eq 0) {throw 'Zero-divisor fault control unexpectedly returned'}
    $taskRows+=@{profile=$taskProfile.name;checks=6;zero_divisor_exit=$LASTEXITCODE;
        sha256=(Get-FileHash -LiteralPath $taskExe -Algorithm SHA256).Hash.ToLowerInvariant()}
}
$taskJson=@{specification='SPEC-0018';profiles=$taskRows;checks=24;zero_divisor_controls=4;emulator_run=$false} | ConvertTo-Json -Depth 8
[IO.File]::WriteAllText((Join-Path $taskBuild 'manifest.json'),$taskJson,[Text.UTF8Encoding]::new($false))
Write-Output 'Mac ROM hosted verification PASS: 24 suites in four profiles; four zero-divisor controls.'
# Expected child faults have already been checked and recorded above. Do not
# propagate the final control's nonzero status through PowerShell's CI wrapper.
$global:LASTEXITCODE=0
