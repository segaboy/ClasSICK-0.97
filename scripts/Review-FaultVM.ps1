# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$VBoxManage,
    [Parameter(Mandatory=$true)][string]$TrialRoot,[Parameter(Mandatory=$true)][string]$Disk,
    [Parameter(Mandatory=$true)][string]$Serial)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$p=Get-Content -LiteralPath (Join-Path $TrialRoot 'prepared.json') -Raw | ConvertFrom-Json
if($p.candidate -notin @('SPEC-0019','SPEC-0020') -or $p.name -notmatch '^ClasSICK-097-B2-' -or
    (Get-FileHash -LiteralPath $VBoxManage -Algorithm SHA256).Hash.ToLowerInvariant() -ne '7fda8e54157eb94a5ebd9cea079a9fb8c23ade92ae71e13399dc3431c021a228'){
    throw 'Fault VM admission rejected.'
}
$raw=@(& $VBoxManage showvminfo $p.uuid --machinereadable)
if($LASTEXITCODE -ne 0){throw 'Configuration read failed.'}
$info=@{};foreach($line in $raw){if($line -match '^([^=]+)=(.*)$'){$info[$Matches[1]]=$Matches[2]}}
$expected=@{firmware='"EFI64"';memory='512';cpus='1';chipset='"piix3"';acpi='"on"';ioapic='"on"';
    SecureBoot='"off"';VMState='"poweroff"';graphicscontroller='"vboxvga"';vram='16';accelerate3d='"off"';
    hidkeyboard='"ps2kbd"';hidpointing='"ps2mouse"';paravirtprovider='"none"';'nested-hw-virt'='"off"';
    boot1='"disk"';boot2='"none"';boot3='"none"';boot4='"none"';audio='"none"';audio_in='"off"';audio_out='"off"';
    usb='"off"';ehci='"off"';xhci='"off"';clipboard='"disabled"';clipboard_file_transfers='"off"';
    draganddrop='"disabled"';vrde='"off"';uart1='"0x03f8,4"';uarttype1='"16550A"';
    uart2='"off"';uart3='"off"';uart4='"off"';storagecontrollername0='"ClasSICK-SATA"';
    storagecontrollertype0='"IntelAhci"';storagecontrollerportcount0='"1"';storagecontrollerbootable0='"on"'}
for($n=1;$n -le 8;++$n){$expected['nic'+$n]='"none"'}
foreach($key in $expected.Keys){if(-not $info.ContainsKey($key) -or $info[$key] -ne $expected[$key]){throw "Isolation setting rejected: $key"}}
if($info.name -ne ('"'+$p.name+'"') -or $info.UUID -ne ('"'+$p.uuid+'"') -or
    $info['"ClasSICK-SATA-0-0"'] -ne ('"'+$Disk.Replace('\','\\')+'"') -or
    $info.uartmode1 -ne ('"file,'+$Serial+'"') -or $info.monitorcount -ne '1'){throw 'Owned disk/serial/VM binding rejected.'}
if(@($info.Keys | Where-Object {$_ -match '^storagecontrollername'}).Count -ne 1 -or
    @($info.Keys | Where-Object {$_ -match '^SharedFolder'}).Count -ne 0 -or
    @($info.Keys | Where-Object {$_ -match '^".*-[0-9]+-[0-9]+"$' -and $_ -notmatch '-(ImageUUID|nonrotational|discard)-' -and $info[$_] -ne '"none"'}).Count -ne 1){throw 'Extra share/controller/media rejected.'}
$running=@(& $VBoxManage list runningvms)
if($LASTEXITCODE -ne 0 -or @($running | Where-Object {$_ -match '^"ClasSICK-097-B2-'}).Count -ne 0){throw 'Another project VM is running.'}
$raw | Set-Content -LiteralPath (Join-Path $TrialRoot 'latest-configuration.txt') -Encoding UTF8
[ordered]@{status='passed-before-launch';uuid=$p.uuid;candidate=$p.candidate;checked_settings=$expected;
    disk=$Disk;serial=$Serial;utc=[DateTime]::UtcNow.ToString('o')} |
    ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $TrialRoot 'configuration-review.json') -Encoding UTF8
Write-Output 'Fresh fault VM configuration/isolation review PASS.'
