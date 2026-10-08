# TEST-0011 — Current-core complete linkage and GCC conformance

Date 2026-10-08. IMPL-0009 / ADR-0009, existing SPEC-0001/0003/0004/0005.
Original caller-storage, negative fixture, PE/map and behavioral predicates;
Codex self-review, human provenance review pending. No historical reference input.

Protocol: full Clang hosted matrix (37 checks per five configurations, including
x64 ASan/UBSan and actual i686), sanitizer controls, endian object audits and six
x64 debugger sessions/three argument rejections. GCC core matrix (22 checks each)
uses Debug twins, Release and actual i686, without Windows adapters or sanitizer/
debugger claims. Five GCC core-test executables must match between Debug twins.

Complete standalone links cover Clang/LLD and GCC/GNU ld, x86-64/i686, O0/O2,
two fresh builds each: eight profiles, sixteen images. Require matching hashes
within each pair; cross-compiler byte equality is not expected. Each original
core object has no undefined symbols/global mutable data. Complete maps retain
eighteen required original symbols, no libraries and correct actual entry RVA;
bounded PE audit and independent LLVM inspection require no DLL imports/TLS/IAT.
GNU ld may emit an all-zero 24-byte import terminator, explicitly distinct from
an imported DLL. No CRT, libgcc, compiler-rt, libmemory or libchkstk is linked.

Four unresolved-runtime controls (two compilers/two widths) must fail for the
original missing helper. Five image controls reject wrong machine, zero entry,
entry differing from its declared symbol, invalid import RVA and a real user32
import fixture. These images are linked/inspected only, never loaded/executed.
The same integration body executes in guarded hosted tests under both compilers.

Retained development failures: full GCC Windows viewer compilation hit bundled
HGDI_ERROR signed-conversion warnings, so verified GCC scope is core-only with
warnings retained. The exploratory GNU i686 link accepted an undecorated missing
entry name and chose its first function; the entry audit rejects it and final
builder supplies the decorated i686 symbol. Initial auditor parse error was fixed.
The first full wrapper stopped at the intended unresolved-helper stderr under
PowerShell 5.1; scoped native-error capture was corrected without weakening the
nonzero-exit/helper-name assertion. That log remains separate from final results.
The second wrapper completed the positive links and four unresolved-helper
controls, then stopped on flattened mutation metadata. Explicit named records
fix that wrapper error while retaining the same image-rejection assertions.
Final source review also changed the hosted harness to allocated storage:
alignment alone does not give a declared byte array the effective type needed
for native input records. Earlier development logs remain exploratory results.

Final fresh local wrapper completed on 2026-10-08: all 37 Clang checks in five
configurations, existing sanitizer/core/endian/debugger checks, 22 GCC checks in
four configurations, five GCC hosted twin hashes, all sixteen standalone links
(eight matching pairs), four unresolved-helper controls and five image rejections
pass. Fresh offline setup/extraction replay from the pinned cache also passes.
Source/hash and observed remote results are pinned in the snapshot below.
Completed Windows CI 37774811569 at evidence source 3950f33 passes the entire
wrapper and fresh setup/bootstrap checks. Twenty-one named local/remote hashes
match (eight existing Clang hosted, five GCC hosted, eight standalone profiles).
This post-observation result update changes documentation only.

No native-image execution, B2 boot, firmware/stack/exception/device implementation,
Macintosh startup/budget or ARM64 linked execution. No general runtime-free C
guarantee for other programs/options; future helper dependencies need review.
Existing WOW64 debugger limitation remains. Immutable results are recorded in
[the core-link snapshot](../../docs/development/core-link-evidence.md).
