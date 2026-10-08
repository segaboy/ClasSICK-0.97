# Source catalog

Access/review date for external sources: **2026-10-07** unless noted. Catalog
eligibility applies to the stated scope, not every resource reachable from a URL.
No Apple executable, source, disassembly, decompilation, or copied asset was used.
No reference-system black-box observation has yet been performed.

## SRC-0060 — Owner's first isolated VM trial go-ahead

- Owner, 2026-10-08, responding to the explicit trial question:
  "Yes—prepare and run the isolated trial".
- Authorizes a reviewed isolated VM configuration and recorded owned-media attach
  procedure, first black-box execution/qualification, timing/input/serial/screen
  evidence, repeat cold starts and ordinary project publication/HQ Git planning.
- SRC-0059 conditions persist. No firmware extraction/study, Apple material,
  existing-VM modification, physical disk writing, delegation, new chats,
  automation or live OneNote. Macintosh/mini vMac and three-edition goals persist.
- OpenAI Codex (GPT-6) resumes implementation leadership from the owner's Claude
  continuation prompt. Requirement only; not evidence of a boot or compatibility.
- The owner then accepted a live VM keyboard test and confirmed cold-04:
  “I tapped Space twice and saw the scene change.” This is scoped operator
  observation, corroborated by our own Space counter, not historical parity.

## SRC-0062 — Project-owned native VM observation artifacts

- Own unchanged SPEC-0014 disk and SPEC-0013 native image observed 2026-10-08
  under SPEC-0015 / TEST-0022; [sanitized evidence](../docs/development/vm-trial-evidence.md).
- Six uninterrupted cold starts reach the native 60-second endpoint; a separate
  synthetic Escape trial checks early stop. cold-04 has owner-confirmed live
  Space input. Local original-scene captures and own serial/host-clock records
  are retained with hashes; only own text and sanitized metadata are published.
- Original project output, GPL-3.0-or-later. No protected reference system,
  firmware pixels/internals, implementation input or Apple material. Formal B2
  qualification and human provenance review remain incomplete; no edition,
  physical hardware or historical Macintosh compatibility claim.

## SRC-0061 — Installed VirtualBox public management interface

