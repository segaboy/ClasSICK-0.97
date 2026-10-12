# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'FaultObservation.ps1')
# Independently chosen synthetic address layout and documented seven-qword frame.
$lines=@('ClasSICK 0.97 UART v2','own img=0000000010000000 bun=0000000020000000 stk=0000000020051000',
    'keyboard ready','qual port=00000003/00000003 a20=00000000/00000000',
    'frame=0000003D sec=0000003C space=00000000','result=00000000 kbd=00000001')
$symbols=@{cs_x64_terminal_halt=0x1234;cs_x64_probe_instruction=0x1200}
$regs=@{rip=0x10001236;rsp=0x20051fc8;rcx=0x20079000;r9=6;r10=0;r11=0x10001200;
    r12=0x20050fe0;r13=2;r14=0x43534631;r15=8;rsi=2;rdi=16}
Confirm-FaultObservation ud $lines $regs $symbols
$gp=$regs.Clone();$gp.r9=13;$gp.r10=40;Confirm-FaultObservation gp $lines $gp $symbols
$normal=@{rip=0x10001236;rsp=0x20050fe0};Confirm-FaultObservation none $lines $normal $symbols
$controls=0
foreach($case in @(@('r9',13),@('r10',40),@('r11',0x10001202),@('rsp',0x20051fd0),
    @('r12',0x20052000),@('rcx',0x20079008),@('r13',1),@('r14',0),@('r15',16),@('rdi',0),@('rsi',0x202),@('rip',0x10001240))){
    $bad=$regs.Clone();$bad[$case[0]]=$case[1];$rejected=$false
    try{Confirm-FaultObservation ud $lines $bad $symbols}catch{$rejected=$true}
    if(-not $rejected){throw 'Invalid captured state accepted.'};++$controls
}
foreach($bad in @(@($lines | Where-Object {$_ -ne 'keyboard ready'}),
    @($lines | Where-Object {$_ -notmatch '^result='}),
    @($lines | ForEach-Object {$_ -replace 'result=00000000','result=00000002'}),
    @($lines | ForEach-Object {$_ -replace 'a20=00000000/00000000','a20=00000001/00000000'}))){
    $rejected=$false;try{Confirm-FaultObservation ud $bad $regs $symbols}catch{$rejected=$true}
    if(-not $rejected){throw 'Incomplete/failed own run accepted.'};++$controls
}
Write-Output "Fault observer controls PASS: three profiles, $controls invalid-run rejections."
