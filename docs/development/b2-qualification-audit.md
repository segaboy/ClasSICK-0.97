# Formal B2 qualification audit — 2026-10-08

**Result: B2 stays partial.** Criteria 1–5 of [the B2 definition](../architecture/boot.md)
are met by observation or by reviewed gating. Two items block formal acceptance:
the firmware *hash* named in criterion 6, which cannot exist under the SRC-0059
no-extraction boundary, and SPEC-0012's own qualification precondition that
controller self-test preserves A20 and required machine state, which has not been
measured. SRC-0063 / TEST-0023. This is a read-only audit of retained evidence
and eligible sources: no VM was started, no code changed and no firmware was read.

The audit covers [SPEC-0015's trial](vm-trial-evidence.md) of payload
`8e25d6947677b538d84bc17f98ff44d056dcd3d7769d1389676072286ad831f2` in raw image
`370b7d4e7fc4200b77c367e09afa0c4944f534c2623d7f5f68db159c1da46078` on
VirtualBox 7.2.16r174877 (public record at `35d9d2da115dd17c073a0bf7c7f0f0f83f7d152a`).

## Criterion map

"Observed" means a retained own output or owner observation. "Gated" means an own
output that the reviewed code emits only after the stated condition holds; the
gate logic is hosted-tested and audited unloaded. "Missing" blocks the gate.

| B2 criterion | Evidence | Class |
| --- | --- | --- |
| 1. Loads without Apple code, a Macintosh environment or a hosted OS | Own UART banner from our image in every run; unloaded audits show zero imports/runtime libraries; the SPEC-0014 checker shows the disk holds only our payload | Observed + audited |
| 2. Validates map/framebuffer, `ExitBootServices` succeeds, transfer to owned entry/stack | The SPEC-0013 observer emits nothing until successful exit, the owned READY marker and validated map/ACPI/8042/framebuffer/arena/timer inputs. A post-scene register query placed RIP at 0x10007617, inside the 53,248-byte O2 image linked at 0x10000000 | Gated + observed RIP. Owned-stack placement is inferred from the reviewed transition only; no exported address ties RSP to the owned stack |
| 3. Own arena, CPU exception state initialized, original scene via core surfaces with post-handoff writes | READY is written only after our descriptor install (SPEC-0008); the presenter rechecks exit and READY; the arena gate precedes drawing; original-scene captures and owner observation | Gated + observed. Initialization is required here; delivery is not |
| 4. Visible progress for at least 60 s on an owned timer, no boot-service callbacks | Six uninterrupted runs reach guest `sec=0000003C`; cold-05/06 ready-to-result arrivals 59.972/59.971 host s with ~59.86–60.12 s brackets; final frame still shown at the 90/120 s captures; zero imports by audit | Observed + audited |
| 5. Keyboard event through our driver after handoff, scene change, bounded diagnostic trace | cold-04: two owner-operated Space presses, counter 2, visible changes, owner confirmation; bounded SPEC-0013 serial records in every run | Observed. **Missing:** SPEC-0012's driver precondition that self-test preserves A20/required machine state |
| 6. Evidence tied to source, toolchain, image, firmware version/hash, configuration, transcript, screenshot; repeatable from power-on | Source/toolchain/media hashes; 46 checked settings; serial transcripts; hashed local original-scene captures. Firmware version 7.2.16r174877. Repeatability: no-input cold-02/03/05/06 have byte-identical own transcripts (SHA-256 `fdc9b969…`; raw serial cold-02 and cold-05 identical too) and identical final captures (`027b6d2c…`), including the GUI-mode cold-03 | Observed. **Missing:** firmware image hash |

## Self-test, A20 and machine state

Intel SDM Vol. 3A §11.7.13.4 (printed 11-33 / PDF 341) says asserting A20M#
masks physical-address bit 20 for all external bus memory accesses, that modern
operating systems do not use it and that newer Intel 64 processors may lack it.
The text gives no IA-32e-mode exemption. So running in 64-bit mode does not by
itself make the precondition moot, and a running scene does not prove it: our
code's stack is at a bit-20-set address (RSP 0x1CFFC9B8, RIP 0x10007617 with bit
20 clear), but a masked A20 would alias every access consistently, and after
`ExitBootServices` nothing else uses the aliased memory. The scene would look
the same either way.

SMSC KBD43W13 (SRC-0049) defines the PS/2-mode output port with bit 0 System
Reset and bit 1 Gate A20 (PDF 11) and command D0 Read Output Port (PDF 12–13).
Neither SMSC nor Holtek HT6542B says that self-test (AA) changes the output port.
Holtek PDF 5 does say a successful self-test registers 55, sets the system flag
and enables the keyboard interface; SPEC-0012 already re-disables, drains and
verifies configuration afterwards, and every run reached `keyboard ready`.

