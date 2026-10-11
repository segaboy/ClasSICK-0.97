# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
# TEST-0025: observe one admitted SPEC-0016 cold start through public interfaces.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$VBoxManage,
      [Parameter(Mandatory=$true)][string]$TrialRoot,
      [Parameter(Mandatory=$true)][ValidatePattern('^cold-[0-9]{2}$')][string]$Trial,
      [ValidateSet('headless','gui')][string]$Mode='headless',
      [switch]$OwnerApproved)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
if(-not $OwnerApproved){throw 'A SPEC-0016 trial requires explicit owner approval.'}
$VBoxManage=[IO.Path]::GetFullPath($VBoxManage)
$TrialRoot=[IO.Path]::GetFullPath($TrialRoot)
$prepared=Get-Content -LiteralPath (Join-Path $TrialRoot 'prepared.json') -Raw | ConvertFrom-Json
$review=Get-Content -LiteralPath (Join-Path $TrialRoot 'configuration-review.json') -Raw | ConvertFrom-Json
if($prepared.candidate -ne 'SPEC-0016' -or $review.status -ne 'passed-before-launch' -or
        $prepared.uuid -ne $review.uuid -or $prepared.name -notmatch '^ClasSICK-097-B2-'){
    throw 'Require reviewed SPEC-0016 project admission.'
}
$ownedVmUuid=[string]$prepared.uuid
$serial=Join-Path $TrialRoot ($Trial+'-serial.txt')
$record=Join-Path $TrialRoot ($Trial+'-observations.json')
$commandLog=Join-Path $TrialRoot ($Trial+'-commands.json')
if((Test-Path -LiteralPath $serial) -or (Test-Path -LiteralPath $record) -or
        (Test-Path -LiteralPath $commandLog)){throw 'Fresh cold-start paths required.'}
$script:commands=@()
function Invoke-OwnedVBox([string[]]$Arguments,[switch]$ObservationMayFail){
    $saved=$ErrorActionPreference
    try {$ErrorActionPreference='Continue'; $output=@(& $VBoxManage @Arguments 2>&1); $code=$LASTEXITCODE}
    finally {$ErrorActionPreference=$saved}
    $script:commands += [pscustomobject]@{utc=[DateTime]::UtcNow.ToString('o');argv=$Arguments;exit=$code;output=($output | ForEach-Object {$_.ToString()}) -join "`n"}
    $script:commands | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $commandLog -Encoding UTF8
    if($code -ne 0 -and -not $ObservationMayFail){throw "Public management command failed ($code); retain $commandLog."}
    if($ObservationMayFail){return [pscustomobject]@{exit=$code;output=$output}}
    $output | ForEach-Object {$_.ToString()}
}
$info=@(Invoke-OwnedVBox @('showvminfo',$ownedVmUuid,'--machinereadable'))
if($info -notcontains 'VMState="poweroff"' -or $info -notcontains 'SecureBoot="off"'){
    throw 'Require powered-off project VM and public SecureBoot=off.'
}
$running=@(Invoke-OwnedVBox @('list','runningvms'))
if(@($running | Where-Object {$_ -match '^"ClasSICK-097-B2-'}).Count -ne 0){throw 'Another project VM is running.'}
$null=Invoke-OwnedVBox @('modifyvm',$ownedVmUuid,'--uart-mode1','file',$serial)
$hostClock=[Diagnostics.Stopwatch]::StartNew()
$startedUtc=[DateTime]::UtcNow.ToString('o')
$null=Invoke-OwnedVBox @('startvm',$ownedVmUuid,'--type',$Mode)
$lineEvents=@(); $captures=@(); $seen=0; $previousPoll=0.0
$ready=$null; $ending=$null; $firstFrame=$false; $registersQueried=$false; $unknown=@()
$patterns=@('^ClasSICK 0\.97 UART v2$',
    '^fw rev=[0-9A-F]{8} state=[0-9A-F]{8} vendor=[\x20-\x7E]{0,31}$',
    '^own img=[0-9A-F]{16} bun=[0-9A-F]{16} stk=[0-9A-F]{16}$',
    '^keyboard ready$', '^qual port=[0-9A-F]{8}/[0-9A-F]{8} a20=[0-9A-F]{8}/[0-9A-F]{8}$',
    '^alias q=[0-9A-F]{16} type=[0-9A-F]{8}$',
    '^frame=[0-9A-F]{8} sec=[0-9A-F]{8} space=[0-9A-F]{8}$',
    '^result=[0-9A-F]{8} kbd=[0-9A-F]{8}$')
