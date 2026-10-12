# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
function Confirm-ServiceObservation([string[]]$Lines,[hashtable]$Registers){
    $own=@($Lines | Where-Object {$_ -match '^own img=([0-9A-F]{16}) bun=([0-9A-F]{16}) stk=([0-9A-F]{16})$'})
    if($own.Count -ne 1){throw 'Owned service layout required.'}
    $null=$own[0] -match '^own img=([0-9A-F]{16}) bun=([0-9A-F]{16}) stk=([0-9A-F]{16})$'
    $bundle=[Convert]::ToUInt64($Matches[2],16)
    foreach($key in @('rcx','r9','r10','r11','r14')){if(-not $Registers.ContainsKey($key)){throw 'Missing service register.'}}
    if($Registers.rcx -ne $bundle+503808+2076){throw 'Own service result pointer rejected.'}
    if($Registers.r9 -ne 10 -or $Registers.r10 -ne 0 -or $Registers.r11 -ne 0x53564331 -or $Registers.r14 -ne 0x43535331){throw 'Shared service probe rejected.'}
}
