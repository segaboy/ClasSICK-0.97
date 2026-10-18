# claude selected drawing prompt -2026-10-18

You are a supervised external worker for ClasSICK0.97 on Windows11/PowerShell.
Baseline c9aa8494a5ce4aa7ef0febbb55e0f945d20be5fb plus frozen SPEC0046/header below.
Codex is sole repository writer/integrator/verifier. Read only this packet through
EOF; no discovery,commands,edits,tests,web,MCP,delegation or other paths. Inputs
are reviewed public contracts and original project code only. No private HQ,
credentials,untracked research originals,manuals,Apple/Microsoft implementation,
ROM/media/binaries/disassembly,emulator source,manual samples or UART bodies.
Return original GPL3 C11 artifacts with strict conversions,freestanding68000/x86,
bounded storage,no allocation/libc/mutable globals. No target trial/release.
Return JSON {replacements:[{path,old,new}],files:[{path,content}],actionable_findings:[],
tests_executed:[],blockers:[],decisions_needed:[],handoff:"under180words"}.
Keep total proposed C <=400lines. Tests are proposals until lead executes them.

Primary deliverable: minimal exact replacements extending code-select.c and
code-image.c for frozen SPEC0046. Do NOT replace entire existing coordinator or
change frozen header. Reuse native envelope,owner/service/retirement. Add private
common runner taking profile,bind profile in root-local control and owner check;
recognized PRF1/PRF2 full scalar integrity in code-image.c. Directory helper must
preserve old2-slot behavior and check4 in PRF2. SnapshotRect then helper then
total scratch/frame; selected root-local views[index],never cast away const.
No build/docs edits needed. Existing code-run.c must remain byte-identical.

Exact reviewed inputs:
- `docs/specifications/SPEC-0046-selected-port-drawing.md`: `f280f9eab1133b28d452d6da83a0eb76f709c9bd019a8d52384911fcb9725d43`
- `docs/specifications/SPEC-0045-integrated-port-selection.md`: `b4886c24e72bf4462b0dafd486401e1b39cdc5faa4c4f9d21ab2a3135ee408ad`
- `personality/system1/code-select.h`: `e798c5c84ab896c279566b3b7465b7e039c7344f9f0bcfaa8956686d46921890`
- `personality/system1/code-image.h`: `6d95d79e9b33817ce2333c4d1323e92275feab884f53d3f10b439606ea91b50d`
- `personality/system1/code-run.h`: `1c4489624e1abccd9dc654aa51d962210c6a10ce37ccbd84ac1447037ff8238b`
- `personality/system1/quickdraw-select.h`: `cd72e096f6a9365ede45fd859df755fec021eb1986002d5ca8269a6c8f838624`
- `personality/system1/quickdraw-view.h`: `4a93567872049f5142a3980eba501c7a2e027c15de794dde11fbbd93cc3bbee7`
- `personality/system1/quickdraw-rects.h`: `db9facdb5ef3742c3c8935255e3f627c30d31b323a8223e470d9851a48c81e09`
- `core/memory/guest.h`: `5a89e549470bd0f3ffe50e7fb714342ff7dc6af0d49a0c154a7baebdf7e113ff`
- `core/services/traps.h`: `a3346547364a2237027dbe1d134b81129e80f9731f4c49c813fc9f68563fd782`
- `personality/system1/code-select.c`: `0a0fe60d3ff0574c0c8c63ad2b0ba9ae9a91c5aaf6e4d1967b98d4e6af3899ae`
- `personality/system1/quickdraw-select.c`: `894f3e06bdfea0add97a659ef71a593b682c88a2310c719478d9e4c0dc191e3d`
- `personality/system1/quickdraw-view.c`: `8bfbb05564a2d6c799790f11c952a1f4b3c43130c2fb855a9035103f5f6da152`

Frozen API:
```json
{
  "personality/system1/code-select.h": "e798c5c84ab896c279566b3b7465b7e039c7344f9f0bcfaa8956686d46921890",
  "docs/specifications/SPEC-0046-selected-port-drawing.md": "f280f9eab1133b28d452d6da83a0eb76f709c9bd019a8d52384911fcb9725d43"
}
```
Complete packet/captures outside Git. CLI permissions are not a hard filesystem sandbox.
