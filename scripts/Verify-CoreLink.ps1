# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildRoot,[switch]$Sanitizers)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$repoRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$BuildRoot=[IO.Path]::GetFullPath($BuildRoot)
if(Test-Path -LiteralPath $BuildRoot) { throw 'Use a fresh output directory; existing evidence is preserved.' }
$gccLock=Get-Content -LiteralPath (Join-Path $PSScriptRoot 'gcc-toolchain-lock.json') -Raw | ConvertFrom-Json
$gcc=Join-Path $repoRoot ('.tools/gcc/'+$gccLock.bin+'/gcc.exe')
& (Join-Path $PSScriptRoot 'Setup-SecondCompiler.ps1') -Offline
& (Join-Path $PSScriptRoot 'Verify-Debugger.ps1') -BuildRoot (Join-Path $BuildRoot 'clang-hosted') -Sanitizers:$Sanitizers
$originalPath=$env:PATH
try {
    & (Join-Path $PSScriptRoot 'Enter-DevEnvironment.ps1')
    $clang=(Get-Command clang).Source
    $nm=(Get-Command llvm-nm).Source
    $reader=(Get-Command llvm-readobj).Source
    & $gcc --version
    foreach($name in @('gcc-debug-a','gcc-debug-b','gcc-release','gcc-i686')) {
        $directory=Join-Path $BuildRoot $name
        $compiler=if($name -eq 'gcc-i686'){Join-Path (Split-Path -Parent $gcc) 'i686-w64-mingw32-gcc.exe'}else{$gcc}
        $configuration=if($name -eq 'gcc-release'){'Release'}else{'Debug'}
        & cmake --preset windows-hosted -S $repoRoot -B $directory "-DCMAKE_C_COMPILER=$compiler" "-DCMAKE_BUILD_TYPE=$configuration" -DCLASSICK_WINDOWS_ADAPTERS=OFF
        if($LASTEXITCODE -ne 0){throw "GCC configure failed: $name"}
        & cmake --build $directory
        if($LASTEXITCODE -ne 0){throw "GCC build failed: $name"}
        & ctest --test-dir $directory --output-on-failure --no-tests=error
        if($LASTEXITCODE -ne 0){throw "GCC conformance failed: $name"}
    }
    $hostedHashes=[ordered]@{}
    foreach($artifact in @('classick_surface_tests.exe','classick_arena_tests.exe','classick_input_tests.exe','classick_clock_tests.exe','classick_core_link_tests.exe')) {
        $a=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ('gcc-debug-a/'+$artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
        $b=(Get-FileHash -LiteralPath (Join-Path $BuildRoot ('gcc-debug-b/'+$artifact)) -Algorithm SHA256).Hash.ToLowerInvariant()
        if($a -ne $b){throw "GCC hosted twins differ: $artifact"}
        $hostedHashes[$artifact]=$a
        Write-Output "Matching GCC SHA-256 ${artifact}: $a"
    }
    $sources=@('core/graphics/surface.c','core/memory/arena.c','core/input/input.c','core/time/clock.c','tools/core-link/probe.c')
    $common=@('-std=c11','-Wall','-Wextra','-Wpedantic','-Werror','-Wconversion','-Wsign-conversion',
        '-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-ident','-mno-red-zone','-mgeneral-regs-only',
        '-fno-asynchronous-unwind-tables','-fno-unwind-tables')
    $results=@(); $missingControls=0; $referenceImage=$null; $referenceMap=$null
    foreach($tool in @('clang','gcc')) {
        $compiler=if($tool -eq 'clang'){$clang}else{$gcc}
        foreach($bits in @(64,32)) { foreach($opt in @(0,2)) {
            $case="$tool-$bits-O$opt"
            $twins=@()
            foreach($repeat in @('a','b')) {
                $directory=Join-Path $BuildRoot ("links/$case-$repeat")
                [void][IO.Directory]::CreateDirectory($directory)
                $objects=@()
                for($i=0;$i -lt $sources.Count;++$i) {
                    $object=Join-Path $directory "$i.o"
                    & $compiler "-m$bits" "-O$opt" @common "-ffile-prefix-map=$repoRoot=." -c (Join-Path $repoRoot $sources[$i]) -o $object
                    if($LASTEXITCODE -ne 0){throw "Standalone compile failed: $case"}
                    if($i -lt 4) {
                        & cmake "-DNM=$nm" "-DOBJECT=$object" -P (Join-Path $PSScriptRoot 'Check-Freestanding.cmake')
                        if($LASTEXITCODE -ne 0){throw "Standalone core boundary failed: $case"}
                    }
                    $objects += $object
                }
                $entry=if($bits -eq 32){'_cs_link_entry'}else{'cs_link_entry'}
                $map=Join-Path $directory 'image.map'
                $image=Join-Path $directory 'core-link.exe'
                & $compiler "-m$bits" -nostdlib "-Wl,--entry,$entry,--subsystem,native,--no-insert-timestamp,--no-gc-sections" "-Wl,-Map,$map" @objects -o $image
                if($LASTEXITCODE -ne 0){throw "Standalone link failed: $case"}
                $audit=& (Join-Path $PSScriptRoot 'Test-LinkImage.ps1') -Image $image -Map $map -Compiler $tool -Bits $bits -PassThru
                $inspection=@(& $reader --file-headers --coff-imports $image)
                if($LASTEXITCODE -ne 0 -or ($inspection -join "`n") -match '(?m)^Import \{'){throw "Independent import inspection failed: $case"}
                $inspection | Set-Content -LiteralPath (Join-Path $directory 'headers.txt') -Encoding UTF8
                $twins += $audit
                if($case -eq 'clang-64-O0' -and $repeat -eq 'a') { $referenceImage=$image; $referenceMap=$map }
            }
            if($twins[0].sha256 -ne $twins[1].sha256){throw "Standalone twins differ: $case"}
            $results += [ordered]@{case=$case;sha256=$twins[0].sha256;size_bytes=$twins[0].size_bytes;empty_import_bytes=$twins[0].empty_import_bytes;status='passed'}
            Write-Output "Matching standalone SHA-256 ${case}: $($twins[0].sha256)"
        }
        # Each architecture must reject a real unresolved helper with default libraries excluded.
        $directory=Join-Path $BuildRoot "controls/$tool-$bits"
        [void][IO.Directory]::CreateDirectory($directory)
        $object=Join-Path $directory 'missing.o'
        & $compiler "-m$bits" @common -c (Join-Path $repoRoot 'tools/core-link/missing.c') -o $object
        if($LASTEXITCODE -ne 0){throw 'Missing-runtime control compile failed.'}
        $entry=if($bits -eq 32){'_cs_link_entry'}else{'cs_link_entry'}
        # Windows PowerShell 5.1 represents intentional native stderr as error records.
        $previousPreference=$ErrorActionPreference
        try {
            $ErrorActionPreference='Continue'
            $linkLog=@(& $compiler "-m$bits" -nostdlib "-Wl,--entry,$entry,--subsystem,native,--no-insert-timestamp" $object -o (Join-Path $directory 'must-not-link.exe') 2>&1)
        } finally { $ErrorActionPreference=$previousPreference }
        if($LASTEXITCODE -eq 0 -or ($linkLog -join "`n") -notmatch 'cs_missing_runtime'){throw 'Unresolved-runtime control failed to reject the intended helper.'}
        $linkLog | Out-File -LiteralPath (Join-Path $directory 'missing.log') -Encoding UTF8
        ++$missingControls
        }
    }
    function Expect-Rejection([string]$image,[string]$map,[string]$reason) {
        $rejected=$false
        try { & (Join-Path $PSScriptRoot 'Test-LinkImage.ps1') -Image $image -Map $map -Compiler clang -Bits 64 | Out-Null }
        catch { if($_.Exception.Message -notmatch $reason){throw}; $rejected=$true }
        if(-not $rejected){throw "Image control was incorrectly accepted: $reason"}
    }
    $controlRoot=Join-Path $BuildRoot 'controls/images'
    [void][IO.Directory]::CreateDirectory($controlRoot)
    $original=[IO.File]::ReadAllBytes($referenceImage)
    $pe=[BitConverter]::ToUInt32($original,60); $optional=$pe+24
    $mutations=@(
        [pscustomobject]@{name='machine';offset=$pe+4;bytes=[byte[]]@(0x4C,0x01);reason='Machine rejected'},
        [pscustomobject]@{name='zero-entry';offset=$optional+16;bytes=[byte[]]@(0,0,0,0);reason='Entry point rejected'},
        [pscustomobject]@{name='wrong-entry';offset=$optional+16;bytes=[BitConverter]::GetBytes([uint32]([BitConverter]::ToUInt32($original,$optional+16)+1));reason='Entry point differs'},
        [pscustomobject]@{name='directory';offset=$optional+120;bytes=[byte[]]@(255,255,255,255,20,0,0,0);reason='Directory RVA rejected'})
    foreach($mutation in $mutations) {
        $bytes=[byte[]]$original.Clone()
        [Array]::Copy($mutation.bytes,0,$bytes,[int]$mutation.offset,$mutation.bytes.Length)
        $image=Join-Path $controlRoot ($mutation.name+'.exe')
        [IO.File]::WriteAllBytes($image,$bytes)
        Expect-Rejection $image $referenceMap $mutation.reason
    }
    $importObject=Join-Path $controlRoot 'import.o'
    $importImage=Join-Path $controlRoot 'import.exe'
    $importMap=Join-Path $controlRoot 'import.map'
    & $clang -m64 @common -c (Join-Path $repoRoot 'tools/core-link/import.c') -o $importObject
    if($LASTEXITCODE -ne 0){throw 'Import control compile failed.'}
    & $clang -m64 -nostdlib '-Wl,--entry,cs_link_entry,--subsystem,native,--no-insert-timestamp' "-Wl,-Map,$importMap" $importObject -luser32 -o $importImage
    if($LASTEXITCODE -ne 0){throw 'Import control link failed.'}
    Expect-Rejection $importImage $importMap 'Imports/TLS rejected'
    [ordered]@{links=$results;gcc_hosted_hashes=$hostedHashes;unresolved_controls=$missingControls;image_rejections=5} |
        ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $BuildRoot 'core-link-results.json') -Encoding UTF8
    Write-Output 'Core linkage and second compiler PASS: 8 standalone configurations / 16 matching twin images, 4 unresolved-helper controls and 5 image rejections. Link fixtures were never loaded; no OS boot claim.'
} finally { $env:PATH=$originalPath }
