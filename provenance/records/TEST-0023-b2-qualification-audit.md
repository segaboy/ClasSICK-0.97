# TEST-0023 — Formal B2 qualification audit

2026-10-08. SRC-0063 owner continuation; reuses SPEC-0012/0013/0015, TEST-0022 and
SRC-0038/0043/0049. Read-only documentation/evidence review: no VM start, no
compiled-code change, no firmware read, no new collection beyond hashing
retained local files. Full findings: [the audit](../../docs/development/b2-qualification-audit.md).

Public state at audit start: `35d9d2da115dd17c073a0bf7c7f0f0f83f7d152a`; HQ
`de22a482b04a9408aa65a54ced1a58138a6881e2`; both owner checkouts clean on `main`.

Checks performed:
- Mapped each B2 criterion in `docs/architecture/boot.md` to retained evidence and
  classified it observed, gated, inferred or missing.
- Compared the sanitized TEST-0022 manifest: each of the seven runs has exactly one
  banner, `keyboard ready` and one terminal result. The no-input runs cold-02/03/05/06
  share own-transcript SHA-256
  `fdc9b969771dcc30…` and final-capture SHA-256 `027b6d2cbb63…`. Retained raw serial
  files cold-02 and cold-05 are byte-identical (SHA-256
  `341b3b24a448a1f3c6d2bf21d6bb8bbc02434b9fd89a4e38acb9616fa44586c7`).
- Read the retained own register snapshot (SHA-256
  `21cf473f62f6353903b872942a8a06f3cb17ed4c3a38ced9f7db6efbee3e916c`):
  RIP 0x10007617 lies inside the 53,248-byte O2 image linked at 0x10000000; RSP
  0x1CFFC9B8 has address bit 20 set. No memory or firmware was read.
- Reviewed Intel SDM Vol. 3A §11.7.13.4, SMSC PS/2-mode output port and D0
  command, Holtek self-test prose and UEFI 2.11 §4.3/4.3.1 system-table identity
  fields (locators in the source catalog).

Findings: criteria 1–5 are met by observation or reviewed gating; B2 remains
partial. Blocking: (1) criterion 6's firmware hash cannot exist without extraction,
which SRC-0059 forbids, so an owner clarification is needed; (2) SPEC-0012's
self-test A20/machine-state precondition is unmeasured, and the scene cannot
discriminate it because bit-20 masking would alias consistently. Outside the gate:
exception delivery, RAM trace export, physical UART, USB, other PCs.

Proposed, not implemented: a separate qualification probe image (output-port
reads around AA, owned bit-20 page-pair alias test, firmware vendor/revision
capture and owned-address printing) and its three-run evidence plan; owner
decisions D1/D2. A Macintosh M0.3 feasibility research milestone is proposed with
no code. Claude (Anthropic; session configured as `claude-opus-5-5`) performed
this audit; human provenance review pending. All edition flags and Mac goals
are unchanged.

Publication verification: [Windows CI 37867141307](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37867141307)
passed every step at `4f667725bb8c3ec6791dc85709653d216bc18b93` (source guard,
pinned setup, bootstrap builds and the full hosted/unloaded/media chain). It ran
no VM. This follow-up records results only.
