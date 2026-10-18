# copilot prepared-port implementation prompt - 2026-10-17

You work on Windows11/PowerShell for ClasSICK0.97 at public baseline25811071daa248780ad91221fa55aa0fd7342a20 plus the lead-frozen SPEC-0045/header/state hashes below. Codex alone writes/integrates/verifies repositories. Do not run commands,edit files,use web/MCP/delegation or execute tests. Read only the supplied public-input packet through EOF; no discovery outside it. HQ,credentials,untracked research originals,manuals,Apple/Microsoft code/ROM/media/binaries/disassembly,emulator source,manual sample code and UART bodies are excluded. All artifacts original GPL3 C11,freestanding native68000/x86,strict conversions,bounded caller storage,no allocator. Existing source/role/edition/physical/full128K/B2/human/boot gates persist; no target run/release.

Return tests/code-select-independent.c as a complete independent C11 artifact from frozen API/contracts and old original fixture helpers. Construct original CODE0 below512 and CODE1 program before PREPARE/LOAD. Cover no-effect admission, Get/Set/current/output/frame semantics, full bindings, one-use generation, cleanup/failure snapshots and trusted test schedules. New implementation/logs withheld; no oracle inference from code. JSON files/coverage/concise handoff; tests_executed empty.

Exact reviewed inputs and SHA256:

- `docs/specifications/SPEC-0045-integrated-port-selection.md`: `b4886c24e72bf4462b0dafd486401e1b39cdc5faa4c4f9d21ab2a3135ee408ad`
- `docs/specifications/SPEC-0044-prepared-port-ownership.md`: `5b44fc57880fac37cecad1bf58305170cc5f2a42434d44d48c923a5dbdaa7cbd`
- `docs/specifications/SPEC-0042-guest-port-selection.md`: `b938f63845b73749e7363c5dd74d9d822c9353da17aae9a5f16880532b9cd8e8`
- `personality/system1/code-select.h`: `9036683e158a2e9d765e8ebe5eb9ae83f531b9272174210251d2e292d7b93f5c`
- `personality/system1/code-image.h`: `6d95d79e9b33817ce2333c4d1323e92275feab884f53d3f10b439606ea91b50d`
- `personality/system1/code-run.h`: `1c4489624e1abccd9dc654aa51d962210c6a10ce37ccbd84ac1447037ff8238b`
- `personality/system1/quickdraw-select.h`: `cd72e096f6a9365ede45fd859df755fec021eb1986002d5ca8269a6c8f838624`
- `personality/system1/quickdraw-view.h`: `4a93567872049f5142a3980eba501c7a2e027c15de794dde11fbbd93cc3bbee7`
- `personality/system1/quickdraw-rects.h`: `db9facdb5ef3742c3c8935255e3f627c30d31b323a8223e470d9851a48c81e09`
- `core/memory/guest.h`: `5a89e549470bd0f3ffe50e7fb714342ff7dc6af0d49a0c154a7baebdf7e113ff`
- `core/cpu/m68k-guard.h`: `b518410b3dfed00ed66dd17c3e704f332ff9aa77806b4f5475f23e9fef3d58f9`
- `core/cpu/m68k-bus.h`: `a0a8fa1ca3075be2c3f6eecfe18f45ef0c3db3c338af00d5451edc293ad2ce0a`
- `core/services/traps.h`: `a3346547364a2237027dbe1d134b81129e80f9731f4c49c813fc9f68563fd782`
- `personality/system1/code-preflight.h`: `e422ee629c35489ec4c10ec6fb6b992e5a256ec5492701489f357a2c052ccac7`
- `apps/loaded-rom-app/probe.h`: `5800d08070c5e88b55a634bf9559c7faa6ab19dbcbde749c8650a4eba03f5f5c`
- `apps/loaded-rom-app/probe.c`: `e201a651189314d4e9613d859984a04b05d453c3c43b5404d5535260e7f76f8d`
- `tests/loaded-rom-app.c`: `99f4ac3bce3f469b3eff296956e602a5ac9046193ec023a67efc6559645e357a`

Frozen API identity manifest:

```json
{
  "personality/system1/code-select.h": "9036683e158a2e9d765e8ebe5eb9ae83f531b9272174210251d2e292d7b93f5c",
  "personality/system1/code-image.h": "6d95d79e9b33817ce2333c4d1323e92275feab884f53d3f10b439606ea91b50d",
  "docs/specifications/SPEC-0045-integrated-port-selection.md": "b4886c24e72bf4462b0dafd486401e1b39cdc5faa4c4f9d21ab2a3135ee408ad"
}
```

Complete frozen public-only packets and raw captures remain outside Git. CLI permission modes are not claimed to be a hard filesystem sandbox.
