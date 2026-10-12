# TEST-0027 - Native PC controlled-fault verification

2026-10-12; SPEC-0019 / SPEC-0008 v2 / IMPL-0023 / ADR-0022 / SRC-0085.
Initial status: verification in progress, native trial not yet run. Subsequent
results belong in the dated fault-checkpoint report; do not rewrite history.

Three host classifier profile fixtures and sixteen invalid-run controls cover
wrong vector/error/saved RIP/IST/interrupted stack/state pointer/state/marker,
selectors/IF/live RIP and missing/failed own completion. Synthetic addresses and
CPU frame expectations are independently chosen from the documented interface.
They are safety/classification checks, not native execution evidence.

Fresh full Windows chain retains the prior hosted/device/media tests unchanged.
Normal/UD/GP O0/O2 separate twins are audited for no-runtime links, exact own
assembly, all 256 vectors, capture/publication/exports/terminal targets and probe
profile. Each profile rejects capture/export mutations and ten omitted objects;
injected profiles also reject altered opcode and misclassification as normal.
Public CI builds/tests only. Native tests require owner-approved new isolation,
own success at second 60, three starts per fault profile and one normal control.
No firmware code/memory read, synthetic keyboard input or UART source change.

Local build follow-up: the full chain passes 876 CTest checks in nine Windows
configurations (110/110/110/83/110/95/95/95/68), including sanitizers. Normal,
UD and GP each produce four matching-by-optimization images; image corruption
rejections are 25/26/26, omitted-object rejections ten each, and each injection
rejects normal-profile classification. Nine normal writer outputs match; thirty
media corruption controls and three writer refusals pass. All three media pass
the independent checker. Native trial still pending at this dated build record.
Full retained local verification log SHA-256:
`90b9d0d5a294f8ba39b96e078e20ae5afeac45e2079446223864e751223e3450`.

Trial follow-up: [dated report](../../docs/development/fault-checkpoint.md)
passes three UD/vector6/error0 and three GP/vector13/error0x28 cold starts,
exact saved instruction, published record and owned IST1 placement, stable
selected registers, and one normal control. Every run completes its own second
60/result0/kbd1 before observation. CI 38250103771 passes both jobs at
`2f64689521fe57866b7a70572f2f3bb8a7bbb008`; six EFI fingerprints match local.
Final powered-off/inventory/unchanged-disk checks pass. The host batch logging
redirection error after all observers return is retained; no guest/test failure
or repeated test is inferred from its outer process status. All limitations persist.
