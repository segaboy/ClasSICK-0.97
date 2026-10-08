# TEST-0015 — Native framebuffer presentation, boot scene and gate

Date 2026-10-08. SPEC-0009 / ADR-0013 / IMPL-0013; original synthetic fixtures only.
No firmware, VM, device or visible native output; only guarded host buffers are written.

Suites: `framebuffer.conversion/errors/independent` cover 21,600 guarded placement
cases (both byte orders, unusual pitch, unaligned odd-stride sources, INT32 edges),
literal byte-order vectors, every error and precedence, and idempotence.
`scene.oracle/bands/budget/errors` compare pixels against an independent point
classifier, check band-partition independence and staging budgets from 1 to 8192
pixels, arena exhaustion, whole-frame drawing and tampered descriptors.
`native.gating/rejection/presentation` (x64 only) cover the exit/ready gates with
no framebuffer writes, target/arena rejection and the 32-byte trace record.
Each suite has an import audit that allows only exact project symbols.

Protocol `Verify-NativeFramebuffer.ps1` runs the complete TEST-0014 wrapper, then
the new hosted twin comparisons and AArch64 LE/BE compile/import audits. The EFI
image now links surface, arena, presenter, scene and gate objects. It has 34 named
original symbols and must reject an omitted-presenter link alongside the earlier
controls.

Compiled/test/script source `c6289e1a5c404494b6b4d39e20ea23bae2f45887`. Pinned
Windows CI [37806764756](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37806764756)
passes: 209-file guard, bootstrap twins and the full wrapper. Clang x64 66 checks
per Debug twin/Release/ASan+UBSan, actual i686 57; scoped GCC x64 51 per Debug
twin/Release, actual i686 42. Four EFI O0/O2 twins, three DIR64 relocations, one
8-byte writable slot, twenty image rejects and three omitted-object links pass.
Thirty-five named fingerprints: 27 are unchanged from the TEST-0014 local evidence,
the two EFI images change as expected and six are new. See
[the snapshot](../../docs/development/native-framebuffer-evidence.md).

Development-only runs in the lead's Linux workspace (GCC 13.3, Clang 18.1, GCC
ASan/UBSan, Clang/LLD dev EFI link) also pass and are not evidence. There, the
existing `loader.freestanding` check fails only under CMake 3.28 script-policy
defaults; it passes under the pinned CMake 4.4.4. A local pinned replay on the
owner workstation was prepared but not run; it remains optional.

Unverified: native mapping/caching/visibility, write performance, the timed loop,
keyboard, diagnostics, firmware, media, B2, historical behavior and the Mac/mini
vMac and physical-edition gates. Human provenance review is pending.
