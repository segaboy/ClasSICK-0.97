# ADR-0011 — Original UEFI loader and an inspected owned-stack scaffold

Date 2026-10-08. Accepted for SPEC-0007, development verification only.
Owner SRC-0037; interface SRC-0038/SRC-0039/SRC-0041; review-only SRC-0040.

Implement actual x64 firmware-call orchestration from public interfaces using our
own declarations. Keep it shared with hosted mocks; a separate minimal EFI wrapper
enforces the nonreturning boundary after any exit attempt. Cache callback pointers
and metadata before attempts, capture only into the preallocated owned bundle,
validate complete image/bundle coverage and permit at most three stale-key exits.
Before attempts, free a valid owned allocation once on failure. Malformed allocation
addresses remain untouched; after an attempt only recapture/exit is permitted.

The native transition masks ordinary interrupts, clears direction, switches to
our aligned stack with x64 home space and calls an original terminal scaffold.
It records entry and re-halts. Own exception tables, paging/device work and B2
remain later contracts. Retaining firmware state is an explicit development limit;
CLI cannot make exceptions/NMI safe. No firmware callbacks or direct graphics are
used by the terminal scaffold. No native execution claim follows from its bytes.

Link original C/assembly without default libraries, inspect EFI subsystem/entry,
sections/permissions, zero imports and valid relocations including an intentional
address anchor. Compare fresh O0/O2 twins and reject corrupt image controls.
Package the audited O2 file in the removable payload directory; formatting media
and installing/running a VM are separate. A stop scaffold does not complete B2.

The exact installed-product x64 build path is now found, correcting the earlier
ARM-manifest lead. Selected module metadata narrows the provenance concern, but
copyright/license declarations cannot establish transitive derivation or the
installed firmware's binary/source correspondence. Do not fetch/run uncertain
firmware to test our loader. Firmware remains ineligible pending that review.

An external loader library would introduce unrelated implementation provenance
and a runtime/link boundary. Our small fixed interface profile is independently
auditable and intentionally rejects unsupported table extensions/modes/bounds.
General UEFI/platform support and other architectures need separate validation.
Real Macintosh/mini vMac replacement startup and physical edition gates persist.
