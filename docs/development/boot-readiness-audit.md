# Native-PC boot-readiness audit — 2026-10-08

**Decision: NO-GO for a native launch. Artifact integrity and unloaded EFI
inspection pass; formatted media, firmware eligibility and machine qualification
remain open. B2 is not passed.** SRC-0055 / TEST-0020.

This is an aggregate evidence/readiness audit of public revision
`653c442a9fa83e4bb2bfea727f43acb0a727f027`, whose compiled/test/script source is
`3b44cb9510a400397a33cd1cc0802f45c36c8110`. It changes documentation only.
[The machine-readable results](boot-readiness-audit.json) retain input hashes,
artifact bindings, EFI replays and limitations.

## Verified evidence

- All 117 recorded local inputs still match the full UART run. Of these, 116
  are byte-identical to the compiled-source Git blobs. The prior PS/2 fingerprint
  JSON has CRLF locally and LF in Git under `text`/`eol=lf`; normalizing only
  those line endings produces identical bytes. Both hashes are retained.
- All 59 comparison labels / 53 distinct hashes agree among the retained local
  log, pinned CI log and published UART manifest. Every hash is present in at
  least two retained owned executable/image artifacts. These are integrity
  checks, not fresh executions of those test programs.
- The retained log confirms 792 passing CTest checks across nine configurations:
  100/100/100/76/100 Clang and 85/85/85/61 scoped GCC. This audit does not repeat
  that matrix. [CI 37827697279](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37827697279)
  remains successful on exact compiled source `3b44cb9510a400397a33cd1cc0802f45c36c8110`.
- The existing owned EFI auditor was freshly run on both O0 twins, both O2 twins,
  and the payload copy. All five pass unloaded: 59 required original symbols,
  zero imports/runtime libraries, three DIR64 relocations and 256 vectors.
  Earlier corruption/omitted-object controls remain TEST-0019 evidence and were
  not rerun here.
- The payload contains only `EFI/BOOT/BOOTX64.EFI`, 37,888 bytes, O2 SHA-256
  `8e25d6947677b538d84bc17f98ff44d056dcd3d7769d1389676072286ad831f2`.
  Its directory layout and file identity pass; it is **not a formatted disk**.

Final evidence is retained in the owner-generated `boot-readiness-audit-20261008-c`
root. Two earlier metadata-collection attempts are retained: the first assumed
all worktree hashes equal Git blobs; the second expected an older CTest summary
spelling. Both stopped before EFI inspection. Correcting those collector
assumptions changed no project input. TEST-0020 records the distinction.

## Open gates

| Gate | Finding | Required next evidence |
| --- | --- | --- |
| Boot medium | Selected retained output is a directory tree. No formatter/partition recipe was found in tracked platform, scripts or specifications. This is not a workstation-wide disk search. | Reviewed independent FAT/UEFI packaging contract; original isolated media generation; independent structural/payload checks; exact image hash and reproducible recipe. |
| Firmware eligibility | VirtualBox 7.2.16 x64 producer/build selection is identified, but installed-binary correspondence and the full selected transitive origin/notice/rights closure remain `needs-review`. | Exact publisher provenance and source/build correspondence, or an independently reviewed alternative, before acquisition/adoption/execution. Build names and license notices do not prove clean derivation. |
| Machine qualification | The native-PC profile is proposed; no isolated VM configuration or device behavior has been observed. | Retained isolated configuration, firmware/startup/Secure Boot state, PS/2 self-test and A20 qualification, UART mapping/model/clock/9600-baud capture, ACPI timer and framebuffer qualification. Preserve existing owner VMs. |
| B2 observation | No owned image has been loaded or executed natively. | Successful owned-stack/ExitBootServices handoff, native exception/device behavior, visible 60-second progress checked against an independent clock, real post-handoff keyboard response and diagnostic trace, safe original-scene capture and repeated cold starts on the named eligible environment. |

The first two preparation tracks can proceed without a VM launch: independently
specify/package/audit our media, and resolve firmware eligibility using permitted
provenance documentation. Do not extract or run uncertain firmware or inspect
uncertain implementation bodies to fill the provenance gap. The proposed
[native-PC profile](native-pc-profile.md) and
[firmware finding](firmware-eligibility.md) remain authoritative prerequisites.

## Provenance and project boundaries

This audit checks the recorded UART process and aggregate artifacts. The owner
approved one fresh implementer under SRC-0053; that implementer independently
reviewed the scoped TI interface in SRC-0054 and authored/reviewed the UART
specification, code and fixtures. The exposed coordinating lead performs this
aggregate audit/publication only. No new UART algorithm, specification or fixture
review/change is claimed, and no additional delegation occurs. Human provenance
review remains pending; the repository guard is not an originality certificate.

B0 and hosted B1 remain passed. All three OS editions remain `not-started`, with
historical parity `not-tested`. The native-PC preparation milestone does not
establish Macintosh compatibility, Macintosh 128K feasibility, replacement-ROM
startup, or a native 68000 boot. Independent real Macintosh **and** mini vMac
validation remain required; emulator results cannot close the physical gate.

OpenAI Codex (GPT-6) assisted with original metadata collection and documentation.
Only owned artifacts and existing scoped records were used. No new external
implementation, protected reference payload, firmware execution or OneNote
operation occurred. This is not legally certified clean-room review. After the
owner paused Codex, Claude (Anthropic) published these staged documents. It
clarified the TEST-0020 HQ-pending wording and this attribution only, with no
new collection or execution.
