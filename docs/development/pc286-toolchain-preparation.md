# PC/AT 286 toolchain preparation

2026-10-23; SRC-0386..0389, ADR-0077, IMPL-0067 / TEST-0079.
[Machine-readable evidence](pc286-toolchain-evidence.json),
[optional reproducer](../../probes/pc286/README.md),
[pinned tool identities](../../scripts/pc286-toolchain-lock.json).

## Result and scope

The pinned Open Watcom2 monthly2026-10-01 Windows x64 host package runs on the
founding Windows11 workstation and its WCC identifies itself as the x86 16-bit
optimizing compiler,2.0 beta Oct1 2026 06:47:30. This is a useful compile-only
candidate, not a qualified complete port or production runtime. No installer,
PATH/registry change, target image, runtime library link, BIOS/ROM, emulator or
hardware run occurred. Extracted tools and five standard-interface headers stay
outside Git; only our original probe/tooling source and reviewed records are here.

## Actual final checks

- 39 final compiler invocations:32 objects emitted,7 expected rejections. The
  rejections are2 deliberately false ABI assertions and5 C11-related probes;
  they are not32 runtime passes or39 application compatibility tests.
- All4 unchanged selected source files(guest.c,surface.c,traps.c,m68k.c) compile
  in small,medium and large models in C99 mode:12 source/model combinations.
  This covers only those files, not the complete interpreter/managers or OS.
- The32 own objects report80286/USE16. Selected post286 register/prefix/opcode
  guards and publisher object tools pass. This is not an exhaustive instruction
  validator, minimum-ISA execution proof, complete link or hardware observation.
- Wrong archive and existing output are refused without overwriting evidence.
  Initial harness result was a DATA-label scanner false positive, fixed by
  restricting the instruction scan to own TEXT sections;final matrix rerun passes.

The diagnostic profile uses -2,-zl,-os and usually -s. The latter suppresses
compiler stack helpers only for these unexecuted objects;it is not a production
stack-safety decision. The checked-stack control retains __STK as an external
reference. No helpers or startup libraries were linked. No full test suite,
normal target build, CI target run or performance benchmark is claimed.

## Findings and dispositions

| Finding | Actual evidence and disposition |
| --- | --- |
| True C16 ABI | Compile-time controls confirm8-bit unsigned char,16-bit int/short,32-bit long,2-byte near and4-byte far pointers. Reject deliberately wrong int/far widths. Adopt only for this pinned compiler/profile |
| Models | Default data/code pointers:small2/2,medium2/4,large4/4. sizeof(size_t)=2;uintptr_t=2 in small/medium and4 in large. Models measured,not chosen as production ABI |
| Trap records | Original shape probes report frame70 bytes,gateway2076 bytes. These are local layouts,not guest serialized structures or whole128K fit |
| High-bit masks | Compile-time controls and own generated code confirm old ~7u misses bit16/31 against32-bit flags;~7UL keeps them. Actual guest.c remains unchanged;reject this idiom for futureC16 adoption |
| Segmented views | Own far probe adds the offset only;the32-bit-index load aliases the16-bit-index load under -os. Ordinary far views cannot cover128KiB guest RAM. Redesign backend/retained views and preflight cross-bank writes before adopting it |
| Pointer identity | Own far ordering compares offsets;far equality compares segment and offset. Unequal representations can alias. Do not infer disjointness from inequality or treat uintptr encodings as physical order;canonical ownership needed |
| Dialect | C99 for-declarations,mixed declarations,long long and selected vendor stdint/stddef interfaces compile. _Static_assert and isolated _Alignof fail;attempting c11 mode does not make the C11 probe compile. Main CMake still requiresC11;no global downgrade or silent macro workaround adopted |
| Helper closure | Arithmetic probes reference __U4M/__U4D/__I4D and64-bit probes reference __U8M/__U8DQ. Actual surface.c references __U8M/__U8DQ/__U8DR through its wide validation/arithmetic. Runtime bodies not inspected,linked or admitted;design original closure or proven checked arithmetic changes later |
| Optimization | Small/medium/large TEXT sizes and helper references are recorded per source. Neither object file size nor these4 partial units establish final RAM/ROM/disk fit or execution speed |

Source-derived points from the worker remain proposals until confirmed here.
Lead corrects the draft's sizeof(char) tautology,pointer-identity wording and
blanket32-bit cost/model claims. Current byte/word/long alignment,permissions,
no-partial-write/fault and lifetime contracts must survive any backend redesign.

## Source and worker handling

Use the publisher's dated [release](https://github.com/open-watcom/open-watcom-v2/releases/tag/2026-10-01-Build),
[license](https://raw.githubusercontent.com/open-watcom/open-watcom-v2/2026-10-01-Build/license.txt)
and selected [compiler-option interfaces](https://open-watcom.github.io/open-watcom-v2-wikidocs/cguide.html).
The archive SHA256 matches publisher metadata. Retain the local unmodified-tool
license;this is not permission to ship a compiler runtime/helper with ClasSICK.
No compiler/runtime implementation source or manual samples were adopted.

One Claude Opus5.5 low primary received11931B/224 lines of reviewed original
inputs. Restricted-context call ended DNS ENOTFOUND after182.91s with no artifact;
one network-authorized correction of the same packet succeeds in94.14s,197 lines/
1096 audit words,zero tools/edits/tests. No fan-out or requalification. Reported
$0.2536408 list/session cost is not balance or measured Codex saving. Lead
integrates the probes,audits current-source objects and writes independent ABI/
negative controls and the reusable harness. Other workers idle;all balances
unverified. Raw sources,transcripts,listings and objects remain outside Git.

## Next bounded work

P286-1 remains partial:specify the C16 language/ABI and helper policy,then the
checked segmented guest-memory contract and measured per-segment map(P286-2).
Preserve current 68000/x64 C11 semantics and native performance requirements.
Do not write a loader or admit firmware merely because these objects compile.
The selected5170 6MHz/640KiB/EGA and provisional20MB fully installed hard-disk
product goal persists;Word edition,fit,speed and full application support remain
unqualified. No system-floppy packaging promise. Existing paired release order,
source/native-app/edition/physical/128K/B2,trial and publication gates persist.
