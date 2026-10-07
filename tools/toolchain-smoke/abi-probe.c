/* IMPL-0001: compile-only freestanding data-width probe; no OS implementation. */
#include <limits.h>
#include <stdint.h>

_Static_assert(CHAR_BIT == 8, "Bootstrap requires 8-bit bytes");
_Static_assert(sizeof(uint16_t) == 2, "16-bit interchange type required");
_Static_assert(sizeof(uint32_t) == 4, "32-bit interchange type required");
_Static_assert(sizeof(uint64_t) == 8, "64-bit arithmetic type required");
_Static_assert(sizeof(uintptr_t) == sizeof(void *), "Pointer-width integer required");
