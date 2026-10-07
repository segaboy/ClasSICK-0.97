# SPEC-0001 — Bounded portable graphics surfaces

Status: proposed independent core contract; **not historical QuickDraw behavior**.
Authoring date: 2026-10-07. Sources: founding requirements SRC-0001; design ADR-0001
and ADR-0002. Implementation: none. Review before freezing the C interface.

## Purpose and scope

This is the first selected implementation subsystem. It enables native core
graphics without a Macintosh display assumption and supplies a small testable
foundation for both hosted rendering and native boot. No allocator, font, window
manager, guest trap, or host graphics API is required for headless tests.

## Observable contract to implement

- A surface describes caller-owned storage, storage length, width, height, stride
  in bytes, and explicit pixel format. Creation validates all calculations before
  writing. The storage remains owned by the caller for the surface lifetime.
- Begin with an explicitly specified 1bpp MSB-first bitmap and one explicit
  32bpp byte-channel format. Do not encode native-endian `uint32_t` pixels as the
  format contract. Declare channel order, padding bits, and row layout precisely
  during interface review. Alpha/compositing is deferred.
- Reject unsupported formats, zero dimensions, insufficient stride/storage, and
  overflow with stable distinct results. Invalid construction changes no bytes.
- Clear/fill rectangles accept signed coordinates, clip to valid bounds, preserve
  row padding and pixels outside the clipped region, and perform no out-of-bounds
  access. Empty/intersection-free rectangles succeed without mutation. Define
  half-open rectangle edges and reversed-coordinate rejection in the C contract.
- Algorithms cannot call a platform drawing API. Presentation is an adapter action
  with a separately reviewed contract. Errors cannot depend on physical display
  geometry or the current host window.
- No global heap allocation, guest address assumption, hardware dependency,
  mandatory floating point, or mutable global surface state.

## Required tests

Original test buffers include guard bytes, odd widths, extra stride, and minimal
lengths. Tests compare exact independently specified bytes for monochrome bit
placement and channel order; check clipping at all edges, signed extremes, empty
rectangles, rejected overflow, preservation of padding/outside bytes, and two
independent surfaces. Run with warnings/errors, sanitizers where validated, and a
freestanding compile/import check. Parameterize geometry beyond 512x342.

Expected bytes must come from this contract, not captured Apple pixels. These
tests prove core behavior; later QuickDraw tests require their own versioned specs.

## First hosted scene

After headless acceptance, a Win32 adapter presents simple original rectangles in
multiple sizes, including a non-Macintosh geometry. A normalized input event
changes one rectangle. No historical font, icon, Finder art, or Toolbox API is
needed. This forms part of B1 after memory/events are integrated.
