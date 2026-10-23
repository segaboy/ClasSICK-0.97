# IMPL-0070 - Isolated C16 assertion and chunk facade

SPEC-0067; SRC-0394/0398. Lead original `arch/i286/c16.h`, optional compile
probe/harness and host checks. Owned C99 typedef assertion, checked single native
chunk narrowing bounded by SIZE_MAX/PTRDIFF_MAX, full-width flag clearing and
numeric guest-offset divisibility. Refusal preserves output. No C type alignment
query, pointer-cast permission, far-pointer alias proof or segmented access.
Production `guest.c`/`surface.c` and all C11 targets remain unchanged. P286-1
stays partial. [Evidence](../../docs/development/cpu-c16-evidence.md).
