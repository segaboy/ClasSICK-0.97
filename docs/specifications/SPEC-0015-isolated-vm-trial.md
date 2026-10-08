# SPEC-0015: Isolated VirtualBox native-PC trial

Version 1, finalized 2026-10-08. SRC-0060 owner go-ahead; SRC-0035 Oracle's
7.2.16 user manual and SRC-0061 installed public CLI usage. ADR-0019 /
IMPL-0019 / TEST-0022. This contract covers configuration, attachment and
observation of owned media. It does not change native code or certify firmware.

## Scope and admission

The owner approved preparing and running the first isolated VirtualBox trial.
SRC-0059 remains the black-box firmware boundary: no extraction, dumping,
disassembly, implementation study, macOS guest options or HFS/APFS/Apple media.
Observe only our outputs and public configuration. Do not inspect NVRAM contents,
firmware memory/code or whole guest dumps. No existing owner VM is modified.

Use installed VirtualBox 7.2.16r174877 on the declared Windows host. Read the
existing VM identity inventory, retain it privately, and require a fresh project
name, UUID and workspace. All mutations target the newly created UUID. Do not
reuse an unrelated VM or automatically unregister/delete anything after failure.

Require the pinned Windows Verify-BootMedia.ps1 chain to pass in a fresh root.
Check raw size 67,108,864 and SHA-256
`370b7d4e7fc4200b77c367e09afa0c4944f534c2623d7f5f68db159c1da46078`, and payload
size 37,888 and SHA-256
`8e25d6947677b538d84bc17f98ff44d056dcd3d7769d1389676072286ad831f2`.
Re-read packaging with Test-BootMedia.ps1. No other disk is attached. The fixed
SPEC-0014 GPT GUIDs mean two copies must never be attached simultaneously.

## Declared machine

| Setting | Value |
| --- | --- |
| Guest type/architecture | Other_64, x86; no macOS option |
| Firmware | EFI64; explicitly disable Secure Boot through documented modifynvram configuration; no key enrollment or NVRAM extraction |
| CPU/RAM | One CPU, 512 MiB; hardware virtualization, no nested virtualization or paravirtualized guest services |
| Board | PIIX3, ACPI and I/O APIC on; guest driver uses polling and disables interrupts after handoff |
| Display | VBoxVGA, 16 MiB, one monitor, 3D off |
| Input | PS/2 keyboard and mouse; no USB controller |
| Serial | UART1 at 0x3F8/IRQ4, 16550A public model, file capture; other UARTs off. Guest sets 9600/8N1; native model/clock behavior is a qualification result |
| Storage | One IntelAhci SATA controller/port, only converted SPEC-0014 VDI; no optical/floppy/other disk, disk-only boot order |
| Integration | All eight NICs off, audio in/out off, USB OHCI/EHCI/XHCI off, clipboard/file transfers/drag-and-drop off, VRDE off; no Guest Additions, shared folders, Extension Pack feature or host device passthrough |

Record the actual public showvminfo output and verify each required setting
before start. Configuration acceptance is not a hardware conformance claim.
Any configuration/attachment error stops admission and is retained, except the
documented trial-specific case in which disable refuses because no platform key
is enrolled and public showvminfo explicitly reports SecureBoot="off". Retain the
nonzero command as a refusal, require that off readback, and enroll no key.

## Original attach procedure

Create a fresh project workspace and machine identity. Convert the verified raw
image with `VBoxManage convertfromraw <raw> <new.vdi> --format VDI --variant Fixed
--uuid <new-medium-uuid>`. Record executable version/hash, exact argv, exit codes,
raw and resulting VDI hashes. Convert the finished VDI back with documented
`clonemedium disk <vdi> <new-roundtrip.img> --format RAW`; require byte-identical
raw SHA-256 before attachment. The container UUID/hash is local run metadata;
raw reproducibility does not imply reproducible VDI metadata.

Create/register only the new machine; configure it with explicit modifyvm
settings; initialize its fresh EFI variable store and disable Secure Boot through
documented modifynvram commands. Add the single SATA controller and attach the
new VDI at port/device 0/0. No physical disk or host USB writing is authorized.
Retain command/configuration results. Never interpret a nonzero CLI exit as pass.

## Observation and acceptance

First start is headless for bounded screen/serial qualification. Retain host UTC
times and monotonic elapsed measurements, capture the guest screen through
documented screenshotpng, and capture UART bytes to a fresh file per cold start.
Do not save VM state or treat a reset/resume as a repeated cold start. Power off
only the project UUID after retaining the result; subsequent attempts begin from
powered-off state, with separate capture paths.

An original scene establishes visible output only. Our serial messages and trace
states must support exit/owned-entry/readiness/device claims; screen pixels alone
do not prove ExitBootServices or post-handoff ownership. Compare the 60-second
guest progress/termination with a host independent monotonic clock. Retain
initial, intermediate and final observations. VM pauses/scheduling remain limits.

Injected `keyboardputscancode` Space/Escape sequences are synthetic integration
evidence and must be labeled as such. They cannot replace real operator keyboard
evidence. If the native path works, arrange a visible owner-operated session for
two Space presses/releases and appropriate termination, keeping a separate
uninterrupted >=60-second run. Escape is early termination, not duration acceptance.
Repeat cold starts and require consistent observable pass sequences before B2.

Fail closed on unsupported profile/data or native faults. Retain empty serial,
screen failures, early halts, restarts and timing/input failures. Diagnose only
from reviewed interfaces, our code and our own output. Do not retrieve firmware
internals to explain a failure. A trial may complete as failed/partial; it does
not justify a native boot claim. A later code correction requires its own review,
fresh verification and a new identified trial, with previous failures preserved.

Public evidence contains sanitized configuration, exact source/tool/media hashes,
own serial text and safe original-scene captures only. Keep owner VM names/IDs,
machine-specific paths, firmware pixels/binaries and personal host data private.
This VM milestone does not close OS-edition or physical Macintosh/mini vMac gates.
