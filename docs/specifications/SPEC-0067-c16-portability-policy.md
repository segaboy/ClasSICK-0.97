# SPEC-0067 - C16 language, checked-arithmetic and helper policy

2026-10-23;SRC-0390/0392/0393,ADR-0078;TEST-0081 cases are unexecuted.

Status: contract only for partial P286-1. No implementation, memory-model or ABI freeze, helper body, loader, or target run. Paired Mac/x64 C11 release stays primary. The 286 product goal remains the selected 5170 profile (6 MHz, 640 KiB, EGA, provisional 20 MB hard disk). Fit, Word edition, and speed stay unqualified. No system floppy in regular use.

## Scope

In: a C16-only adaptation policy for dialect, type widths, masks, shifts, division, guest spans versus `size_t`, pointer canonicalization, ClasSICK-owned assert and alignment interfaces, and helper admission.

Out: opcode listings, register or call conventions, runtime or OMF bodies, firmware, emulators, benchmarks, CMake C11 changes, and production edits. `guest.c` stays unchanged. Source review and compile observations stay separate records. This text does not restate the full toolchain study.

## Preconditions

1. Widths bind only the reviewed WCC 2.0 beta pin (2026-10-01, x86 16-bit optimizing compiler). A new pin needs fresh qualification;earlier observations remain dated evidence.
2. Cited facts are compile-time observations on unlinked objects (39 invocations, 32 objects, 7 expected refusals, 2 harness controls). They are not execution proof.
3. Main API error, overflow, alignment, and transaction results must stay at least as strict.
4. No link, startup library, or unknown helper binary. `-s` was diagnostic on those objects and is not stack policy. A checked-stack control still referenced `__STK`.
5. Small, medium, and large remain measured candidates. No model is chosen.

## Dialect

This pin compiled C99 `for` declarations, mixed declarations, `long long`, and the selected vendor `stdint`/`stddef` headers. C16 code may use that subset.

`_Static_assert` and isolated `_Alignof` failed. A C11 language mode did not make that probe compile. The primary tree keeps its C11 CMake requirement. C16 must not redefine reserved keywords and must not paper over the gap with a silent keyword macro.

Shared C semantics with Mac/x64 sources. Any C16 fast path is specified and measured on its own. All three targets retain native performance requirements;paired Mac/x64 delivery is the primary workstream.

## Widths (this pin only)

`unsigned char` is 8 bits. `short` and `int` are 16. `long` is 32. Near pointers are 2 bytes and far pointers are 4. Default data/code pointers: small 2/2, medium 2/4, large 4/4. `size_t` is 2 in all three. `uintptr_t` is 2 in small and medium and 4 in large. `uint32_t` is 4 bytes. `uint64_t` and `uintmax_t` are 8. Deliberately wrong `int` and far widths were rejected at compile time.

`sizeof(char)==1` is not evidence. These sizes are profile facts, not an adopted production ABI.

## Masks, shifts, and division

A mask is formed in the width of the flags it changes. `~7u` is a 16-bit complement and drops bits 16 and 31 of a 32-bit flag word. `~7UL` keeps those bits. That miss is a compile-time fact. Future C16 adoption rejects the narrow idiom. Current `guest.c` is untouched.

Before a shift, prove the count is in `0 .. width-1` of the promoted left operand. Prefer unsigned operands;for signed left shift also prove nonnegative input and a representable result. Avoid or explicitly specify right shift of negative signed values;count bounds alone are insufficient. Before `/` or `%`, prove the divisor is nonzero. For signed division, also exclude minimum-value divided by `-1` unless the proven bounds make that pair unreachable.

The arithmetic operator is allowed only after those input proofs. A later compare does not repair undefined behavior already evaluated. Constant masks and constant divisors are written so a C99 translator can fold them. That is a source rule, not a new folding measurement.

## Spans and pointers

Guest dimensions and addresses may be 32-bit. `size_t` is 16-bit in every measured model, so every narrowing to one native chunk needs an explicit representability check before storage. This does not reject a valid larger logical guest operation:the future segmented backend must split it into checked chunks while preserving its original result/fault/commit contract. A product of two `size_t` values does not prove a 32-bit guest span.

`uintmax_t` may hold a bound. Each 64-bit multiply or divide remains a helper candidate until a width proof removes it. `surface.c` validation references `__U8M`, `__U8DQ`, and `__U8DR` through its wide checks. Its `SIZE_MAX` and `PTRDIFF_MAX` tests are not re-proven for 16-bit `size_t` and `ptrdiff_t` by this contract.

Compiled far addition changes the offset only. Far equality compares segment and offset. Far ordering compared offsets. Unequal representations can alias. Inequality is not disjointness. A `uintptr_t` encoding is not physical order. One far pointer is not a 128 KiB view. Canonical ownership is required before use. Segmented access and retained views stay unresolved until P286-2.

## Owned assert and alignment

ClasSICK will own new names, chosen in a later header-only task, for a compile-time assertion built from a C99 constraint this pin accepts, and for an alignment query that carries a per-type, per-model proof.

`offsetof` reports that member's offset and the padding before it. It is not general alignment of the type and it does not authorize an arbitrary cast. Treating that padding as `alignof` requires a separate proof. Byte, word, and long alignment, permission checks, no partial write on fault, and lifetime rules survive every backend redesign.

## Helper admission

Prefer original C whose width proof shows the operation stays inside a type that does not need a multi-precision helper. If that proof fails,do not silently admit a compiler helper. An original explicitly multiword operation behind a project-owned C interface may be specified and verified without guessing a compiler-private ABI. Implementing compiler-named helper symbols instead requires review of the publisher's documented calling interface first. Both routes require output/helper audits and retained semantics.

Symbol names are not a calling convention. Do not author stubs for `__U4M`, `__U4D`, `__I4D`, `__U8M`, `__U8DQ`, `__U8DR`, or `__STK` from guesses. No third-party compiler runtime is admitted for shipping by this contract. This contract does not patch `surface.c`. Overflow returns stay. `cs_surface_init` stores only after success. `cs_surface_fill` must not gain a weaker early store.

## Verification (specified, unexecuted)

- V1. Reuse the recorded39-case compiler matrix. When an adaptation is implemented,compile its own positive/negative cases;do not rerun unchanged units merely to claim progress.
- V2. A unit that uses `_Static_assert` or isolated `_Alignof` is rejected. A C99 owned assert with the same predicate is accepted.
- V3. Compile-time constants show `~7u` dropping bits 16 and 31 and `~7UL` keeping them.
- V4. Review rejects an unproven shift count, a zero divisor, and signed minimum divided by `-1`. A proven form is accepted.
- V5. A single native chunk request that cannot be represented is refused before narrowing or effect. A valid larger guest operation uses the later checked chunk interface;its error,alignment and no-partial-commit behavior stays intact.
- V6. Reuse the pinned far-pointer compile observations as dated evidence. Later test canonical ownership and cross-bank runtime access;compile observations do not prove physical aliasing behavior or actual segmented execution.
- V7. Adapted objects have an explicit helper-reference list. The admission set stays empty until the documented interface review exists. `-s` objects are not a stack waiver.
- V8. `offsetof` padding is not recorded as general alignment without a separate proof. The same API on x64 C11 is unchanged.

No unit run, target execution, or hardware timing is authorized here.

## Next bounded work

Close the rest of P286-1 with one unlinked header-only adaptation interface and a width-proof note for the surface span checks. Then P286-2: the checked segmented guest-memory contract and a measured per-segment map. Loader and firmware stay out. Clean-room, native-app, target-run, edition, and publication gates stay in force.
