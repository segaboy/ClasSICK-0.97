# TEST-0094 - Strict register decoder and native audit

SPEC-0072/IMPL-0081/SRC-0477. Seven actual host test groups/10 CLI cases,focused CTest and three faulty decoder controls pass/detect.17 resealed native corruption sites wired but UNEXECUTED until source CI. No target/input qualification. [Receipt](../../docs/development/mac-register-checkpoint.json).

First native attempt,d100c88/CI40326299729:assembler rejects the short branch to the exported global spin symbol (out-of-range byte-field value). Lead uses a local numeric self-branch at the same exported location;the native audit still requires exact60fe. No target ran;fixed native proof remains pending.

SRC-0478:corrected c40cd29 fullCI40327277294/native72-byte proof and51 lead resealed controls pass;default baselines unchanged,zero extra RAM. Actual automatic500/5000 snapshots read at exact own spin/IPL7/fault0. Second host event lacks verified viewport targeting;no motion qualification. [Actual results](../../docs/development/mac-register-checkpoint.md).
