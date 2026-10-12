# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
# Pure host classifier. Inputs are own serial output, map RVAs and selected CPU registers.
function Confirm-FaultObservation {
    param([ValidateSet('none','ud','gp')][string]$Profile,[string[]]$Lines,
        [hashtable]$Registers,[hashtable]$Symbols)
    $owned=@($Lines | Where-Object {$_ -match '^own img=([0-9A-F]{16}) bun=([0-9A-F]{16}) stk=([0-9A-F]{16})$'})
    if($owned.Count -ne 1 -or @($Lines | Where-Object {$_ -eq 'ClasSICK 0.97 UART v2'}).Count -ne 1 -or
        @($Lines | Where-Object {$_ -eq 'keyboard ready'}).Count -ne 1 -or
        @($Lines | Where-Object {$_ -eq 'qual port=00000003/00000003 a20=00000000/00000000'}).Count -ne 1 -or
        @($Lines | Where-Object {$_ -eq 'result=00000000 kbd=00000001'}).Count -ne 1 -or
        @($Lines | Where-Object {$_ -match '^frame=[0-9A-F]{8} sec=0000003C space=[0-9A-F]{8}$'}).Count -ne 1){
        throw 'Own successful completion required.'
    }
    # Enforce own milestone order; an early halt/fault is never acceptance.
    $positions=@([Array]::IndexOf($Lines,'ClasSICK 0.97 UART v2'),[Array]::IndexOf($Lines,$owned[0]),
        [Array]::IndexOf($Lines,'keyboard ready'),[Array]::IndexOf($Lines,'qual port=00000003/00000003 a20=00000000/00000000'),
        [Array]::IndexOf($Lines,(@($Lines | Where-Object {$_ -match '^frame=[0-9A-F]{8} sec=0000003C '})[0])),
        [Array]::IndexOf($Lines,'result=00000000 kbd=00000001'))
    for($i=1;$i -lt $positions.Count;++$i){if($positions[$i] -le $positions[$i-1]){throw 'Own output order rejected.'}}
    $null=$owned[0] -match '^own img=([0-9A-F]{16}) bun=([0-9A-F]{16}) stk=([0-9A-F]{16})$'
    $base=[Convert]::ToUInt64($Matches[1],16);$bundle=[Convert]::ToUInt64($Matches[2],16);$top=[Convert]::ToUInt64($Matches[3],16)
    if($base -eq 0 -or $bundle -eq 0 -or $base -ge 0x800000000000L -or $bundle -ge 0x800000000000L -or
        $base%4096 -ne 0 -or $bundle%4096 -ne 0 -or $top -ne $bundle+331776){throw 'Owned layout rejected.'}
    foreach($key in @('rip','rsp')){if(-not $Registers.ContainsKey($key)){throw 'Missing register.'}}
    $halt=$base+$Symbols.cs_x64_terminal_halt
    if($Registers.rip -lt $halt -or $Registers.rip -gt $halt+3){throw 'Terminal RIP rejected.'}
    if($Profile -eq 'none'){
        if($Registers.rsp -lt $top-65536 -or $Registers.rsp -ge $top){throw 'Normal stack rejected.'}
        return
    }
    foreach($key in @('rcx','r9','r10','r11','r12','r13','r14','r15','rsi','rdi')){
        if(-not $Registers.ContainsKey($key)){throw 'Missing register.'}
    }
    if($Registers.r9 -ne @{ud=6;gp=13}[$Profile]){throw 'Vector rejected.'}
    if($Registers.r10 -ne @{ud=0;gp=40}[$Profile]){throw 'Error code rejected.'}
    if($Registers.r11 -ne $base+$Symbols.cs_x64_probe_instruction){throw 'Saved RIP rejected.'}
    if($Registers.rsp -ne $bundle+331776+4096-56){throw 'IST stack rejected.'}
    if($Registers.r12 -lt $top-65536 -or $Registers.r12 -ge $top){throw 'Interrupted stack rejected.'}
    if($Registers.rcx -ne $bundle+495616){throw 'State pointer rejected.'}
    if($Registers.r13 -ne 2 -or $Registers.r14 -ne 0x43534631){throw 'Record publication rejected.'}
    if($Registers.r15 -ne 8 -or $Registers.rdi -ne 16 -or ($Registers.rsi -band 0x200) -ne 0){throw 'Saved selectors/IF rejected.'}
}
