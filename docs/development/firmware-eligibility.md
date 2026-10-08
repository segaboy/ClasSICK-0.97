# Native-PC firmware eligibility follow-up

Date 2026-10-08. SRC-0035/SRC-0040; no dependency adoption or VM launch.

The exact official VirtualBox 7.2.16 x64 producer is now identified from publisher
build metadata: OvmfPkgX64.dsc/fdf with VBOX=1 produces OVMF.fd, renamed to
VBoxEFI-amd64.fd and embedded by the device packaging manifest. This supersedes
the earlier unknown-producer/ARM-manifest lead, not the pending eligibility gate.
Exact tag, file blobs and scope are recorded in [the source catalog](../../provenance/sources.md).

The selected FDF region includes VBoxHfs, VBoxAppleSim and VBoxApfsJmpStartDxe.
Their build/license declarations identify Oracle copyrights and alternative GPL/
CDDL terms. Module names and notices alone neither prove Apple code inclusion nor
establish clean derivation. The manual's Bhyve Apple-attribution notice is not
automatically in this x64 selection. Do not generalize an inventory to binary contents.

Remaining review must bind the installed image to the exact source/build, inventory
all selected transitive modules/libraries and notices, and establish acceptable
derivation/rights for that closure. Metadata review cannot supply missing historical
origins or authorize reading uncertain implementation. Seek provenance documentation
or an independently reviewed alternative before adoption. Do not extract/run the
image or inspect module code while it remains uncertain. No protected boot disk,
APFS driver, Apple OS/ROM or whole external source archive is fetched.

## Decision — 2026-10-08 (SRC-0059)

The documents-only review that followed closes what documents can close. Oracle
publishes the v7.2.16 source and build metadata (SRC-0035/0040), but no permitted
document binds the installed image to that source; only a rebuild could, and
fetching or studying the source archive stays excluded. That gap is not closable
here. The binding [clean-room policy](../clean-room/POLICY.md) restricts what
enters ClasSICK (implementation inputs), not the machine it runs on: running an
owned image and recording only its own outputs takes nothing from the firmware.
A physical PC's vendor firmware is the same situation.

The owner therefore cleared VirtualBox 7.2.16 x64 EFI as a **black-box test
platform only**, under these conditions:

- No extraction, dumping, disassembly or decompilation of the firmware image, and
  no reading of its module implementation bodies, for any purpose.
- Diagnose from the UEFI specification, our own code and our own outputs (screen,
  serial capture, our retained trace). Firmware misbehavior is recorded as an
  observed result, not investigated inside the firmware.
- Present only SPEC-0014 media; configure no macOS-guest option and attach no
  HFS/APFS or Apple-derived media. Preserve the owner's existing VMs.
- Nothing from the firmware is committed, redistributed or used as a test oracle
  for historical behavior. VirtualBox remains an owner-installed external tool.
- Results are VM evidence for that exact configuration only; they do not certify
  the firmware or substitute for physical or Macintosh/mini vMac gates.

This does not authorize a VM trial; the first launch needs the owner's separate
explicit go-ahead. Remaining before it: an isolated VM configuration recorded
against the [profile](native-pc-profile.md) and a reviewed procedure for
converting/attaching the raw SPEC-0014 image.

Our original EFI loader and hosted tests proceed independently. An unloaded image
audit is not a VirtualBox boot. Firmware eligibility, isolated VM configuration,
formatted media, native exception/device contracts and B2 remain open; selecting
a different startup path would require an explicit revised profile and evidence.
