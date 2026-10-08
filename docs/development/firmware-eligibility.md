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

Our original EFI loader and hosted tests proceed independently. An unloaded image
audit is not a VirtualBox boot. Firmware eligibility, isolated VM configuration,
formatted media, native exception/device contracts and B2 remain open; selecting
a different startup path would require an explicit revised profile and evidence.