What is already observed: AA returned 55 and configuration readback passed in all
seven runs (otherwise no `keyboard ready`); no run reset or restarted. What must
still be measured: (a) the controller-reported output port bits 0–1 before and
after AA, and (b) the physical effect, whether owned addresses that differ only
in bit 20 alias after self-test. Legacy real-mode concerns (wraparound, BIOS data
area, IRQ1 routing with interrupts masked) do not apply to this profile.

## Firmware identity

Within current boundaries the available identity facts are:
- product version/revision 7.2.16r174877;
- VBoxManage and the firmware-embedding container hashes (SRC-0035), the latter
  explicitly not a firmware image hash;
- the publisher signature on that installed container, readable with ordinary
  read-only signature verification but not yet recorded;
- the firmware's own runtime identity: UEFI 2.11 §4.3.1 (PDF 174–175, within
  SRC-0038) defines `FirmwareVendor` and `FirmwareRevision` in the system table,
  still valid after exit. These are not yet captured.

Oracle's download checksums cover installers, not the embedded image. None of
these is a hash of the firmware image, and producing one would need extraction,
which SRC-0059 forbids. A physical PC's firmware is the same: its hash means
reading flash. As worded, criterion 6 cannot be met on any black-box platform.
Closing it needs an owner-approved clarification; relabeling the container hash
would not be honest.

## Outside this gate

Actual exception/fault delivery (B2 requires initialized state, which is gated).
Full RAM trace inspection (the bounded serial records satisfy "records a bounded
diagnostic trace"; offsets 128/256 remain unexported). Physical UART clock, baud
and electrical behavior; USB input; other PCs; VirtualBox device-model
conformance beyond what we observed. All three OS editions, parity, Macintosh
128K feasibility, replacement startup and real Mac/mini vMac boots.

## Proposed next qualification (draft only, not implemented)

A separate **qualification probe image** keeps the B2 candidate image unchanged.
It reuses the same loader, transition and SPEC-0012 startup and adds four bounded
observations. Each goes to UART and to a new 64-byte record at trace offset 1024
(trace bytes 896–16,383 are unused).

1. **Output port.** One D0 command and data read immediately before AA, and one
   after the post-self-test drain. Pass when bits 0–1 match; on a mismatch, stop
   with a distinct result before enabling the keyboard. Never write D1.
2. **Bit-20 alias.** Before exit, the loader asks for the page at an owned bundle
   page's address XOR 1 MiB, as one extra owned span, trying a bounded number of
   bundle pages. After self-test, write distinct 64-bit patterns to the pair and
   read both back. Pass when neither aliases. If no pair could be reserved, report
   UNAVAILABLE, which counts as incomplete.
3. **Firmware self-report.** Before exit, copy `FirmwareRevision` and at most 32
   UCS-2 units of `FirmwareVendor` (non-ASCII replaced) into the handoff and print
   them in the banner.
4. **Owned addresses.** Print image base, bundle base and stack top, so a
   post-ready register query can show RSP inside the owned stack.

Evidence plan: the full pinned verification chain with updated unloaded audits
and new hashes, then a newly identified trial on the existing admitted project VM
with three headless cold starts. No operator session is needed. Its D0 reads are
read-only and its startup sequence is otherwise identical, so the measured
controller and chipset behavior qualifies this machine for the B2 candidate, whose
existing operator evidence stands. Pass requires items 1 and 2 in all three runs.
Implementing this needs the owner's approval: SRC-0060 authorized trials of the
existing image, not new guest code.

## Owner decisions requested

- **D1:** approve implementing the probe image and running it in a new trial.
- **D2:** criterion 6 for black-box platforms. Either accept product
  version/revision plus the container's SHA-256 and verified publisher signature
  plus the runtime `FirmwareVendor`/`FirmwareRevision`, recording explicitly that
  no image hash exists, or keep the wording, which leaves B2 partial on VirtualBox.

## Macintosh path

The next bounded research milestone proposed for M0.3 is a feasibility study with
no code. It would:
- review eligible primary sources for the 68000 ISA and Macintosh 128K hardware
  (memory map, ROM overlay, reset vectors, video buffer, VIA);
- review mini vMac's published user documentation for its ROM-image and 128K
  configuration requirements (documentation only, never its source);
- plan a pinned m68k toolchain (SRC-0006 options);
- produce a measured RAM/ROM budget for the current portable core and scene
  against 64 KiB ROM and 128 KiB RAM.

It produces source records and a report: no Apple ROM, no emulator run.
