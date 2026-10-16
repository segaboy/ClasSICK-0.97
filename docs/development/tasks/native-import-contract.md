# Native scene verification guard brief

Baseline public revision `0d0bce1286fc2536e068c0347ae1e3e3dff36293`.

Task boundary: verification/protocol maintenance of completed SPEC-0038 native drawing only. No runtime/driver/ABI/loader, emulator/VM/hardware/reference run or release. Codex alone edits repositories. Return proposed artifacts in final output; do not write files or run commands/tests. Only read packet.txt in this working directory; no parent/sibling files, HQ, credentials, untracked research, manuals, UART implementation/test bodies, Apple/Microsoft implementation, ROM/media, disassembly, emulator source, web, MCP, subagents, skills, memory or remote control. No permission bypass. Refuse unavailable controls instead of broadening access.

Useful defect: initial CI38972004559 passed2812 CTests but failed the older Verify-NativeFramebuffer.ps1 allowlist because it retained cs_drawing_app_probe after boot-scene switched to cs_native_draw. Fixed commit1127806 and completed native checkpoint0d0bce1 pass2812 CI/2280 local tests. Do not rerun/rewrite Claude's pixel tests or Grok's earlier boundary review. New guard prevents this exact class of inconsistent verification settings, without compiling/running target code.

Guard API: scripts/Verify-SceneImportContracts.py, standard-library Python3. audit(root: pathlib.Path)->list[str] deterministic diagnostics; [] means pass. main(argv=None)->int accepts --root (default repository parent of script), prints concise diagnostics/pass and returns0/1 (argparse usage2). Read only four exact paths below, as UTF-8 (accept BOM/CRLF). Never execute/import project scripts, recurse, edit, invoke commands or network. Missing/malformed/ambiguous selected records fail closed. Whitespace and import order are insignificant. Reject duplicate/empty import tokens and duplicate selected records. Ignore single-line commented-out decoys. No claim of parsing arbitrary PowerShell/CMake; document supported literal layouts.

Canonical imports are the unique literal ('apps/boot-scene/scene.c','boot','...') row in scripts/Verify-DrawingAppObjects.ps1. It must contain cs_native_draw and exclude cs_drawing_app_probe. Compare exact sets to the unique literal scene row in scripts/Verify-NativeFramebuffer.ps1; the selected source entry's numeric allowlist in scripts/Verify-UEFIImage.ps1; and only the scene.freestanding CMake add_test block (also check it targets classick_boot_scene_freestanding). In UEFI, derive scene's index from the literal $sources=@(...) list, require unique scene source and unique corresponding numeric $allowedImports=@{...} entry; do not hard-code9 or accidentally use native-drawing's appended import. Canonical set currently seven imports. CMake currently has one extra unused cs_surface_fill permission; propose removing that token only from the scene.freestanding allowlist. Do not weaken the guard to accept it. Compiler import audits remain authoritative for real imports.

Independent test interface: tests/scene-import-contracts.py imports the guard by exact path (importlib.util), builds tiny synthetic four-file trees in TemporaryDirectory and calls audit. Use current documented literal layouts. Tests must prove stale probe rejection, missing native call even when all lists agree, extra import, duplicate token/record, missing/malformed file, UEFI index movement, CMake wrong-block/comment decoy and order/CRLF tolerance. Fixtures are entirely original; no target binaries. Do not count proposed tests as executed.

Exact reviewed inputs at that revision:

- `scripts/Verify-NativeFramebuffer.ps1`, lines1..38.
- `scripts/Verify-DrawingAppObjects.ps1`, lines1..40.
- `scripts/Verify-UEFIImage.ps1`, lines17..38.
- `CMakeLists.txt`, lines1150..1163.

Actual frozen packet identities/line counts are in the qualification JSON. Antigravity also received the reviewed control summary. No private HQ, excluded originals or whole history was included.
