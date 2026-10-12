# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'ServiceObservation.ps1')
$lines=@('own img=0000000010000000 bun=0000000020000000 stk=0000000020051000')
$registers=@{rcx=0x2007B81C;r9=10;r10=0;r11=0x53564331;r14=0x43535331}
Confirm-ServiceObservation $lines $registers
foreach($case in @(@('rcx',0x2007B818),@('r9',9),@('r10',1),@('r11',0),@('r14',0))){
    $bad=$registers.Clone();$bad[$case[0]]=$case[1];$rejected=$false
    try{Confirm-ServiceObservation $lines $bad}catch{$rejected=$true}
    if(-not $rejected){throw 'Bad service observation accepted.'}
}
Write-Output 'Service observer controls PASS: one valid result and five corrupted-result rejections.'
