# System 1 and ROM boot compatibility requirements - 2026-10-14

Owner requirement, SRC-0164. Required, not yet demonstrated. This records target
behavior; it is not implementation evidence or a new boot/reference trial.

The owner states: "Original system 1 disks must boot on our custom rom, but also
the custom System 1 needs to be able to boot from an original Apple rom."

| Requirement | Firmware | Disk/system | Acceptance state |
| --- | --- | --- | --- |
| BOOT-COMPAT-1 | Independently authored ClasSICK replacement ROM | Owner-designated original System 1 release disks | Required; not tested. |
| BOOT-COMPAT-2 | Identified, supported original Macintosh Apple ROM | Independently authored ClasSICK System 1 replacement disk/system | Required; not tested. |
| Existing independent boot goal | ClasSICK replacement ROM | ClasSICK replacement system | Required; complete disk/system boot not established by the existing test scene. |

The two compatibility directions are separate gates. A passing replacement ROM
scene, hosted test, compiler build, application preflight or one matrix row does
not close another row or a complete-edition gate. They supplement the Macintosh
128K and original/native-PC/Linux edition goals; PC/Linux ports and the complete
independent distribution continue to work without an Apple-supplied ROM.

Before observations, identify the exact Macintosh model, original-ROM revision,
disk/System release and lawful owner-managed reference location. R-001 and the
January1984/0.97 identity question remain open; later manuals do not identify an
earlier release. "Original System 1 disks" means the owner-designated target set,
to be pinned by exact identity, not a claim about every disk or Macintosh ROM.
Record original-media identity through eligible metadata/owner records and any
separately approved safe identity protocol, not firmware extraction/disassembly.

Eligible original firmware and disks are future black-box reference platforms/
inputs, never implementation sources or distributed payloads. No Apple firmware,
System/Finder code, resources/assets, disk images or derived implementation is
imported, inspected, disassembled, copied into the project, sent to workers or
published. Published interface specifications and authorized external behavior
remain the implementation basis. The owner's requirement supersedes an older
replacement-ROM-only assumption for the second compatibility target; it does
not authorize reading Apple implementation or making an Apple ROM a dependency
of the independent edition.

Future trials still need source/platform admission and the owner's explicit
go-ahead for an emulator/VM/reference run. This instruction asks documentation
and continued development, not an actual launch or a protected-payload download.
Legitimate possession, private handling and observation/export boundaries must
be settled before using a reference. No such trial is started by this record.

Each future result records the declared row, exact model/ROM/media/configuration,
our artifact hashes and source revision, startup path, externally observable boot
completion and input/application smoke criteria, repeat count and retained faults.
Define that workload before measuring it. Keep own-bootloader/ROM services versus
disk-system responsibilities explicit. Boot-block/filesystem/ROM-interface and
application contracts remain implementation prerequisites; no Apple internals
are inferred from this requirement or from ClasSICK's engineering choices.

Use independently authored probes and sanitized textual observations. Protected
code/resources/images and reference screenshots are not public fixtures. A claimed
Macintosh physical/128K result needs its own measurements; emulator success alone
does not establish it. Native-application owner gate, physical/parity/three-edition,
B2criterion6 and human provenance review remain separate.
