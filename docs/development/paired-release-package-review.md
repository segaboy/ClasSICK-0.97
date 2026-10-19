# Paired release final package review -2026-10-19

SRC-0301..0304 / ADR-0057 / IMPL-0054 / TEST-0060.

Final package review only: unchanged exact CI/boot-observed native68000 ROM and native x86-64 EFI/raw disk; corrected component-specific offline notes, complete c631b824 source and GPL text, deterministic demo ZIPs/checksum inventories, independent audit and37 actual checks. Both proposed prereleases remain unpublished pending explicit owner approval and acceptance of PC human capture/GUI and other stated preview limits. No runtime edit, rebuild, target run, tag or release.

The exact [approval proposal](../../releases/paired-native-preview-1-approval.md) contains all eight upload identities, notes/setup links and acceptance items. [Machine evidence](paired-release-package-review.json) records all37 checks, source/qualification pins, input hashes and integrated helper/doc hashes. Frozen images remain Mac2b600319...,EFI23e021a4...,diskfcfc9002...; source c631b824,qualification87e95de4 and CI39301375020 stay distinct.

## Work division and review corrections

Claude Opus5.5 low delivered the builder and four component documents in one275.66s call; zero tools/writes/tests.497 artifact lines exceeded430 cap. Copilot Sonnet5.5 low independently delivered a283-line auditor in48.97s from the contract,without builder or logs; one prompt read,zero writes/tests,240-line cap exceeded. Codex integrated sound artifacts,corrected the points below and performed all tests; no duplicate provider review/requalification or Grok/Argon call.

- Removed PC-only guide from Mac package and replaced broken relative package links with offline text or pinned HTTPS links.
- Corrected source ZIP directory admission; legitimate Git directory entries allowed only in source ZIP, never demo ZIP.
- Removed false DEFLATE-level flag inference; builder explicitly uses level9 and two complete builds match byte-for-byte.
- Added Windows junction/ancestor alias refusal and concise malformed-input failure handling.
- Added exact Mac output path, full PC source/tool/media build steps, installed VirtualBox7.2.16 option check, all8 NIC/VRDE isolation and explicit unqualified human test instructions.
- Download notes are release-shaped experimental notes, while pending/unpublished markers stay outside the upload allowlist.
- Lead negative-control harness first used the wrong docs root in its overlap case; corrected run37/37 passed. No product defect was hidden as a pass.

## Actual verification

Two new full kit builds match all10 output files byte-for-byte. The independent audit passes.26 adversarial archive/manifest/allowlist cases reject changes even when outer hashes/review metadata are resealed; preflight cases reject missing/wrong inputs and invalid docs before output creation.37 total executed checks are enumerated in the receipt; source-directory and symlink controls exercise source ZIP name/type admission separately. This is packaging verification,not a target/human/historical test.

The one initial harness error passed the public checkout instead of the fixture docs directory to the overlap control; it built a valid extra private kit and failed the harness assertion. The corrected control and full rerun pass; earlier directories are preserved. No target binary was executed.

## Publication boundary

Package review passed and the pair is ready for an owner decision. Release-ready remains false until explicit acceptance/publication approval. Root pending markers are outside upload directories; no DRAFT suffix or false unpublished-status statement is frozen into the download notes. Current published pointers,earlier candidates/assets/tags and all compatibility gates remain unchanged. No additional screenshot or target run was needed.24 research originals,the held brief,UART bodies,old DRAFT bundles and frozen wiki are preserved.

Provider-reported Claude list cost0.7337544 USD and Copilot1 premium request are task observations,not charges or cross-provider quota savings. Remaining account allowances for Claude,Copilot,Grok and Antigravity are unavailable/not exposed. Full worker transcripts/control fixtures stay outside Git.
