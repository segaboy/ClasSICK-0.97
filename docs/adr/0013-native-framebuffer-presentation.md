# ADR 0013: Native framebuffer presentation and original boot scene

- Accepted scoped original design, 2026-10-08; owner SRC-0044 continuation.
- SPEC-0009 / IMPL-0013 / TEST-0015; UEFI GOP fields SRC-0034. Human provenance
  review pending. Builds on ADR-0010–0012 without superseding them.

## Context

B2 requires direct post-handoff rendering through the core surface API. The loader
already validates and copies the current GOP mode, and the owned bundle reserves a
128-KiB arena. A full-screen RGBA8 staging image (3 MiB at 1024×768) cannot fit
there. The native stop path also ran after failed exit attempts, so device access
had to be gated explicitly.

## Decision

Split presentation into three original layers. A portable `platform/pc` presenter
converts clipped RGBA8 rows to RGB/BGR-reserved pixels, using volatile byte stores
and a zero reserved byte. An `apps/boot-scene` module renders a deterministic
geometric scene band by band, with at most 64 KiB of staging from the core arena.
A hosted-testable `platform/uefi` gate presents only after a recorded successful
exit and a ready descriptor marker. It writes a bounded trace record in owned
storage. The EFI entry wrapper only converts handoff addresses to identity pointers.

The scene draws corner markers in distinct primaries so that a future capture can
confirm orientation and channel order. It is an original test pattern, not a
historical startup image.

## Alternatives

A full-frame staging image would require revising the bundle ledger and losing the
arena's headroom for input/clock state; tiles were rejected for v1 because whole-
width bands keep conversion contiguous. GOP Blt is a firmware service and is
forbidden after exit. Writing the core surface directly over the framebuffer would
expose RGBA alpha in the reserved byte and couple the core to volatile MMIO.

## Consequences

Hosted tests verify conversion, clipping, scene pixels, budgets and gates; the EFI
image links the new objects without imports or writable data. Real framebuffer
mapping, caching and visibility, the 60-second timed loop, keyboard, diagnostics,
firmware eligibility, media and B2 remain open. The three-edition contract and the
required real Macintosh/mini vMac goals are unchanged.
