# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$VBoxManage,[Parameter(Mandatory=$true)][string]$TrialRoot,
    [Parameter(Mandatory=$true)][ValidateSet('none','ud','gp')][string]$Profile,
    [Parameter(Mandatory=$true)][ValidatePattern('^cold-[0-9]{2}$')][string]$Trial,[switch]$OwnerApproved)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
if(-not $OwnerApproved){throw 'Require owner approval for SPEC-0019 launch.'}
. (Join-Path $PSScriptRoot 'FaultObservation.ps1')
. (Join-Path $PSScriptRoot 'ServiceObservation.ps1')
$p=Get-Content -LiteralPath (Join-Path $TrialRoot 'prepared.json') -Raw | ConvertFrom-Json
$admission=Get-Content -LiteralPath (Join-Path $TrialRoot 'fault-admission.json') -Raw | ConvertFrom-Json
if($p.candidate -notin @('SPEC-0019','SPEC-0020') -or $admission.uuid -ne $p.uuid -or
    ($p.candidate -eq 'SPEC-0020' -and $Profile -ne 'none')){throw 'Owned fault/service VM admission rejected.'}
$medium=$admission.profiles.$Profile
$serial=Join-Path $TrialRoot ($Trial+'-serial.txt');$record=Join-Path $TrialRoot ($Trial+'-observations.json')
$commandLog=Join-Path $TrialRoot ($Trial+'-commands.json')
if((Test-Path -LiteralPath $serial) -or (Test-Path -LiteralPath $record) -or (Test-Path -LiteralPath $commandLog)){throw 'Fresh observation paths required.'}
if((Get-FileHash -LiteralPath $medium.vdi -Algorithm SHA256).Hash.ToLowerInvariant() -ne $medium.vdi_sha256){throw 'Owned medium changed before trial.'}
$script:commands=@()
function Run([string[]]$Arguments){
    $saved=$ErrorActionPreference
    try{$ErrorActionPreference='Continue';$output=@(& $VBoxManage @Arguments 2>&1);$code=$LASTEXITCODE}
    finally{$ErrorActionPreference=$saved}
    $script:commands += [pscustomobject]@{utc=[DateTime]::UtcNow.ToString('o');argv=$Arguments;exit=$code;output=($output | ForEach-Object {$_.ToString()}) -join "`n"}
    $script:commands | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $commandLog -Encoding UTF8
    if($code -ne 0){throw "Own VM command failed: $($Arguments[0])"}
    $output | ForEach-Object {$_.ToString()}
}
$info=@(Run @('showvminfo',$p.uuid,'--machinereadable'))
if($info -notcontains 'VMState="poweroff"'){throw 'Require cold powered-off start.'}
$null=Run @('storageattach',$p.uuid,'--storagectl','ClasSICK-SATA','--port','0','--device','0','--type','hdd','--medium',$medium.vdi)
$null=Run @('modifyvm',$p.uuid,'--uart-mode1','file',$serial)
& (Join-Path $PSScriptRoot 'Review-FaultVM.ps1') -VBoxManage $VBoxManage -TrialRoot $TrialRoot -Disk $medium.vdi -Serial $serial
$clock=[Diagnostics.Stopwatch]::StartNew();$events=@();$snapshots=@();$seen=0;$started=$false
$observation=[ordered]@{contract=$p.candidate;trial=$Trial;profile=$Profile;source_revision=$admission.source_revision;
    status='incomplete';generated_input=$false;firmware_memory_read=$false;events=@();registers=@()}
try{
    $null=Run @('startvm',$p.uuid,'--type','headless');$started=$true
    $lines=@()
    while($clock.Elapsed.TotalSeconds -lt 120){
        if(Test-Path -LiteralPath $serial){
            $stream=[IO.File]::Open($serial,[IO.FileMode]::Open,[IO.FileAccess]::Read,[IO.FileShare]::ReadWrite)
            try{$reader=New-Object IO.StreamReader($stream,[Text.Encoding]::ASCII);$s=$reader.ReadToEnd()}finally{$stream.Dispose()}
            $banner=$s.IndexOf("ClasSICK 0.97 UART v2`r`n")
            if($banner -ge 0){$parts=@($s.Substring($banner) -split "`r`n");$lines=@($parts | Select-Object -First ($parts.Count-1))}
        }
        while($seen -lt $lines.Count){
            $events += [pscustomobject]@{line=$lines[$seen];host_seconds=$clock.Elapsed.TotalSeconds;utc=[DateTime]::UtcNow.ToString('o')}
            if($lines[$seen] -match '^keyboard ready$|^result='){Write-Output $lines[$seen]};++$seen
        }
        if(@($lines | Where-Object {$_ -eq 'result=00000000 kbd=00000001'}).Count -eq 1){break}
        $observation.events=$events;$observation | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $record -Encoding UTF8
        Start-Sleep -Milliseconds 100
    }
    # Require successful own completion before any selected CPU register query.
    if(@($lines | Where-Object {$_ -eq 'keyboard ready'}).Count -ne 1 -or
        @($lines | Where-Object {$_ -eq 'result=00000000 kbd=00000001'}).Count -ne 1 -or
        @($lines | Where-Object {$_ -eq 'qual port=00000003/00000003 a20=00000000/00000000'}).Count -ne 1){throw 'No successful own run; do not query registers.'}
    $symbols=@{};foreach($item in $medium.symbols.PSObject.Properties){$symbols[$item.Name]=[long]$item.Value}
    $registerNames=if($Profile -eq 'none'){@('rip','rsp')}else{@('rip','rsp','rcx','r9','r10','r11','r12','r13','r14','r15','rsi','rdi')}
    if($p.candidate -eq 'SPEC-0020'){$registerNames+=@('rcx','r9','r10','r11','r14')}
    foreach($sample in @(1,2)){
        Start-Sleep -Milliseconds 1000
        $out=@(Run (@('debugvm',$p.uuid,'getregisters')+$registerNames));$registers=@{}
        foreach($line in $out){
            if($line -notmatch '^([a-z0-9]+) = 0x([0-9A-Fa-f]{1,16})$' -or $registers.ContainsKey($Matches[1])){throw 'Selected-register response rejected.'}
            $registers[$Matches[1]]=[Convert]::ToUInt64($Matches[2],16)
        }
        $snapshots += [pscustomobject]@{host_seconds=$clock.Elapsed.TotalSeconds;values=$registers}
        $observation.registers=$snapshots;$observation.events=$events
        $observation | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $record -Encoding UTF8
        Confirm-FaultObservation $Profile $lines $registers $symbols
        if($p.candidate -eq 'SPEC-0020'){Confirm-ServiceObservation $lines $registers}
    }
    foreach($key in $registerNames){if($snapshots[0].values[$key] -ne $snapshots[1].values[$key]){throw 'Terminal register observation unstable.'}}
    $observation.status='passed';Write-Output "Native $Profile fault/control observation PASS."
}catch{$observation.status='failed';$observation.failure=$_.Exception.Message;throw}
finally{
    $observation.events=$events;$observation.registers=$snapshots
    $observation | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $record -Encoding UTF8
    if($started){$null=Run @('controlvm',$p.uuid,'poweroff')}
}
if((Get-FileHash -LiteralPath $medium.vdi -Algorithm SHA256).Hash.ToLowerInvariant() -ne $medium.vdi_sha256){throw 'Owned disk changed during trial.'}
