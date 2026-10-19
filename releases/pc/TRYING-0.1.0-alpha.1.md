# Trying ClasSICK PC Preview 0.1.0-alpha.1

This is an experimental preview. It is a small UEFI program that draws the same original test scene as ClasSICK ROM 0.3.0-alpha.1. It is an early OS preview without a desktop or System 1 boot. Reliable human pointer delivery has not passed in the VirtualBox window; see Known limitations.

The qualified host is Windows 11 with Oracle VirtualBox 7.2.16 and VBoxManage. Other hosts and versions are unqualified. No emulator or compiler is included.

## What is in the ZIP

- `EFI/BOOT/BOOTX64.EFI`: the program itself, 159,232 bytes.
- `classick-pc-0.1.0-alpha.1-x86_64.img`: a 64 MiB raw disk whose only job is to hold the program for booting. Most of it is empty, which is why the ZIP is much smaller. It is not the size of an operating system.
- README.txt, RELEASE.txt, COPYING, SOURCE.txt and SHA256SUMS.txt.

Check each file with `Get-FileHash -Algorithm SHA256` and compare the result with SHA256SUMS.txt. Do not write the raw disk to a real drive; physical PCs are not qualified.

## 1. Convert the raw disk to a new VDI

Leave the original .img unchanged and create a new fixed-size VDI from it:

    VBoxManage convertfromraw classick-pc-0.1.0-alpha.1-x86_64.img classick-pc-0.1.0-alpha.1.vdi --format VDI --variant Fixed

## 2. Create an isolated VM

    $vm = 'ClasSICK-PC-0.1.0-alpha.1'
    VBoxManage createvm --name $vm --ostype Other_64 --register
    VBoxManage modifyvm $vm --firmware efi64 --cpus 1 --memory 512 --graphicscontroller vboxvga --keyboard ps2 --mouse ps2
    VBoxManage modifyvm $vm --nic1 none --audio-enabled off --usb-ohci off --usb-ehci off --usb-xhci off
    VBoxManage modifyvm $vm --nic2 none --nic3 none --nic4 none --nic5 none --nic6 none --nic7 none --nic8 none
    VBoxManage modifyvm $vm --clipboard-mode disabled --drag-and-drop disabled --vrde off
    VBoxManage storagectl $vm --name SATA --add sata --controller IntelAhci
    VBoxManage storageattach $vm --storagectl SATA --port 0 --device 0 --type hdd --medium classick-pc-0.1.0-alpha.1.vdi

If VBoxManage cannot find the VDI, give its full path. Secure Boot must be off in this isolated VM; do not change host firmware security settings. Start from a fresh boot, never a saved state. Do not add shared folders, networking, USB devices or audio.

## 3. Run and test input

    VBoxManage startvm $vm --type gui

The program draws a black-and-white 512x342 scene centred on a 1024x768 screen: a ring with rails, six phases, the result 0097 and a counter.

1. If the guest reaches its scene, click inside the VM window. VirtualBox asks whether it may capture the mouse and keyboard; accept. To release them, press Right Ctrl (the host key).
2. After capture, release the button and write down the counter value. The capture click may itself count; measure from the new baseline. A mouse-integration warning is expected without Guest Additions and does not confirm capture.
3. For this experimental check, hold the left button for about one second, release for one second, then repeat. Press and release Space once. Record any miss; this timing suggestion is not a qualified capture fix.
4. The counter should now be 3 higher than the value you wrote down.

This preview runs continuously. It ignores the usual 60-second limit, and Escape does not quit it. To stop, close the VM window and choose Power off the machine.

## Known limitations

- Pointer capture and focus for a person using the GUI are unresolved. A tester reported intermittent clicks, and nobody has passed the +3 check by hand.
- One GUI launch failed before boot with VERR_CPUM_TOO_MANY_CPUID_SUBLEAVES. The cause is not known. Stop and report the error rather than changing host security or CPU settings; the headless observation is not a proven GUI workaround.
- What was checked: an unchanged VM was started headless and received 38 injected mouse events and 5 Space presses over 185 guest seconds. The static scene stayed unchanged. That tests the program, not a person's GUI session.
- Physical PCs are not qualified.

## More information

- Release notes: RELEASE.txt in this ZIP.
- Source: https://github.com/segaboy/ClasSICK-0.97/tree/c631b824b2a264c6d81eb9832f17b7b5918aa9ff
- Evidence: https://github.com/segaboy/ClasSICK-0.97/blob/87e95de40cfb0f06b6398a3cfa762e9c62b87375/docs/development/preview-session-evidence.json
- License: GPL-3.0-or-later (see COPYING).
