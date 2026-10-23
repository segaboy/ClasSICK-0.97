# TEST-0080 - Foreground coordinator verification

2026-10-23;SPEC-0066/IMPL-0068,SRC-0390/0391/0393.
[Receipt](../../docs/development/window-foreground-evidence.json).

Actual338 Clang64 Debug CTests,3 GCC64 Release,3 ASan/UBSan,total344 profiles.
276 new mock-owner assertions and79 real-owner assertions per profile;not distinct
counts multiplied into scenario claims. Initial API-only and adapted prior real-
owner oracles predate reading primary;later metadata/cancel-time/begin-fault cases
added and final independence qualified. Raw-byte snapshots avoid padding copies.
Three compiled mutants(owner under exclusion,completion erases loss,stop not
terminal)reject;valid control passes.12 width/endian/EFI-ABI audits/import allowlist,
no mutable globals,matching delayed twins,per-function512 bound pass. Actual68000
closure/size/frame evidence and normal source-pinned CI are pending.

Cases verified in software:baseline/no duplicatecancel,PRIMING staging,one ordered
sample,loss during delivery/baseline/cancel,helper mismatch beforeclaim,first fault
overloss,retained prefix,terminal repeated stop,no-wrap/no-effect restart and
failed restart closedepoch snapshot;real pointer/window clamp/drag/release.
No actualCPU mask/order/ISR/quiescence/latency,MMIO,hidden edge detection,whole128K
budget or target run. Original IC and M1-M7 hardware parts remain partial/open.