- Oracle VirtualBox 7.2.16r174877 public VBoxManage usage observed 2026-10-08,
  scoped createvm, modifyvm, modifynvram Secure Boot configuration, convertfromraw,
  clonemedium, storagectl/storageattach, showvminfo, startvm/controlvm screen,
  scan-code input and power-off operations. Reuses SRC-0035's exact publisher
  [manual](https://download.virtualbox.org/virtualbox/7.2.16/UserManual.pdf),
  sections 15.7, 15.12/15.13, 15.15, 15.34/15.35, 15.44, 15.47–15.49; used locators and
  executable/usage hashes are retained with TEST-0022.
- Only public command contracts and existing primary manual text are reviewed.
  No tool/firmware/device implementation body, dump or external driver code.
  Native machine behavior is not inferred from help text or model names.
- Earlier COM access denial did not recur in the approved management session.
  Unsupported subcommand `--help` probes returned usage with nonzero exits;
  they are retained as discovery attempts, not successful trial commands.

## SRC-0059 — Owner's firmware test-platform decision

- Claude reported the documents-only firmware result on 2026-10-08: no permitted
  document can bind the installed VirtualBox 7.2.16 x64 EFI image to its published
  source (only a rebuild could, which stays excluded), while the binding policy
  governs what enters System 0.97, not the platform it runs on. Offered: clear it
  as a black-box test platform, keep it blocked, or seek another firmware. The
  owner chose "Test platform only (Recommended)".
- Scope: VirtualBox 7.2.16 x64 EFI may serve only as a black-box execution
  platform for our own images, under the conditions in the
  [firmware decision](../docs/development/firmware-eligibility.md). It does not
  certify the firmware's provenance or rights, adopt or redistribute it, permit
  extraction or study of its internals, or authorize a VM trial; that needs a
  separate explicit owner go-ahead. Physical PCs' vendor firmware is treated the
  same way when those gates come. Requirement only; no edition claim.

## SRC-0056 — Owner's boot-media continuation and FAT32 decision

- Owner, 2026-10-08: "ok, do it." after Claude proposed the two preparation tracks
  that need no VM: original FAT/UEFI media packaging with independent checks, and a
  documents-only firmware eligibility review. Ordinary specification, code, tests,
  provenance, commits/pushes and HQ registration under the existing workflow.
- Asked whether Claude could fetch Microsoft's FAT32 specification through the
  owner's desktop browser pane, the owner chose "Yes, download it". Asked to accept
  its embedded agreement, the owner asked about alternatives (Linux FAT support was
  explained as excluded implementation code; ECMA-107/FAT16 was offered), then
  answered: "Our goal is to only clean room system 0.97. Adding FAT32 support is
  outside of that. I think that's acceptable."
- Recorded reading, stated back to the owner in chat with an offer to switch to
  ECMA-107/FAT16: the owner accepts the Microsoft specification terms for this PC
  boot tooling, and the clean-room requirement governs the System 0.97
  reimplementation rather than published PC interface specifications used under
  their own terms. No correction had been received at publication. This does not
  relax the no-Apple-material rule, permit reading other implementations, waive
  firmware eligibility or authorize a VM, native run, delegation or OneNote.
- Claude (Anthropic; session configured as `claude-opus-5-5`) leads this work.
  Human provenance/rights review pending; this is not legal advice.

## SRC-0057 — Microsoft FAT32 File System Specification

- Microsoft, *Microsoft Extensible Firmware Initiative FAT32 File System
  Specification — FAT: General Overview of On-Disk Format*, Version 1.03,
  December 6, 2000; legal agreement updated March 30, 2011. Publisher copy
  <https://download.microsoft.com/download/1/6/1/161ba512-40e2-4cc9-843a-923143f3456c/fatgen103.doc>
  (UEFI 2.11 "Links to UEFI-Related Documents" names this document), fetched
  2026-10-08 into the owner's desktop browser pane: 222,720 bytes, SHA-256
  `b17d66c796d9cd3070adf4ccbc00add5a8b6b6f5491fe8ea949df778fce60172`, served
  Last-Modified 17 Feb 2025. Text was extracted in the page for review; the
  document was not saved to the repository or vendored.
- Reviewed: Boot Sector and BPB tables (offsets 0–35), FAT32 structure at offset
  36, sector signature notes, FAT data structure and cluster/sector arithmetic,
  FAT type determination, FAT32 28-bit entries/EOC/bad mark, FAT[0]/FAT[1], FAT
  volume initialization (FAT32 sectors-per-cluster table and FAT size
  computation), FSInfo and backup boot sector, 32-byte directory entries, name
  rules, attributes, dot/dotdot, date/time formats and size limits. Long-name
  sections were scanned only to confirm short names suffice; not implemented.
- The document's C code fragments were read as interface description; none were
  copied. The SPEC-0014 writer and both checkers are original.
- Agreement: royalty-free copyright license and patent covenant limited to
  products that comply with the unmodified specification and are used for UEFI
  boot, install, setup, repair, diagnostics or inventory purposes; both terminate
  if the user initiates patent litigation against Microsoft or covered parties;
  export compliance applies. Accepted by the owner under SRC-0056. Scope fit and
  rights are pending human review; no Microsoft code or boot code is used.

## SRC-0058 — UEFI Forum published media and partition interfaces

- *UEFI Specification* 2.11 (same publisher PDF and SHA-256
  `a64b8e442004b91becc3de9afaf8ca61b259a9a3b436accb6b3711ab5400cee9` as
  SRC-0034/0038), reviewed 2026-10-08 from the owner's local reference copy by
  text extraction in the owner's Cowork Linux shell.
- 4.2 CRC note (PDF 174); 5.1–5.3.3 with Tables 5.1–5.8 (printed 111–120 /
  PDF 195–204): legacy and protective MBR, GPT overview, header, entry array,
  ESP type GUID, attributes and alignment guidance; 13.3–13.3.4.3 (462–467 /
  PDF 546–551): FAT variants, system partition, names, directory structure,
  partition discovery and media formats; Appendix A Table A.1 (1970 / PDF 2054)
  GUID storage. The removable-media file name is SRC-0038's 3.5.1.1 review.
- Eligible public interface prose and tables. Adjacent structure declarations
  were visible, not copied. Does not establish any firmware's actual FAT/GPT
  behavior, VirtualBox media handling or historical Macintosh behavior.

## SRC-0055 — Owner's readiness audit request

- Owner, 2026-10-08: "i typed 1 in error. Please perform the audit."
- Scope: audit current retained evidence, native-PC boot readiness and provenance
  prerequisites; record findings under the existing documentation/publication
  workflow. The accidental numeric reply grants no additional work or delegation.
- Requirement only. No firmware waiver, VM/native execution, additional agent,
  new chat, automation, live OneNote or change to the real Macintosh/mini vMac
  and three-edition requirements. TEST-0020 is aggregate evidence review;
  UART implementation boundaries and human review remain separate.

## SRC-0053 — Owner's single fresh UART implementation-agent authorization

- Owner, 2026-10-08: continue the missing native boot components with exactly
  one fresh implementation agent for bounded polling UART diagnostics; no inherited
  conversation/search history. Specifications, implementation, original fixtures,
  audits, provenance and ordinary commits are authorized under the existing scope.
- Requirement only; firmware eligibility remains needs-review. No native/VM
  execution, additional agents/chats, automation, live OneNote or change to the
  independent real Macintosh/mini vMac and three-edition goals. Implementation
  work is separately recorded from the lead's HQ/publication responsibility.
- OpenAI Codex (GPT-6) assisted; fresh context and self-review are not a legally
  certified or independently staffed clean-room procedure. Human review pending.

## SRC-0054 — TI TL16C550C published UART interface

- Texas Instruments, *TL16C550C Asynchronous Communications Element with
  Autoflow Control*, SLLS177I, March 1994, revised March 2021. Publisher
  <https://www.ti.com/lit/ds/symlink/tl16c550c.pdf>, directly reviewed from the
  owner's local reference copy 2026-10-08. SHA-256
  `3ef2d68733a8599cc2d0d7300b69c3530bc714de3ebb0fcfda03df60a4c7694e`.
- PDF/printed 21–23, Tables 7-1/7-3 register selection/DLAB/bit declarations;
  7.7.2 on 24 (FCR); 7.7.4/5 on 25 (polling/IER); 7.7.7 on 27 (LCR,
  Tables 7-6/7); 7.7.8 on 28 (LSR/error-read/THRE/TEMT); 7.7.9 on 29
  (MCR/Table 7-8); 7.7.11 on 31 (baud equation/Table 7-9); 7.7.13/14 on
  32 (SCR/THR). Selected prose and register tables were visually reviewed using
  PDFium images in the ignored owner workspace; pypdf extraction corroborated it.
- Adjacent block/reset/FIFO interrupt/IIR/MSR/RBR and clock circuit declarations
  were visible, unused beyond contextual read-only interface review. No code
  samples, third-party drivers/headers/OS implementations, search engines or
  firmware bodies were consulted. Initial extraction hit a local Windows encoding
  error; UTF-8 extraction then succeeded. Document/images are not vendored.
- Eligible scoped hardware interface only, copyrighted documentation linked rather
  than copied. Does not establish universal 16550 support, VirtualBox device
  implementation, PC address decoding, clock or native delivery. SPEC-0013's
  proposed 0x3F8/1.8432-MHz profile, scratch/readback checks, queue, time/call
  limits and conservative terminal error policy are original design decisions.
  No historical Macintosh behavior or external implementation/license adoption.

UART exposure boundary: existing eligible project contracts and owned source,
SRC-0052's already reviewed original byte-port instructions and SRC-0046 timer
are reused. The fresh implementer did not retrieve uncertain source or firmware.
Provenance eligibility and human rights review remain distinct from test success.

## SRC-0048 — Owner's return-to-Codex keyboard handoff

- Owner-supplied continuation, 2026-10-08, from Claude's verified handoff:
  resume SPEC-0012 through specification, implementation, verification and normal
  documentation/commit/push publication. Real Macintosh/mini vMac, all three
  editions and firmware provenance gates remain required. No delegation, new
  chats, automation, OneNote publication or firmware waiver is authorized.
- Requirement only. OpenAI Codex (GPT-6) is the implementation assistant for this
  checkpoint; implementation-lead self-review and human review remain distinct.

## SRC-0049 — Published PS/2-mode controller interfaces

- SMSC, *KBD43W13 Keyboard and PS/2 Mouse Controller*, undated 22-page edition,
  manufacturer-issued datasheet archived at
  <https://k.lse.epita.fr/data/8042.pdf>, accessed 2026-10-08. SHA-256
  `20f990798e61f3f876864a6e14667bd907d8c7c4ec30718b2acf85a5551ae678`.
  Reviewed buffers on printed/PDF 8, PS/2 status on 10, configuration and
  commands on 12, and AB/AD/AE interfaces on 13. AT-mode status meanings on 8
  are not substituted for the PS/2-mode contract. No internal implementation,
  firmware, third-party header or driver is an input.
- Holtek, *HT6542B Keyboard Controller with PS/2*, November 30, 1995, printed/PDF
  5 power-on self-test/status prose (55 success). Manufacturer-issued archived copy
  <https://telcontar.net/KBK/Holtek/docs/Holtek%20HT6542B%20datasheet.pdf>, accessed
  2026-10-08; SHA-256
  `27f40a0e58c9b476f2b39fbc51c9ec433686f6019a9b724997f91ef061d7910f`.
  Corroborates the success code for that chip's power-on self-test; the explicit
  AA command comes from SMSC. Requiring 55 after AA is the original qualified
  profile's acceptance condition, not a documented universal-device guarantee.
  This evidence does not establish another
  device's implementation or VirtualBox conformance. SPEC-0012 is a qualified
  compatible-device profile, not a universal i8042 claim.
- Copyrighted interface documentation is linked, not republished. Selected-page
  text/visual review occurred in the ignored owner workspace. No sample code used.

## SRC-0050 — Published PS/2 keyboard protocol and set-2 codes

- Holtek, *HT82K629B USB + PS/2 Keyboard Encoder*, revision 1.10, August 30,
  2022, publisher <https://www.holtek.com.tw/webapi/11842/HT82K629Bv110.pdf>,
  accessed 2026-10-08; SHA-256
  `0fcc9cb8cb394c8f709edfec9f47bf5a92ffd3b06eb8823bfb71d9098b690979`.
  Printed/PDF 4–6: reset/BAT, ACK/resend/error, F5/F0/02/F4 commands and timing;
  10: Space/Escape set-2 make/break bytes; 11: extended/Pause prefixes.
  Hardware interface prose/tables only; adjacent USB/circuit/unused commands
  are not adopted. The protocol does not establish historical Macintosh behavior.
- SPEC-0012's timeouts, two retries, two-key scope and parser recovery are
  original policy choices. No third-party implementation or asset is copied.

## SRC-0051 — ACPI controller-present indication

- Extends SRC-0036/0046's pinned ACPI 6.6 publisher PDF (same SHA-256):
  section 5.2.9.3, Table 5.11, PDF 196 / printed 125, IAPC_BOOT_ARCH bit 1;
  Table 5.9's field offset 109. Reviewed 2026-10-08, including visual page review.
  Eligible interface declarations; no actual firmware table or parser used.
  A declaration is a prerequisite, not proof of controller readiness or rights.

## SRC-0052 — Intel byte-port instruction interfaces

- Extends SRC-0039/0043: Intel SDM volume 2A, order 253666-093, September 2026,
  IN, PDF 573–574 / printed 3-455–456, same pinned hash
  `87c5acb6f27e24d9d364841a0d2346c91a8482e36f403409954e2d2a8c817bac`.
- Intel SDM volume 2B, order 253667-093, September 2026, OUT,
  PDF 179–180 / printed 4-171–172, publisher
  <https://cdrdv2-public.intel.com/929354/253667-093-sdm-vol-2b.pdf>, accessed
  2026-10-08; SHA-256
  `a261998ace8e07f624bf3e2486bb6cfe950fb8be8d0dc9631bcecb4c5ec953e4`.
  Opcode EC/EE, AL/DX operands and privilege/exception interface only. Original
  Microsoft-x64 register marshaling; no other assembly implementation consulted.

Discovery exposure for SRC-0049–0052: search results showed OSDev/Wikipedia
summaries and a NURVE Chameleon user-guide snippet containing driver/API examples.
Those snippets were not adopted or studied as implementation inputs; the full
guide and OS code were not opened. SMSC KBD42W11, Microchip LPC47M172 and Holtek
HT6542B datasheet discovery was limited to published hardware interface pages.
Only the catalogued scoped contracts support SPEC-0012. No Apple implementation,
executable, ROM, copied asset or uncertain firmware body was retrieved or used.

## SRC-0047 — Owner's progress-loop continuation

- Owner, 2026-10-08: "do it." This came after the SPEC-0010 handoff named the
  60-second timed progress loop next. It authorizes SPEC-0011, tests, provenance
  and normal publication under SRC-0044. No external source is added.
- Requirement only; no firmware waiver, VM launch, OneNote or edition claim.

## SRC-0045 — Owner's PM-timer continuation

- Owner, 2026-10-08: "Do it." This came after the lead's SPEC-0009 handoff named
  the ACPI PM timer as the next concrete task. It authorizes SPEC-0010
  discovery/timer work, tests, provenance and normal publication under SRC-0044.
- Requirement only; no firmware waiver, VM launch, OneNote or edition claim.

## SRC-0046 — ACPI 6.6 system description table interfaces

- Same pinned publisher PDF as SRC-0036 (SHA-256
  `8c7542dd4de974ae47bba71bb0336637fe1e3838daad7692370ab4cf218efd35`), accessed
  2026-10-08 from the owner workspace copy. Sections 5.2.3.2 / Tables 5.1–5.2
  (GAS, PDF 172–174), 5.2.5.2–5.2.5.3 / Table 5.3 (UEFI RSDP location and GUIDs,
  RSDP fields, PDF 175–176), 5.2.6 / Table 5.4 (header, PDF 176), 5.2.8 / Table
  5.8 (XSDT, PDF 181), and 5.2.9 / Table 5.9 offsets 0–131 and 208–219 (FADT
  PM_TMR_BLK, PM_TMR_LEN, Flags, X_PM_TMR_BLK, PDF 182–188). Table 5.10 bits 8
  (TMR_VAL_EXT) and 20 (HW_REDUCED_ACPI), PDF 190–194.
- Text was extracted locally with pypdf into the owner's ignored workspace.
  Eligible published interface tables/prose only. No ASL, sample OS code,
  firmware tables or third-party parser consulted. The original policy (ACPI 2.0+
  only, one FADT, I/O-space timer, map-type checked reads) is narrower than ACPI.

## SRC-0044 — Owner's Claude implementation-lead handoff

- Owner, 2026-10-08: a written handoff that made Claude (Anthropic) the 0.97
  implementation lead from the verified state at 2bceffc / HQ 2e160cc. It
  authorizes continuing the missing B2 boot components, specifications,
  implementation, tests, provenance, documentation, commits and pushes. It
  recommends native framebuffer presentation and an original post-handoff scene
  next. Later the same day, after the lead proposed Linux-VM development checks
  plus pinned Windows gates, the owner said "Do whatever you feel is best".
- Requirement only; no firmware provenance waiver, delegation, live OneNote
  instruction, historical identity or completed edition. Existing scope, policies
  and Macintosh/mini vMac goals preserved.
- SPEC-0009 reuses SRC-0034's reviewed UEFI 12.9.2 GOP pixel-format and
  pixels-per-scanline declarations. On 2026-10-08 the lead tried to re-open the
  publisher PDF (agent proxy 403) and the HTML chapter (fetched text truncated
  before 12.9). No newer UEFI text was reviewed. A web search listed third-party
  GOP header titles, which were not opened or used.

## SRC-0042 — Owner's independent PC-boot continuation

- Owner, 2026-10-08: "ok, let's do it" following the status explanation of the
  next minimal independent PC boot and remaining Macintosh goals. Authorizes
  missing startup components, original specifications/implementation/tests and
  normal Git evidence/wiki plans. No firmware provenance waiver or boot claim.

## SRC-0043 — Intel published x64 descriptor and exception interfaces

- Intel SDM Volume 3A, 253668-090US, February 2026, accessed 2026-10-08;
  [publisher PDF](https://cdrdv2-public.intel.com/874249/253668-090-sdm-vol-3a.pdf),
  SHA-256 `42166ab4aeb53df119a794a1da36eaa8a9f123eea61d90e8fe6195da4ef6654f`.
  Sections 3.4–3.5 (segment descriptors/tables), 6.8.5.1 (far transfers),
  7.12.1.3–7.14.5 (interrupt gates/error codes/frame/IST), Table 7-1 / PDF 202–203 and 7.15
  (exception vectors), 10.2.3–10.2.4 and 10.7 (TSS descriptor/register/format).
  Figures 7-8, 10-4, 10-11 visually reviewed at PDF pages 220,295,308;
  associated prose PDF 217–222,294–295,307–308. Public ISA layouts/prose only.
- Extends SRC-0039 Volume 2A 253666-093US, September 2026, same pinned hash;
  LGDT/LIDT printed 3-553–555 / PDF 671–673 reviewed. No sample implementation,
  OS source, external headers or firmware adopted. Independently specified table
  layout, allocation and terminal policy; no historical Macintosh applicability.
- Initial sandbox download lacked DNS and local PyMuPDF was absent; reviewed
  publisher download succeeded with network permission, using bundled pypdf and
  PDFium rendering. Manual remains local, linked rather than vendored.

## SRC-0037 — Owner's native loader and firmware-review continuation

- Owner, 2026-10-08: "OK, do it" after the proposed exact firmware eligibility
  review and native loader. Original interfaces/loader/transition/tests, inspected
  own EFI packaging, evidence/publication and Git wiki plans are authorized.
- Requirement only; no firmware provenance waiver, VM launch, historical identity,
  completed OS edition or OneNote publication instruction.

## SRC-0038 — UEFI Forum published loader interfaces

- *UEFI Specification* 2.11, cover Nov 21, 2024; accessed 2026-10-08.
  [Publisher PDF](https://uefi.org/sites/default/files/resources/UEFI_Spec_Final_2.11.pdf),
  SHA-256 `a64b8e442004b91becc3de9afaf8ca61b259a9a3b436accb6b3711ab5400cee9`.
- Extends SRC-0034's x64 calling/map/GOP/exit review. Sections 4.1–4.4 (printed
  87–94 / PDF 171–178), 7.2.1–7.2.2 (154–156 / PDF 238–240), 7.3.7 and
  7.3.9 (172–173,175–178 / PDF 256–257,259–262), 7.3.16 (193–194 / PDF
  277–278), 7.5.1 (205–206 / PDF 289–290), 7.5.3 (209–210 / PDF 293–294),
  9.1 (255–257 / PDF 339–341), 3.5.1.1/Table 3.4 (85 / PDF 169), Appendix
  D status codes (1984–1985 / PDF 2068–2069): table/CRC, allocation, protocol
  access, watchdog, LoadedImage and removable x64 payload name.
- Eligible public interface prose/declarations; adjacent protocol implementation
  examples were visible, neither copied nor adopted. No external UEFI headers,
  firmware library, example loader or document redistribution. Original bounded
  SPEC-0007 policy is narrower than general UEFI support. No Macintosh applicability.

## SRC-0039 — Intel public x64 instruction interfaces

- Intel, *64 and IA-32 Architectures Software Developer's Manual*, Volume 2A,
  253666-093US, September 2026; accessed 2026-10-08.
  [Publisher index](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html),
  [publisher PDF](https://cdrdv2-public.intel.com/929353/253666-093-sdm-vol-2a.pdf),
  SHA-256 `87c5acb6f27e24d9d364841a0d2346c91a8482e36f403409954e2d2a8c817bac`.
- CLD printed 3-141 / PDF 259, CLI 3-148–149 / PDF 266–267, HLT 3-439 /
  PDF 557: direction flag, CPL/interrupt-mask semantics and halt/resume behavior.
- Eligible ISA interface, including instruction pseudocode, not an OS algorithm.
  Original minimal stack/call/terminal assembly uses the reviewed x64 UEFI ABI.
  CLI does not mask exceptions/NMI; descriptor tables/fault handlers are not owned
  in this scaffold. No Intel implementation or firmware adopted. Initial local
  PDF printing failed character encoding, then bounded UTF-8 extraction succeeded.
- SPEC-0014 addendum (2026-10-08, Claude): JMP rel8 `EB cb` (printed 3-503–504 /
  PDF 622–623 of the same volume and hash): short jump with a sign-extended 8-bit
  displacement added to the address of the next instruction. Used with CLI/HLT
  only for the four-byte legacy stub in the FAT32 boot sector; UEFI never runs it.

## SRC-0040 — Exact VirtualBox x64 producer and selected module metadata

- Oracle official v7.2.16 tag `4cf0b89546257f5044534a7b94cde2dac1e8c175`,
  accessed 2026-10-08. Extends SRC-0035; eligible build/license declarations only.
  Recursive source-tree path inventory SHA `d84e06dbfafbe9d8001be6179a662f9d857a1645`
  was used to locate build files, not read implementation bodies.
- [Firmware build manifest](https://github.com/VirtualBox/virtualbox/blob/4cf0b89546257f5044534a7b94cde2dac1e8c175/src/VBox/Devices/EFI/Firmware/Makefile.kmk),
  blob `4910daa4af034774c3408814dfb23dfa9bba92e5`, line 120 and x64 build
  lines 424–453: OVMF.fd is renamed VBoxEFI-amd64.fd; OvmfPkgX64.dsc/fdf are
  selected with VBOX=1/VBOX_WITH_OVMF=1. Conditional feature definitions visible.
- [x64 DSC](https://github.com/VirtualBox/virtualbox/blob/4cf0b89546257f5044534a7b94cde2dac1e8c175/src/VBox/Devices/EFI/Firmware/OvmfPkg/OvmfPkgX64.dsc),
  blob `9cb47d072f0b54c41232785c051e600f41ce62e0`, copyright/license/build
  declarations; [x64 FDF](https://github.com/VirtualBox/virtualbox/blob/4cf0b89546257f5044534a7b94cde2dac1e8c175/src/VBox/Devices/EFI/Firmware/OvmfPkg/OvmfPkgX64.fdf),
  blob `1bee9af6836349d7a67c42663048aac9582a5d82`, VBOX-selected lines 327–334
  include VBoxHfs, VBoxAppleSim and VBoxApfsJmpStartDxe among other modules.
- Their .inf build/license declarations only were read: VBoxHfs blob
  `1894a70523fb781549ae161264204eb898faa123`, VBoxAppleSim
  `30a0fd54471b381df987c2691ad700ef7ec9b428`, VBoxApfsJmpStartDxe
  `6bb7302ceecd63649df9cc2d728f8d7601490ccb`. They declare Oracle copyrights
  and GPL-3.0-only OR CDDL-1.0; implementation filenames were visible, no bodies
  fetched. Names/notices do not prove Apple code inclusion or clean provenance.
- Exact x64 producer is now identified. Installed binary-to-source correspondence,
  transitive source/notice/derivation closure and rights remain needs-review.
  The earlier Bhyve notice is not automatically selected x64 content. No firmware
  extraction, machine-code inspection, uncertain implementation adoption or VM run.
  No external metadata contents are vendored or used to implement the loader.

## SRC-0041 — Microsoft public PE relocation and section interfaces

- Microsoft Learn, [PE Format](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format),
  accessed 2026-10-08; extends SRC-0032. Optional Header Windows-Specific Fields,
  Data Directories, Section Table/Flags and .reloc/Base Relocation Block/Types:
  EFI application subsystem, section/raw/image alignment and permissions, bounded
  directory/block/type-offset layout and DIR64 delta semantics.
- Eligible public file-format prose/declarations. Adjacent import/exception/TLS
  declarations/example visible, unused. Original PowerShell bounded file/map
  auditor; no external PE parser or firmware relocator adopted. This checks our
  unloaded output, not actual firmware loading/relocation or hardware permissions.

## SRC-0033 — Owner's native preboot-contract continuation

- Owner, 2026-10-08: "OK, move on" after proposed firmware handoff/device contracts
  before the first VirtualBox boot. Scoped original contracts/validators/tests,
  provenance and normal evidence/Git wiki publication. No live OneNote request.
- Requirement only; no firmware eligibility waiver, OS boot or historical result.

## SRC-0034 — UEFI Forum published x64 boot interfaces

- *UEFI Specification*, Release 2.11, cover Nov 21, 2024; accessed 2026-10-08.
  [Publisher PDF](https://uefi.org/sites/default/files/resources/UEFI_Spec_Final_2.11.pdf),
  SHA-256 `a64b8e442004b91becc3de9afaf8ca61b259a9a3b436accb6b3711ab5400cee9`.
- 2.3.4 (printed 28–31 / PDF 112–115), 7.2.3 (156–160 / PDF 240–244),
  7.4.6 (203–204 / PDF 287–288), 12.9.2 (445–449 / PDF 529–533): calling
  convention, map stride/key/descriptors, exit lifetime/retry and GOP fields.
- Eligible interface prose/declarations, not firmware implementation. Adjacent
  allocation/exit prose and GOP bitmask sample were visible, not copied/adopted.
  Original fixed-format validators only. HTML 403/oversized web PDF preceded
  publisher download and bounded local extraction. No PDF/example vendored.
- Project limits/policies are distinct from UEFI conformance; no Macintosh claim.

## SRC-0035 — Oracle VirtualBox interfaces and firmware eligibility metadata

- Installed Oracle VirtualBox 7.2.16r174877; read-only version/filename/hash
  inventory, no machine-code inspection. *User Guide for Release 7.2*,
  [publisher manual](https://download.virtualbox.org/virtualbox/7.2.16/UserManual.pdf),
  sections 5.8/5.13/5.14/5.18/5.22/15.35/18.2.3.12, PDF pages 68–69,71–75,
  79–83,411,518–520
  and published device/setting prose. Local manual SHA-256
  `bc95af3c86f226c253070e8c5916057822fc6f97c8beb89b1127797899e8a1ad`.
- VBoxManage.exe SHA-256 `7fda8e54157eb94a5ebd9cea079a9fb8c23ade92ae71e13399dc3431c021a228`;
  VBoxDD2.dll `6cd01002214a130353b9e12405fa836b5efef7461e04e451460e529109796487`.
  The DLL fingerprint is not a standalone firmware-image fingerprint.
- Official [v7.2.16 tag](https://github.com/VirtualBox/virtualbox/tree/4cf0b89546257f5044534a7b94cde2dac1e8c175)
  and path inventory only. [VBoxPkg.dsc](https://github.com/VirtualBox/virtualbox/blob/4cf0b89546257f5044534a7b94cde2dac1e8c175/src/VBox/Devices/EFI/Firmware/VBoxPkg/VBoxPkg.dsc)
  (Git blob 3b663df43a995e45b6b0b9ed79b2b8461a5f28ea) declares ARM/AARCH64,
  so cannot validate x64. [Device packaging manifest](https://github.com/VirtualBox/virtualbox/blob/4cf0b89546257f5044534a7b94cde2dac1e8c175/src/VBox/Devices/Makefile.kmk)
  (blob 146f5548bfb41200031c51cac15fc1168c5bdd36) names embedded amd64 firmware
  inputs. No module implementation, firmware extraction or whole source archive fetched.
- Eligibility: interface/version/build/license metadata only. License inventory
  lists Apple-attributed portions in OVMF/Bhyve; it does not prove Apple ROM code
  or selected-image inclusion. Exact x64 source/dependency/binary closure remains
  unresolved; firmware needs-review and is not adopted. Product/permissive-license
  labels are not approval. External VM/runtime distribution rights remain separate;
  no Extension Pack or guest image adopted. Accessed 2026-10-08.
- VBoxManage modifyvm help failed COM E_ACCESSDENIED; no VM/settings examined or
  changed. Missing guessed metadata URL and ARM manifest trial are unsuccessful
  eligibility leads. Unrelated search snippets discarded; no third-party/Apple-derived
  implementation input. Public manual examples visible, not copied/adopted.

## SRC-0036 — UEFI Forum published ACPI PM timer interface

- *ACPI Specification*, Release 6.6, May 13, 2025; accessed 2026-10-08.
  [Publisher PDF](https://uefi.org/sites/default/files/resources/ACPI_Spec_6.6.pdf),
  SHA-256 `8c7542dd4de974ae47bba71bb0336637fe1e3838daad7692370ab4cf218efd35`.
- 4.8.1.4, 4.8.2.1 and 4.8.3.3 / Table 4.14 (printed 67–68,83; PDF 138–139,154):
  optional counter width/frequency/access. Nearby fixed-register prose was visible,
  unused; 5.2.9 FADT located but not reviewed as parser input.
- Eligible interface for proposed timer contract; no timer/FADT parser or native
  driver implemented, no physical precision claim. HTML 403/incorrect PDF-name
  lead preceded publisher download. No code copied or document vendored.

## SRC-0030 — Owner's standalone-link and second-compiler continuation

- Author/date: owner, 2026-10-08, "Do it" after proposed standalone linking and
  another compiler before native boot work.
- Scope: current-core link/runtime-boundary proof, necessary portable compiler,
  tests/provenance/publication and Git wiki plans. B1 is already passed.
- Requirement only; no firmware/VM selection, native boot or live OneNote request.

## SRC-0031 — Portable GCC development distribution

- Christopher Wellons / w64devkit, [immutable v2.10.0 release](https://github.com/skeeto/w64devkit/releases/tag/v2.10.0),
  published 2026-09-14; release API asset digest/size and
  [pinned README](https://github.com/skeeto/w64devkit/blob/v2.10.0/README.md),
  Usage, Special linking considerations, Notes and Licenses; accessed 2026-10-08.
- Package includes GCC 16.2.0 / GNU Binutils 2.47.20260726. Lock records official
  package and observed compiler/linker hashes. Extracted tool/version outputs and
  bundled COPYING.MinGW-w64-runtime.txt notices were inspected. Upstream
  [UNLICENSE](https://github.com/skeeto/w64devkit/blob/v2.10.0/UNLICENSE) describes
  the packaging project, not every bundled component's license.
- Eligible external development tools; no tool implementation source fetched,
  adopted or copied. README shell examples and runtime-helper descriptions were
  visible, not adopted. GCC/MinGW hosted-test runtimes remain development-only;
  standalone fixtures link no runtime library or helper. No binary redistribution.
- GCC Runtime Library Exception link discovery timed out; no legal conclusion or
  future product-runtime approval relies on its unread text. Preserve bundled
  notices and independently review any later distribution or runtime adoption.

## SRC-0032 — Compiler/linker and PE interface contracts

- Accessed 2026-10-08: GNU [GCC link options](https://gcc.gnu.org/onlinedocs/gcc-16.2.0/gcc/Link-Options.html),
  -nostdlib/-nodefaultlibs/-e; [C dialect options](https://gcc.gnu.org/onlinedocs/gcc/C-Dialect-Options.html),
  freestanding; GNU Binutils [ld options](https://sourceware.org/binutils/docs/ld/Options.html)
  and [WIN32](https://sourceware.org/binutils/docs/ld/WIN32.html), entry/map/subsystem;
  LLVM [Clang manual](https://clang.llvm.org/docs/UsersManual.html) and
  [LLD Windows support](https://lld.llvm.org/windows_support.html); Microsoft
  [PE format](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format),
  COFF/optional/section headers and data-directory interface fields.
- Current documentation versions may differ from pinned tools; the observed
  commands/maps/bytes define tested scope. Pinned GCC C-dialect URL was unavailable;
  the public current manual was read. Bundled tool --version/help and header
  declarations are interfaces; no compiler/linker implementation code inspected.
- Examples/interface snippets were visible; original probe/auditor/scripts only.
  No Apple implementation input or historical Macintosh applicability.

## SRC-0028 — Owner's B1 audit and live viewer validation

- Author/date: owner, 2026-10-07–08, "OK, do it" after proposed remaining
  hosted-start audit; on 2026-10-08 selected "I can test now—open the viewer".
- Scope: existing B1/M0.1 audit, necessary combined checks/operator validation,
  ordinary provenance/publication and Git wiki plans. No new core requirement,
  OS boot claim, historical-source approval or live OneNote publication request.
- Owner-operated test is project validation, not a reference-system observation.

## SRC-0029 — Microsoft queued-message interfaces

- Microsoft Learn, accessed 2026-10-08, parameters/returns/remarks of
  [PostMessageW](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-postmessagew)
  (updated 2023-03-21) and
  [GetMessageW](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmessagew).
- Host message posting, retrieval, quit/error handling and ordering prose.
  Adjacent public examples were visible, not copied or used as implementation.
  Original bounded integration uses existing project Win32 interfaces.
- No input-origin guarantee, historical Macintosh behavior or new runtime/tool
  adoption follows. Posted key messages remain synthetic host-level fixtures.

## SRC-0025 — Owner's debugger checkpoint authorization

- Author/date: owner, 2026-10-07, "Do it" after proposed debugger verification.
- Scope: hosted debugger tests, necessary viewer launch fixes, evidence/publication
  and Git wiki plans. Existing independent core specifications remain authoritative.
- Requirement only; no B1, native boot, historical behavior or live OneNote request.

## SRC-0026 — LLVM LLDB command interfaces and diagnostic lead

- LLVM project, accessed 2026-10-07: [Tutorial](https://lldb.llvm.org/use/tutorial.html)
  command structure, breakpoints, frame variables and thread stepping;
  [command map](https://lldb.llvm.org/use/map.html), source-map and launch workflow.
  Bundled LLDB --help, process-handle help and version output were also inspected.
- Published command-interface prose/examples, not tool implementation source.
  Original verification commands and evidence checks; no copied implementation.
- Diagnostic lead: [LLVM issue 58065](https://github.com/llvm/llvm-project/issues/58065)
  reports a similar WOW64 frame symptom. Its diagnostic assembly excerpt was visible,
  not copied or used as an implementation input. The local failure is independently
  observed; shared root cause is an inference, not established by the report.
- No Apple implementation inputs, historical applicability or universal debugger claim.

## SRC-0027 — Microsoft runtime argument interface

- Microsoft Learn, [__argc / __argv / __wargv](https://learn.microsoft.com/en-us/cpp/c-runtime-library/argc-argv-wargv?view=msvc-170),
  remarks and header requirements, accessed 2026-10-07. Existing pinned stdlib.h
  declarations were checked; runtime implementation source was not inspected.
- Host-only parsed argument count/strings replace raw WinMain string comparison.
  This uses the already linked Windows development runtime, with no new dependency
  or portable-core change. No documentation sample implementation adopted.

## SRC-0023 — Owner's clock continuation and VirtualBox availability

- Author/date: owner, 2026-10-07, after discussing the next clock milestone.
- Instruction: retain installed VirtualBox for future boot tests and move on.
- Scope: SPEC-0005 time/provider/viewer tests and ordinary provenance/publication.
  VirtualBox availability is a project requirement/tooling hint, not an OS boot,
  VM/firmware provenance approval or replacement for Macintosh validation.
- Read-only local VBoxManage version check returned 7.2.16r174877. No VM created,
  altered or started, firmware adopted, package redistributed or existing VM read.

## SRC-0024 — Microsoft elapsed-time and timer interfaces

- Microsoft Learn, accessed 2026-10-07. Parameters/returns/remarks of
  [QueryPerformanceCounter](https://learn.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancecounter),
  [QueryPerformanceFrequency](https://learn.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancefrequency),
  [SetTimer](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-settimer)
  and [KillTimer](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-killtimer),
  plus interface guidance in [Acquiring high-resolution time stamps](https://learn.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps).
- Published host-interface prose only. QPC's adjacent C++ example was visible
  during access; no sample implementation copied/adopted. Original portable time,
  host integer conversion, provider transitions, timed strip and independent tests.
- No historical Macintosh applicability, hardware timing certification, UTC,
  nanosecond accuracy or native timer/scheduler implementation claim.

## SRC-0021 — Owner's next implementation step

- Author/date: project owner, 2026-10-07: move to the next step after arenas.
- Requirement authority for SPEC-0004 normalized keyboard input, verification,
  provenance and ordinary publication/wiki maintenance. No historical/boot claim.

## SRC-0022 — Microsoft keyboard message interfaces

- Publisher: Microsoft Learn; accessed 2026-10-07. Interface prose: parameters,
  bit tables, returns and remarks of [WM_KEYDOWN](https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-keydown)
  and [WM_KEYUP](https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-keyup),
  plus Space/Escape entries in [Virtual-Key Codes](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes).
- Scope: Windows adapter only; one delivered message produces one normalized
  event. No Windows sample implementation copied; original mapping/queue/tests.
  The WM_KEYDOWN page's adjacent example was visible during documentation access;
  it was not used as an implementation source. No Apple material involved.
- No Macintosh applicability, keyboard layout/text, hardware driver or clock claim.

## SRC-0019 — Owner's boot-goal reaffirmation and continuation

- Author/date: project owner, 2026-10-07, in the implementation chat.
- Requirement: retain the clean-room, independently implemented 0.97 goal,
  bootable on a real Macintosh or mini vMac; then continue the project.
- Scope: next roadmap contract SPEC-0003 arenas and viewer integration, with the
  established verification/provenance/publication/wiki workflow. mini vMac is an
  intended validation path; existing physical-hardware edition gates remain.
- No emulator/firmware dependency, historical identity or completed boot approved
  by this requirement. Inputs remain behavioral contracts, never Apple code.

## SRC-0020 — WG14 C11 committee draft interface semantics

- Publisher: ISO/IEC JTC1/SC22/WG14, N1570, 2011-04-12; accessed 2026-10-07.
- [Committee draft](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf),
  sections 6.2.8 (alignment), 6.5 paragraphs 6–7 (effective type),
  6.5.6 (bounded pointer arithmetic), 7.19 (max_align_t) and 7.22.3
  (host allocation alignment/lifetime).
- Eligibility: published language/interface prose only. Independently designed
  monotonic arena and tests; no sample implementation copied or adopted.
- No historical Macintosh applicability, allocator algorithm source, runtime
  dependency or general compiler certification. Host heap use stays in the adapter.

## SRC-0017 — Owner's next milestone authorization

- Author/date: project owner, 2026-10-07, after the surface handoff.
- Instruction: proceed with the proposed Windows presentation/original-scene
  checkpoint, using the established implementation, verification, publication and
  wiki workflow. Requirement authority only; no historical or boot certification.

## SRC-0018 — Microsoft Win32 presentation interfaces

- Publisher: Microsoft Learn, live public Win32 documentation, accessed 2026-10-07.
- Sections used: parameters, return values and remarks of
  [StretchDIBits](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-stretchdibits),
  [BITMAPINFOHEADER](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader),
  [BITMAPINFO](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfo),
  [CreateDIBSection](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createdibsection),
  [RectVisible](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-rectvisible),
  [IntersectClipRect](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-intersectcliprect),
  [PrintWindow](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-printwindow)
  and the [painting lifecycle](https://learn.microsoft.com/en-us/windows/win32/learnwin32/painting-the-window).
- DPI: parameters/returns/remarks of
  [SetProcessDpiAwarenessContext](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setprocessdpiawarenesscontext),
  [AdjustWindowRectExForDpi](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-adjustwindowrectexfordpi),
  and message semantics of [WM_DPICHANGED](https://learn.microsoft.com/en-us/windows/win32/hidpi/wm-dpichanged).
- Eligibility: host interface prose and existing pinned Windows header signatures.
  Independent SPEC-0002/code; no documentation sample implementation, third-party
  rendering code or artwork incorporated. No Macintosh historical applicability.
- Limits: Microsoft recommends manifest DPI configuration for production apps.
  This bounded development viewer uses the documented API before any UI, avoiding
  new resource tooling. Actual cross-monitor changes/physical colors unverified.

## SRC-0016 — Owner's first subsystem implementation assignment

- Author/date: project owner, 2026-10-07; instruction retained by the owner.
- Public derivative: SPEC-0001 v1 and IMPL-0002, independently summarized.
- Classification: project requirement/implementation authorization after bootstrap.
- Scope: finalize bounded surfaces, implement without heap/platform dependencies,
  verify independent headless tests, freestanding boundaries and supported
  sanitizers, maintain provenance/status/wiki, commit and push the scoped result.
- Eligibility: requirement authority only, not a historical behavioral source.
- Limits: surfaces do not establish M0.1, QuickDraw, any bootable edition or exact
  January 1984 historical identity. No third-party implementation/asset input.

## SRC-0001 — Owner's founding brief

- Author/date: project owner, 2026-10-07; private instruction retained by owner.
- Public derivative: [charter](../docs/charter.md), independently summarized scope.
- Classification: authorized project requirements and design constraints.
- Applicability: all project decisions; not primary historical evidence.
- Limitations: internal System 0.97 identification and exact reference versions
  remain research questions; the brief is not proof of Apple implementation details.

## SRC-0002 — LLVM-MinGW publisher documentation and release

- Author: Martin Storsjo / LLVM-MinGW maintainers.
- [Project documentation](https://github.com/mstorsjo/llvm-mingw);
  [release 20260908](https://github.com/mstorsjo/llvm-mingw/releases/tag/20260908).
- Sections used: Releases / UCRT package descriptions; release asset metadata.
- Eligibility: development-tool documentation and binaries only, not historical
  implementation input. No upstream toolchain source copied into project code.
- Evidence: package URL/version/size/SHA-256 pinned in `toolchain-lock.json`;
  setup and native compiler output were independently exercised locally.
- Limits: Windows hosted support does not prove original m68k or bare-metal support;
  documented debugger/sanitizer capability is not verified local workflow evidence.

## SRC-0003 — Kitware CMake/CTest documentation and release

- [CMake 4.4 presets manual](https://cmake.org/cmake/help/v4.4/manual/cmake-presets.7.html),
  Introduction and configure/build/test preset fields; publisher: Kitware.
- [Release 4.4.4](https://github.com/Kitware/CMake/releases/tag/v4.4.4), asset metadata.
- Eligibility/scope: build-system interfaces and pinned tooling, not compatibility
  semantics. Used to author independent project presets/scripts.
- Limitations: bootstrap tests only the configured native Windows flow.

## SRC-0004 — Ninja publisher

- [Ninja project](https://ninja-build.org/);
  [release 1.13.2](https://github.com/ninja-build/ninja/releases/tag/v1.13.2).
- Scope: CMake-generated build execution and release asset pinning.
- Eligibility: developer infrastructure; no source incorporated.

## SRC-0005 — UEFI Forum specification

- Publisher: UEFI Forum, UEFI Specification 2.11.
- [Official specification](https://uefi.org/specs/UEFI/2.11/), boot/runtime services
  lifecycle and Graphics Output Protocol; [official PDF](https://uefi.org/sites/default/files/resources/UEFI_Spec_Final_2.11.pdf).
- Scope: planned firmware handoff contract; no native implementation yet.
- Eligibility: public interface specification, linked rather than republished.
- Limits: selecting the specification is not tested firmware support; exact entry,
  memory-map, ExitBootServices and GOP sections must be reviewed before coding.

## SRC-0006 — GCC M680x0 compiler options

- Publisher: GNU GCC project, current compiler manual accessed 2026-10-07.
- [M680x0 Options](https://gcc.gnu.org/onlinedocs/gcc/M680x0-Options.html),
  `-m68000` and `-mshort` descriptions.
- Scope: candidate native 68000 compiler direction and ABI warning.
- Eligibility: tool documentation; no Apple implementation content.
- Limits: no Windows-hosted m68k package has been selected or executed. Option
  documentation does not prove complete freestanding runtime/link support.

## SRC-0007 — Apple archived QuickDraw documentation, candidate only

- Publisher: Apple, *Inside Macintosh: Imaging With QuickDraw*, archived web index.
- [Imaging With QuickDraw](https://developer.apple.com/library/archive/documentation/mac/QuickDraw/QuickDraw-2.html).
- Inspected: overview/index, for legitimate public-documentation discovery only.
- Eligibility: published prose/API documentation candidate; no sample code copied.
- Applicability: later Macintosh documentation; **not approved as a 1984 behavioral
  oracle**. Establish edition/date and exact sections before any derived spec.
- Limits: no QuickDraw behavioral implementation or historical fact is based on it.

## License/reference-policy sources

| ID | Publisher and exact source | Sections / purpose |
| --- | --- | --- |
| SRC-0008 | [Apache Software Foundation, Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0) | Patent grant/termination and distribution conditions; decision comparison only |
| SRC-0009 | [Open Source Initiative, BSD-2-Clause](https://opensource.org/license/bsd-2-clause) | Permissive terms and retained notices; decision comparison only |
| SRC-0010 | [Free Software Foundation, GPL v3](https://www.gnu.org/licenses/gpl-3.0.html) | Corresponding Source, distribution, patents and installation obligations; selected after owner approval |
| SRC-0011 | [Mozilla, MPL 2.0](https://www.mozilla.org/MPL/2.0/) | Covered files, distribution and patent terms; comparison only |
| SRC-0012 | [GitHub, Licensing a repository](https://docs.github.com/en/repositories/managing-your-repositorys-settings-and-features/customizing-your-repository/licensing-a-repository) | Public visibility, absent-license rights and GitHub view/fork exception |
| SRC-0015 | [Open Source Initiative, MIT](https://opensource.org/license/mit) | Permissive use and notice retention; comparison only |

The owner approved GPL-3.0-or-later on 2026-10-07. The standard GPL v3 text in
`LICENSE` was retrieved verbatim from the [FSF publisher](https://www.gnu.org/licenses/gpl-3.0.txt)
after that approval. Download SHA-256:
`3972dc9744f6499f0f9b2dbf76696f2ae7ad8af9b23dde66d6af86c9dfb36986`.
The project's version-3-or-later grant is stated explicitly in README/COPYRIGHT.md
and source SPDX notices; the standard license text itself is not modified. Other
listed license texts are alternatives, not adopted terms. No compatibility inference
follows from any license.

## SRC-0013 — GitHub Actions security guidance

- [Secure use reference](https://docs.github.com/en/actions/reference/security/secure-use).
- Scope: full-commit action pinning, least-privilege workflow design.
- Eligibility: development/security documentation; no source copied.

## SRC-0014 — Official checkout action release

- Publisher: GitHub / `actions/checkout`.
- [Pinned official revision](https://github.com/actions/checkout/tree/3d3c42e5aac5ba805825da76410c181273ba90b1),
  tag v7.0.1 verified through the official repository's tag reference API.
- Scope: CI source checkout; configured without credential persistence.
- Code is executed as an external pinned action, not vendored into this project.
- CI execution and hosted runner revisions are recorded separately; runner images
  can change despite tool-package pins.