while($hostClock.Elapsed.TotalSeconds -lt 120){
    $lines=@()
    if(Test-Path -LiteralPath $serial){
        $stream=[IO.File]::Open($serial,[IO.FileMode]::Open,[IO.FileAccess]::Read,[IO.FileShare]::ReadWrite)
        try {$reader=New-Object IO.StreamReader($stream,[Text.Encoding]::ASCII); $serialText=$reader.ReadToEnd()}
        finally {$stream.Dispose()}
        $banner=$serialText.IndexOf("ClasSICK 0.97 UART v2`r`n")
        if($banner -ge 0){
            $parts=@($serialText.Substring($banner) -split "`r`n")
            for($i=0;$i -lt $parts.Count-1;++$i){$lines += $parts[$i]}
        }
    }
    $sample=$hostClock.Elapsed.TotalSeconds
    while($seen -lt $lines.Count){
        $line=$lines[$seen]; $matched=$false
        foreach($pattern in $patterns){if($line -match $pattern){$matched=$true; break}}
        $event=[pscustomobject]@{line=$line;recognized=$matched;previous_poll_seconds=$previousPoll;observed_host_seconds=$sample;utc=[DateTime]::UtcNow.ToString('o')}
        $lineEvents += $event
        if(-not $matched){$unknown += $line}
        if($line -eq 'keyboard ready'){$ready=$event; Write-Output ('Keyboard READY at {0:F3} host seconds.' -f $sample)}
        if($line -match '^qual '){Write-Output $line}
        if($line -match '^frame='){$firstFrame=$true}
        if($line -match '^result='){$ending=$event; Write-Output $line}
        ++$seen
    }
    if($null -ne $ready -and -not $registersQueried){
        # Read only RIP/RSP after our native READY output; no memory/code query.
        $registers=Invoke-OwnedVBox @('debugvm',$ownedVmUuid,'getregisters','rip','rsp') -ObservationMayFail
        [pscustomobject]@{utc=[DateTime]::UtcNow.ToString('o');exit=$registers.exit;output=($registers.output | ForEach-Object {$_.ToString()}) -join "`n"} |
            ConvertTo-Json | Set-Content -LiteralPath (Join-Path $TrialRoot ($Trial+'-own-registers.json')) -Encoding UTF8
        $registersQueried=$true
    }
    $captureLabel=$null
    if($firstFrame -and @($captures | Where-Object {$_.label -eq 'initial'}).Count -eq 0){$captureLabel='initial'}
    elseif($null -ne $ready -and $sample-$ready.observed_host_seconds -ge 30 -and
            $null -eq $ending -and @($captures | Where-Object {$_.label -eq 'intermediate'}).Count -eq 0){$captureLabel='intermediate'}
    if($null -ne $ending){$captureLabel='final'}
    if($null -ne $captureLabel){
        $captureName=$Trial+'-'+$captureLabel+'.png'
        $capture=Invoke-OwnedVBox @('controlvm',$ownedVmUuid,'screenshotpng',(Join-Path $TrialRoot $captureName)) -ObservationMayFail
        $captureEvent=[ordered]@{label=$captureLabel;name=$captureName;exit=$capture.exit;elapsed_host_seconds=$hostClock.Elapsed.TotalSeconds;utc=[DateTime]::UtcNow.ToString('o')}
        if($capture.exit -eq 0){$captureEvent.sha256=(Get-FileHash -LiteralPath (Join-Path $TrialRoot $captureName) -Algorithm SHA256).Hash.ToLowerInvariant()}
        $captures += [pscustomobject]$captureEvent
    }
    [ordered]@{trial=$Trial;uuid=$ownedVmUuid;candidate='SPEC-0016';mode=$Mode;started_utc=$startedUtc;
        poll_interval_ms=100;generated_keyboard_input=$false;events=$lineEvents;captures=$captures;unrecognized_lines=$unknown} |
        ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $record -Encoding UTF8
    if($null -ne $ending){break}
    $previousPoll=$sample
    Start-Sleep -Milliseconds 100
}
if($null -eq $ending){throw 'No own terminal serial line within 120 host seconds; preserve incomplete trial.'}
if($null -ne $ready){Write-Output ('Ready-to-terminal arrival difference: {0:F6} host seconds.' -f ($ending.observed_host_seconds-$ready.observed_host_seconds))}
Write-Output 'Observation complete; qualification classification and operator confirmation remain separate.'
