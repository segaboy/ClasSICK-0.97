# Paired native preview 1: approval proposal

Prepared2026-10-19. Nothing is tagged, uploaded or released. Current published versions stay ROM0.2.0-alpha.1 and PC unreleased.

Final package review only: unchanged exact CI/boot-observed native68000 ROM and native x86-64 EFI/raw disk; corrected component-specific offline notes, complete c631b824 source and GPL text, deterministic demo ZIPs/checksum inventories, independent audit and37 actual checks. Both proposed prereleases remain unpublished pending explicit owner approval and acceptance of PC human capture/GUI and other stated preview limits. No runtime edit, rebuild, target run, tag or release.

Both previews use the same portable C drawing workload compiled to their native architectures. On PC, the OS/workload is x86-64; the legacy68k interpreter is separate. Logical display512x342 remains the standing PC target; the tested framebuffer is1024x768 with the scene centered.

## Proposed publication

| Component | Version | Proposed annotated tag |
| --- | --- | --- |
| ClasSICK ROM | 0.3.0-alpha.1 | `rom-v0.3.0-alpha.1` |
| ClasSICK PC Preview | 0.1.0-alpha.1 | `pc-v0.1.0-alpha.1` |

Both tags would select the reviewed package-metadata commit identified in the approval request. That is distinct from compiled source `c631b824b2a264c6d81eb9832f17b7b5918aa9ff` and qualification evidence `87e95de40cfb0f06b6398a3cfa762e9c62b87375`. No image bytes are changed to add a version. A later publication receipt would record actual tag targets, GitHub assets/digests and version-pointer updates; these admission snapshots stay immutable.

## Exact upload allowlist

Four files per component, two separate experimental prereleases. Root review.json and APPROVAL-PENDING.txt are local approval aids and are never uploaded. The source archive is identical for both components. No emulator, compiler or third-party firmware is bundled.

### ClasSICK ROM

| File | Bytes | SHA-256 |
| --- | ---: | --- |
| `SHA256SUMS.txt` | 311 | `cd3d096f5b2f38aafddc86bb78c7b7ef6b2bdc468d143d212bf2f693c726c901` |
| `classick-rom-0.3.0-alpha.1-demo.zip` | 22530 | `e7b1d6a867a9335b952457e0dc24081b03ce6ec85d72251135754f5231dbbf4c` |
| `classick-rom-0.3.0-alpha.1-mac128k.rom` | 65536 | `2b600319d334145e582adc3db0e0fcf495d5f44309488c19e652dfb378391780` |
| `classick-rom-0.3.0-alpha.1-source.zip` | 2100291 | `b8e1537750316012a3bad8afb497d3f4043bb0cf0baf1ea8bb6eebcd92d28865` |

### ClasSICK PC Preview

| File | Bytes | SHA-256 |
| --- | ---: | --- |
| `SHA256SUMS.txt` | 309 | `b17962367afba6e9c8f2a98c1f9102b12de8abf75627fff35a3e51135427b451` |
| `classick-pc-0.1.0-alpha.1-source.zip` | 2100291 | `b8e1537750316012a3bad8afb497d3f4043bb0cf0baf1ea8bb6eebcd92d28865` |
| `classick-pc-0.1.0-alpha.1-x86_64.efi` | 159232 | `23e021a4ab57cfd57559777e05d57e41f2b2217b4975b0543d9834414989d8f0` |
| `classick-pc-0.1.0-alpha.1-x86_64.zip` | 255134 | `4c6a95de87c56600aad34e78e1c169ad4ffd17b13f34d7450d43bd683c02ea8c` |

## Verification and acceptance

37 actual package checks passed, including an independently authored verifier, two identical complete package builds and altered-package/refusal controls. Source ZIP is byte-identical to a fresh Git archive of the compiled revision. Both demo ZIPs include their own platform guide, notes, original payloads, GPL COPYING, pinned source/evidence/CI identity and internal checksums. Exact outer checksums cover the three other uploads; checksum files never checksum themselves.

Existing CI3150 tests/11 profiles and the recorded own-image trials are reused; packaging did not rebuild or boot either target. Mac human2 clicks passed. PC38 mouse+5 Space injected events passed through guest185; eight static and two full Mac pixel comparisons passed. The current PC preview keeps running after60 seconds and ignores Escape; close/power off via the host. Old diagnostics are unchanged.

Explicit owner acceptance is still needed for these experimental preview limits:

- PC human pointer capture/focus after intermittent-click report
- One VirtualBox GUI preboot CPU-initialization failure;headless fallback passed
- Snow persistent preference isolation
- Physical Macintosh/PC,full128K,B2,historical/edition,System1-ROM boot directions,native external apps,Linux complete edition

Approval would authorize only the two annotated component tags at the selected metadata commit, two GitHub experimental prereleases with exactly the listed files, digest/download verification and ordinary public/HQ publication receipts/version-pointer updates. It would not authorize new builds, target/reference/hardware runs or compatibility-gate closure.

[Review](../docs/development/paired-release-package-review.md), [machine receipt](../docs/development/paired-release-package-review.json), [ROM notes](rom/0.3.0-alpha.1.md), [Mac setup](rom/TRYING-0.3.0-alpha.1.md), [PC notes](pc/0.1.0-alpha.1.md), [PC setup](pc/TRYING-0.1.0-alpha.1.md), [screenshots](../docs/evidence/screenshots/README.md).
