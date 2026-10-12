# TEST-0029 - Original A-line execution verification

2026-10-12; SPEC-0021 / IMPL-0025. Verification in progress.

Independent arithmetic oracle exhausts all 2,048 MOVEQ words and preserved
registers/flags, all unsupported 16-bit words, all 4,096 A-line formats,
mode/address/stack/fetch/return failures and stable frame/table/buffer guards.
The original fixture checks six instructions/three service calls, return PC/A7,
SR, register preservation and installed address. Two gateways stay independent.
No-runtime imports, width/endian/sanitizer builds and own native image mutations
are separate from actual execution. Native CPU-entry/RTE observations must be
recorded later against the admitted image; compilation alone is insufficient.

Initial Windows run passed five Clang configurations; GCC then rejected misleading
indentation in the new test harness main. Correct its formatting and repeat the
full chain in a fresh directory. The initial build history is retained separately.
A local record-update command initially used the host default text encoding and
refused existing UTF-8 text; corrected without changing prior catalog content.

Corrected local chain passes 1,002 checks in nine configurations:
124/124/124/97/124/109/109/109/82. Twenty CPU object audits and original
Clang/GCC hosted twins pass, alongside the existing twenty service object audits,
Mac 24 hosted suites/four controls, native EFI profiles and media checks.
Eight new observer rejection controls pass. Final local log SHA-256:
`68c41b775595b52f212d074df1491c51517a1bb6f829571f203fba076cffdc10`.
Four original Mac twins pass eight corruptions, four resealed bridge/vector/
fixture mutations and eleven omitted-object controls each. Os/O0 ROM use
7,248/11,328 bytes, data/BSS 2,140; retain 8-KiB stack. Frame sums plus a
192-byte assembly/exception allowance are 1,676/1,460, not measured high-water.
Native trials are not yet credited at this implementation checkpoint.
