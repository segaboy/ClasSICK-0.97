# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
# SPEC-0015: prepare one fresh project VM; never starts it or modifies other VMs.
# -Candidate names the admitted payload: SPEC-0015's first image or SPEC-0016's
# qualification-diagnostics image. Both use the same machine configuration.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][ValidateSet('SPEC-0015','SPEC-0016')][string]$Candidate,
      [Parameter(Mandatory=$true)][string]$VBoxManage,
      [Parameter(Mandatory=$true)][string]$BuildRoot,
      [Parameter(Mandatory=$true)][string]$TrialRoot,
      [Parameter(Mandatory=$true)][string]$VMName,
      [switch]$OwnerApproved)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
if(-not $OwnerApproved){throw 'SPEC-0015 requires explicit owner approval.'}
if($VMName -notmatch '^ClasSICK-097-B2-[A-Za-z0-9-]+$'){throw 'Use a distinct project VM name.'}
$VBoxManage=[IO.Path]::GetFullPath($VBoxManage)
$BuildRoot=[IO.Path]::GetFullPath($BuildRoot)
$TrialRoot=[IO.Path]::GetFullPath($TrialRoot)
if(-not (Test-Path -LiteralPath $VBoxManage -PathType Leaf)){throw 'VBoxManage not found.'}
if(Test-Path -LiteralPath $TrialRoot){throw 'Use a fresh trial root.'}
$image=Join-Path $BuildRoot 'media/clang-hosted-debug-a.img'
$payload=Join-Path $BuildRoot 'efi/payload/EFI/BOOT/BOOTX64.EFI'
$result=Get-Content -LiteralPath (Join-Path $BuildRoot 'boot-media-results.json') -Raw | ConvertFrom-Json
if($result.loaded -or $result.firmware -or $result.corruption_rejections -ne 30 -or $result.tool_refusals -ne 3){throw 'Full packaging evidence rejected.'}
if(@($result.writer_builds.PSObject.Properties).Count -ne 9){throw 'Require all nine Windows writer builds.'}
$imageHash=(Get-FileHash -LiteralPath $image -Algorithm SHA256).Hash.ToLowerInvariant()
$payloadHash=(Get-FileHash -LiteralPath $payload -Algorithm SHA256).Hash.ToLowerInvariant()
$admitted=@{
    'SPEC-0015'=@{payload_bytes=37888;payload='8e25d6947677b538d84bc17f98ff44d056dcd3d7769d1389676072286ad831f2';image='370b7d4e7fc4200b77c367e09afa0c4944f534c2623d7f5f68db159c1da46078'};
    'SPEC-0016'=@{payload_bytes=43520;payload='920ed92c2b30609718d1ed95867e0cf79aab82ae5f9ce76ffee2c9b806ce5d16';image='e2e6a431722b39d6ed7c215bbc60727aef74ae776f74934f60ddfd0e6f4069e9'}}[$Candidate]
