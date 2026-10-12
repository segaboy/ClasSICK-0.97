# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'ALineObservation.ps1')
$lines=@('own img=0000000010000000 bun=0000000020000000 stk=0000000020051000')
$registers=@{rcx=0x2007B828;r9=8;r10=0;r11=0x414C4E31;r12=6;r13=3;r14=0x43534131}
Confirm-ALineObservation $lines $registers
foreach($key in @('rcx','r9','r10','r11','r12','r13','r14')){
    $bad=$registers.Clone();$bad[$key]++;$rejected=$false
    try{Confirm-ALineObservation $lines $bad}catch{$rejected=$true}
    if(-not $rejected){throw 'Bad A-line observation accepted.'}
}
$bad=$registers.Clone();$bad.Remove('r12');$rejected=$false
try{Confirm-ALineObservation $lines $bad}catch{$rejected=$true}
if(-not $rejected){throw 'Incomplete A-line observation accepted.'}
Write-Output 'A-line observer PASS: one valid observation, seven corruptions and one missing-register rejection.'
