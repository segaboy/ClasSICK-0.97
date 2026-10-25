# PC286 bank-catalog preflight contract

Scope: unbound NUMERIC backing-catalog preflight under SPEC-0069. It
performs no native pointer use, physical memory access, far conversion,
allocation, commit, BIOS, boot or production ABI, and has no side effects.

## API

`cs_bc_validate(map, banks, count, ram_first, ram_bytes)` takes
`cs_bc_bank {id, physical_base, native_offset}` descriptors and returns
`CS_BC_OK`, `CS_BC_ARGUMENT`, `CS_BC_MAP` or `CS_BC_BACKING`.

## Error order

1. `banks == NULL` gives ARGUMENT. Then `cs_bm_validate(map)`: ARGUMENT
   gives ARGUMENT, any other non-OK gives MAP. A malformed logical bank
   anywhere therefore takes precedence over every backing error.
2. `count == map->bank_count`; RAM interval nonempty, `ram_first <
   0x100000`, `ram_bytes <= 0x100000 - ram_first`. Otherwise BACKING.
3. Per descriptor: id unique among descriptors and present in
   `map->banks`, whose length is used (a zero logical length already gives MAP);
   `native_offset <= 65535`; `bytes <= 65536 - native_offset`;
   `physical_base >= ram_first`, `< 0x100000`, `<= ram_end`, and
   `bytes <= ram_end - physical_base`. All checked by subtraction; no wrap.
4. No two canonical physical spans overlap, whatever their ids. Spans were
   individually range-checked in step 3 before any addition.

Unique existing ids plus the exact count give exact logical-bank coverage
with no extras.

## Explicit engineering inputs

The below-1MiB envelope and the canonical-base/native-offset relationship
are engineering inputs. They are not a discovered RAM map and do not prove
hardware, ownership or alias freedom.

## Caller obligations (not verified)

Truthful, live, aligned, immutable, disjoint map and descriptor arrays with
real capacities; canonical physical bases; a qualified relation to the
future native representation; an independently owned usable RAM interval.

## Non-guarantees

Native chunks remain <= 32767 bytes. Bank admission does not validate a
complete bank as one C16 pointer span. Lifetime, generation, stable
ownership and infallible commit remain prerequisites.

## Implementation notes

C99, C16-safe: 32-bit unsigned arithmetic only, no division, modulo,
variable shifts, allocation, IO, far casts, aggregate assignment or copy
helpers. Loops are bounded by `uint16_t` counts. Id lookup is linear, so
the overlap pass is cubic in bank count, bounded at64 banks. Its target cost
is unmeasured. This is initialization preflight, with no per-byte callback.
