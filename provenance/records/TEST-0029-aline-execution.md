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

Native follow-up: three fresh native PC starts pass the same six-instruction/
three-call/eight-check interpreter probe, exposing 8/0/414c4e31/6/3 after own
successful second60 output; two register samples agree each run. Two fresh Snow
128K own-ROM starts pass both portable probe and three actual vector10/RTE calls,
preserved register/SR checks, stage4/ticks3610/one TRAP/expected seal/zero fault.
[Detailed evidence](../../docs/development/aline-execution-evidence.md) retains
CPU-path distinctions, 64 own diagnostic bytes and remaining unsupported scope.
CI38284131256 passes both jobs at `f29113b13775102f294d76e9c3deb1691228b1c8`;
six EFI/two ROM profiles and every ROM input fingerprint match local. VM off,
VDI unchanged, both Snow windows absent/loop-exit logs; no Snow exit-code claim.
