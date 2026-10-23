# Lead 286 feasibility source review

2026-10-23;SRC-0375..0382 / ADR-0075. [Reviewed proposal](../development/pc286-feasibility.md),
[machine receipt](../development/pc286-feasibility.json). Source/design intake
only;no ABI,implementation,tool installation,fixture,feature test or target run.
The existing Mac/x64 paired release remains immediate;286 is a separate future
investigation lane,not a change to the single native64-bit PC distribution goal.

## Eligible selected sources

- **SRC-0376:** Intel SDM Vol1,253665-021,September2006,cover and2.1.1..2.1.3
  (printed2-1..2-2,PDF31..32);Vol3A,253668-021,October2006,cover and17-38..39
  (PDF630..631). Published architecture/compatibility prose only:16-bit286,
  24-bit protected-mode base,64KiB segment limit,386 introduces32-bit registers.
  Read through publisher web PDF extraction;local downloads unavailable and
  no local hashes or visual-figure checks claimed. No mode-switch recipe admitted.
- **SRC-0377:** [GCC16.1 x86 options](https://gcc.gnu.org/onlinedocs/gcc-16.1.0/gcc/x86-Options.html),
  -m16/-m32/.code16gcc. This describes32-bit code in a16-bit execution mode,
  not a286 compiler. Combined with Intel register history,this rules out using
  that option as proof of286 compatibility. No compiler implementation used.
- **SRC-0378:** [Open Watcom User's Guide](https://openwatcom.org/ftp/manuals/current/cguide.pdf),
  Version1.8,copyright2002-2008,prefaceiii(C90/partialC99),printed5/PDF17
  (WCC versus WCC386,8086 default and286/386 generation options),113..114/PDF125..126
  (near/far/huge models and normalization cost),120/PDF132(short/int2B,long4B).
  Visually checked model table114. Dated guide,even in a /current/ directory,
  does not qualify currentv2,Windows11 host,C11,library/runtime or licence terms.
  An initially retrieved QNX guide is excluded from this adopted source set.
- **SRC-0379:** IBM *Options and Adapters*,Hardware Reference Library:
  EGA description printed1/PDF5,datedAugust2,1984(640x350),visually checked;
  CGA description printed1..2/PDF5..6(640x200 or320x200),visual page2;
  PC/AT High Capacity Diskette Drive description printed1/PDF5,August31,1984
  (1.2MB5.25-inch media and lower-density read capability). CGA section date
  not established from its checked pages. Original manufacturer scans,not
  archive commentary,are evidence. No BIOS listing,ROM font/pixels or driver
  algorithm adopted;drive characteristics are not a full BIOS boot contract.
- **SRC-0380:** [Hercules Graphics Card Owner's Manual](https://www.retrocmp.de/hardware/hercules-card/GB101_Owners-Manual_text.pdf),
  Edition2.0,copyright1984,p1/PDF3(two720x348 graphics pages). Visually checked
  copyright/edition and intro. No HBASIC/programming routines or disk fetched.

Manuals/page images stay outside Git and worker inputs. Only selected prose,
type facts and tables are adopted;sample code/listings and unrelated search
excerpts are excluded. Intel/GCC web locators are recorded in the receipt.
Failed Intel/brochure downloads and unreviewed VGA brochure search text are not
local identity or hardware qualification evidence. Exact VGA286 driver/profile,
BIOS/media ABI,serial protocol/hardware and CPU interrupt admission remain open.
These sources do not establish January1984 Mac behavior or measured286 coverage.

|Private admitted copy|Bytes|PDF pages|SHA256|
|---|---|---|---|
|openwatcom-general.pdf|849005|557|`7db3a7563e3436302bf76384bbd2f54339a991b106e69651eb07ef83249e2517`|
|ibm-ega.pdf|4242750|174|`d2411a689d762e9c6915fd3b15a92f760beada41a4cd1b0020fd29814db151bd`|
|ibm-cga.pdf|565691|42|`658276c95354923bf65e04044f82af7d260c1de22d867d4c9d5b9c35f339916a`|
|hercules-gb101.pdf|6092414|15|`9bbbe148d3c09a02064def6026d26802570d0c6abc8e74ec3a3ffff07f181f84`|
|ibm-at-drive.pdf|244603|12|`fb7b8b1a1bb82e49c1caf874ec9061e5817ce1d464c199c6ed8f67e0c85f2612`|

## Original code spot-check and dispositions

At baseline4f231b5,guest.c:12/25 use32-bit flags/access with ~7u. Under a
16-bit unsigned ABI that mask is0xFFF8,then0x0000FFF8 after widening:READ plus
bit16(0x10001) can evade the high-bit rejection. This is a reasoned portability
finding,not a measured C16 failure or a bug on current32-bit-int targets.
No patch here. The checked guest view is contiguous;bank-splitting alone cannot
repair long accesses/copies across banks. Host segmented-pointer representation
and retained view/transaction rules need a genuine backend/ABI contract.

One Claude Opus5.5 low primary53.45s,29739B431line frozen public/original
packet returns220lines/2158words:word cap2200 met,line cap180 missed. Zero
tools/edits/tests,retry,second worker or requalification. Lead adopts study,
corrects odd-byte alignment,32-bit instruction-cost overstatement and ordinary
Windows11-versus16-bit execution;adds native standalone/DOS independence and
logical-guest-versus-physical-host mapping. Coverage/phase labels avoid existing
B2/M1-M7 names. Standing512x342 policy governs CGA;no change/permission sought.
Individual dispositions in receipt. [Actual prompt](../development/tasks/pc286-study-claude.md).

Recommend conditional investigation go:P286-1 toolchain/compile-only portability
proof first,then measured map/backend contract. Neither a working286 port nor
real-app performance is proved. All8 P286-V proposed checks are UNEXECUTED.
No tool/runtime licence conclusion,installation,source-code change,fixture,
build,ROM/disk/media import,emulator/VM/reference/hardware run or release.
Current runtime/old evidence remain intact. Preserve24 originals,held brief,
UART bodies,old rows/assets/tags/retired wiki and every existing gate.
Reported$0.2222208 list/session cost is not balance or Codex savings;remaining
allowances unverified. Other workers idle. Raw captures/private sources:
C:/ClasSICK/pc286-feasibility-20261023-a. Record guards are not feature tests.
