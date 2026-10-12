# Shared trap-address service checkpoint - 2026-10-12

SPEC-0020 / ADR-0023 / IMPL-0024 / TEST-0028; SRC-0088-0091.

**The first shared service slice passes on both architectures.** The same
original portable C implements checked GetTrapAddress/SetTrapAddress, a complete
trap-word decoder, direct intrinsic calls and explicit unsupported/needs-execution
results. Three x86-64 cold starts and two 68000 ROM cold starts complete its
ten-check probe with zero service errors and signature `53564331`.

This is the `classick-trap-address-v1` engineering profile adopting selected
published 1985 interfaces. It does **not** execute A-line opcodes or legacy
applications, provide general callbacks/Toolbox stack handling, or finalize the
January 1984 SPEC-0017 draft. Symbolic service addresses are scalar identities,
not executable native pointers or guest-readable stubs.

[Sanitized evidence](shared-services-evidence.json) retains own serial/register
observations, forty diagnostic RAM bytes per Mac run, image identities and build
metrics. Private paths, project UUIDs, inventory, pre-banner serial bytes, manual
PDFs, screenshots and emulator binaries stay outside Git. No Apple ROM/code or
assets, firmware memory, emulator source, disassembly or full guest dumps used.

## Build and independent checks

Compiled source and observer: `02e2f5a6daf29a1fd4dc7a61ea460d371fa493ee`.
[CI 38257185223](https://github.com/segaboy/ClasSICK-0.97/actions/runs/38257185223)
passes both Windows and 68000 jobs. All six O0/O2 EFI fingerprints match local;
the downloaded own Os ROM matches the admitted image, and both ROM profiles and
all source-input fingerprints agree with local admission. CI does not run these
VM/GUI observations.

The full Windows chain passes 939 CTest checks in nine configurations:
117/117/117/90/117/102/102/102/75. New suites exhaust all 65,536 words, all 512
entries and invalid 16-bit numbers, boundaries/failure stability, A0/SR/register
preservation, direct calls and restoring a patched service's original entry.
Clang/GCC hosted twins and actual i686 pass; x64 sanitizers retain distinct scope.
Twenty additional O0/O2 width and ARM64 LE/BE object audits enforce original
imports/no mutable global core state. Five service-observer corruptions reject.

Normal/UD/GP EFI profiles each produce four images and deterministic twins per
optimization. Corruption rejects are 26/27/27, omitted-object rejects twelve
each; both injections refuse normal-profile classification. Exact audits retain
all exception stubs/record exports and check the new service-result helper.
Nine normal media writers match; thirty media corruptions and three writer
refusals reject. All three media pass the independent checker; only normal
service media is executed in this milestone. Earlier fault trials remain dated
evidence for their original image.

Four 68000 builds/twins reject seven corruptions and eight omitted objects each.
No target runtime archive or startup library enters either native link; the
existing independently authored 68000 helper module remains. Mac hosted checks
retain 24 suites and four expected zero-divisor controls.

| Admitted artifact | Identity |
| --- | --- |
| Normal O2 EFI, 46,080 bytes, loaded extent 61,440 | `203f26ecc501c8fd931312df07aefbcab8522371467a13f41f103321638f33b1` |
| Own raw GPT/FAT32 disk, 67,108,864 bytes | `bd1dd8c4de1769a4dae7e9744bf6a12ceb883ec1067e6de535455186a9f2f5b2` |
| Own Os ROM, 65,536 bytes, seal `6ad16297` | `838a4d6213e3b9656dce43e3b0b6e626766f2c1068f3aa10f01ad5655f6604a0` |
| Own O0 ROM, 65,536 bytes, seal `ac8e9df7` | `6f6ab14ef4be10586e99914d27c6068f50de12928ad7fa703f4e09b4fcc20451` |

Os/O0 use 5,260/7,896 ROM bytes. Both use 2,116 bytes of data/BSS below the
new enforced 0x2000 upper bound, retaining the 8-KiB stack and hardware/screen
ranges. Listed frame sums plus 64-byte margin are 1,164/960 bytes, conservative
for the reviewed nonrecursive graph, **not measured stack high-water marks**.
PC service state/result occupies a named 4-KiB slice in the existing owned loader
bundle tail; no inherited/firmware storage or extra writable image globals.

## Native observations

One fresh VirtualBox 7.2.16r174877 machine uses the existing reviewed isolation:
one CPU, EFI64/Secure Boot off, owned fixed disk only, no network/audio/USB/shares/
clipboard/drag/drop/remote display. Exact executable hash matches SRC-0059.
RAW-to-fixed-VDI-to-RAW reproduces the original disk; final VDI is unchanged and
the machine is powered off. Inventory adds only that identity; earlier VMs/media
are retained. No reset/resume/saved-state run is counted.

Each PC cold start reaches existing successful keyboard/qualification/second-60
output before any selected-register query. Two delayed samples agree: live
RIP/RSP in our terminal loop/ordinary stack, RCX at owned service-result storage,
R9=10, R10=0, R11=`53564331`, R14=`43535331`. These values expose the completed
portable probe, with no UART source changes, guest-memory reads or CR2 queries.
Arrival timing includes host/serial scheduling; no calibrated-clock claim.

Two fresh Snow profiles use exact v1.5.0 binary hash
`32cf5285582c0f509e4f1ac38d86bfb90440bae9da43b7823bfb0841dea588e6`, Early128K,
RelativeHw mouse, own ROM only, audio off, no disks/PRAM/extension/bridge/shared
folder, normal speed. Both reach the original scene and own diagnostic stage 4:
3,610 counted flags, zero presses, one existing TRAP return, expected seal, zero
fault, ten service checks, zero service status and matching signature. No new
operator or generated guest-input evidence is credited.

Computer Use inspected the published RAM viewer. Its initial default shows our
own vectors; navigating to B00 also displays adjacent own directory bytes. Only
the forty-byte diagnostic record B00-B27 is retained. No ROM/code/disassembly
viewer opened. Both windows close through normal Alt-F4, confirmed by window
absence and loop-exit logs. Process exit codes were not retained; do not claim 0.

## Failure history and remaining work

An extra Clang i686 O0 audit caught compiler-generated memcpy from aggregate
copy/initialization. Explicit scalar stores remove it; corrected fresh full/native
verification passes. Provisional images are retained and never admitted. An
existing-directory object retry refused as required. Snow accessibility clicks
did not open menus; fresh screenshot-based alternate-button inputs worked. No
uncertain implementation or new emulator capability was used to recover.

Formal B2 criterion 6's firmware image fingerprint stays open. All complete OS
editions/parity, physical Mac/PC, exact Mini vMac qualification and full 128K
budgets remain open. Human provenance review is pending; no legal/independently
staffed clean-room certification. Next is the original CPU/address-space adapter
that can execute A-line calls to these services, followed by individually scoped
manager semantics. Keep later-manual adoption distinct from January 1984 proof.