if((Get-Item -LiteralPath $image).Length -ne 67108864 -or $imageHash -ne $admitted.image){throw 'Raw image rejected.'}
if((Get-Item -LiteralPath $payload).Length -ne $admitted.payload_bytes -or $payloadHash -ne $admitted.payload){throw 'Payload rejected.'}
& (Join-Path $PSScriptRoot 'Test-BootMedia.ps1') -Image $image -Payload $payload | Out-Null
[void][IO.Directory]::CreateDirectory($TrialRoot)
$script:stepNumber=0
function Run-VBox([string[]]$Arguments,[switch]$AllowMissingPlatformKey) {
    ++$script:stepNumber
    $previousErrorAction=$ErrorActionPreference
    try {
        $ErrorActionPreference='Continue'
        $output=@(& $VBoxManage @Arguments 2>&1)
        $code=$LASTEXITCODE
    } finally { $ErrorActionPreference=$previousErrorAction }
    $name=('{0:D2}-{1}.txt' -f $script:stepNumber,$Arguments[0])
    (($output | ForEach-Object { $_.ToString() }) -join "`r`n") | Set-Content -LiteralPath (Join-Path $TrialRoot $name) -Encoding UTF8
    [ordered]@{step=$script:stepNumber;utc=[DateTime]::UtcNow.ToString('o');argv=$Arguments;exit=$code;output=$name} |
        ConvertTo-Json -Compress | Add-Content -LiteralPath (Join-Path $TrialRoot 'commands.jsonl') -Encoding UTF8
    if($code -ne 0){
        $message=($output | ForEach-Object { $_.ToString() }) -join "`n"
        if(-not $AllowMissingPlatformKey -or $message -notmatch 'Secure boot is not available because the platform key \(PK\) is not enrolled'){
            throw "VBoxManage $($Arguments[0]) failed ($code); retain $name."
        }
        Write-Warning 'Disable command refused without an enrolled PK; require public SecureBoot=off readback.'
    }
    $output | ForEach-Object { $_.ToString() }
}
$version=@(Run-VBox @('--version')) -join "`n"
if($version.Trim() -ne '7.2.16r174877'){throw 'Installed version differs from declared trial.'}
$before=@(Run-VBox @('list','vms'))
if(@($before | Where-Object { $_.StartsWith('"'+$VMName+'" ') }).Count -ne 0){throw 'Project name already registered.'}
$uuid=[Guid]::NewGuid().ToString()
$mediumUuid=[Guid]::NewGuid().ToString()
$vdi=Join-Path $TrialRoot 'classick-097.vdi'
$roundtrip=Join-Path $TrialRoot 'roundtrip.img'
$null=Run-VBox @('convertfromraw',$image,$vdi,'--format','VDI','--variant','Fixed','--uuid',$mediumUuid)
$null=Run-VBox @('clonemedium','disk',$vdi,$roundtrip,'--format','RAW')
if((Get-FileHash -LiteralPath $roundtrip -Algorithm SHA256).Hash.ToLowerInvariant() -ne $imageHash){throw 'VDI round-trip raw mismatch.'}
$null=Run-VBox @('createvm','--name',$VMName,'--platform-architecture','x86','--ostype','Other_64','--basefolder',$TrialRoot,'--uuid',$uuid,'--register')
$serial=Join-Path $TrialRoot 'cold-01-serial.txt'
$settings=@('modifyvm',$uuid,'--firmware','efi64','--cpus','1','--memory','512','--chipset','piix3',
    '--acpi','on','--ioapic','on','--graphicscontroller','vboxvga','--vram','16','--accelerate-3d','off',
    '--monitor-count','1','--mouse','ps2','--keyboard','ps2','--hwvirtex','on','--nested-hw-virt','off',
    '--paravirt-provider','none','--boot1','disk','--boot2','none','--boot3','none','--boot4','none',
    '--audio-enabled','off','--audio-in','off','--audio-out','off','--usb-ohci','off','--usb-ehci','off','--usb-xhci','off',
    '--clipboard-mode','disabled','--clipboard-file-transfers','disabled','--drag-and-drop','disabled','--vrde','off',
    '--uart1','0x3F8','4','--uart-type1','16550A','--uart-mode1','file',$serial,
    '--uart2','off','--uart3','off','--uart4','off')
for($n=1;$n -le 8;++$n){$settings+=@(('--nic'+$n),'none')}
$null=Run-VBox $settings
$null=Run-VBox @('modifynvram',$uuid,'inituefivarstore')
$null=Run-VBox @('modifynvram',$uuid,'secureboot','--disable') -AllowMissingPlatformKey
$secureBootInfo=@(Run-VBox @('showvminfo',$uuid,'--machinereadable'))
if($secureBootInfo -notcontains 'SecureBoot="off"'){throw 'Require explicit public SecureBoot=off state.'}
$null=Run-VBox @('storagectl',$uuid,'--name','ClasSICK-SATA','--add','sata','--controller','IntelAhci','--portcount','1','--hostiocache','off','--bootable','on')
$null=Run-VBox @('storageattach',$uuid,'--storagectl','ClasSICK-SATA','--port','0','--device','0','--type','hdd','--medium',$vdi)
$info=@(Run-VBox @('showvminfo',$uuid,'--machinereadable'))
$after=@(Run-VBox @('list','vms'))
$expected=@($before)+@('"'+$VMName+'" {'+$uuid+'}')
if(@(Compare-Object ($expected | Sort-Object) ($after | Sort-Object)).Count -ne 0){throw 'VM inventory changed outside the one new identity.'}
[ordered]@{contract='SPEC-0015';candidate=$Candidate;prepared_utc=[DateTime]::UtcNow.ToString('o');name=$VMName;uuid=$uuid;
    medium_uuid=$mediumUuid;raw_image=$image;raw_sha256=$imageHash;payload_sha256=$payloadHash;
    vdi=$vdi;vdi_sha256=(Get-FileHash -LiteralPath $vdi -Algorithm SHA256).Hash.ToLowerInvariant();
    roundtrip_sha256=(Get-FileHash -LiteralPath $roundtrip -Algorithm SHA256).Hash.ToLowerInvariant();
    vbox_version=$version.Trim();vbox_executable_sha256=(Get-FileHash -LiteralPath $VBoxManage -Algorithm SHA256).Hash.ToLowerInvariant();
    existing_vm_count=$before.Count;inventory_check='one-new-identity-only';serial=$serial;started=$false;
    configuration_review_required=$true;source_revision=(& git -C (Join-Path $PSScriptRoot '..') rev-parse HEAD)} |
    ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $TrialRoot 'prepared.json') -Encoding UTF8
Write-Output 'Project VM prepared; review retained configuration before start. No VM was started.'
