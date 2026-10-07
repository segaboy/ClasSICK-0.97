# ADR-0002 — Guest/native address and format boundaries

- Date: 2026-10-07
- Status: accepted
- Sources: SRC-0001; independent portability design

## Context

Guest interfaces/records and disk formats must not inherit native pointer sizes,
byte order, structure padding, or CPU alignment. Historical details still need
exact-version specifications.

## Decision

Represent guest addresses as explicit integers, accessed through bounded mapping.
Native allocations use native sizes/pointers. Parse external bytes explicitly;
marshal Toolbox arguments/records at the personality boundary. Do not expose
native struct layouts, bitfields, or host pointers as historical records.

## Alternatives

Casting guest pointers or packed disk structs is initially convenient but fails
on different endianness, alignment, pointer widths and malformed data. Globally
imitating classic C widths conflates our native ABI with guest conventions.

## Consequences and verification

Adapters add code and tests. Test high host addresses, bounded guest memory,
unaligned byte streams, overflow, malformed layouts and 32/64-bit hosts. Exact
68000 addressing and classic ABI behavior remain explicit research requirements.

## Review conditions

Finalize widths/address-mask semantics only from approved specifications; optimize
after correctness without collapsing the address/representation distinction.
