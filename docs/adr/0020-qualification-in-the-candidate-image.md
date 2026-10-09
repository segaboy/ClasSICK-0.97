# ADR 0020: Build the qualification measurements into the candidate image

Accepted scoped design 2026-10-09. SRC-0064 / SPEC-0016; SRC-0059's black-box
boundary and SRC-0060's trial limits remain binding. Human provenance review
pending.

TEST-0023 proposed a separate probe image so the SPEC-0015 candidate would stay
unchanged. The owner delegated the choice to the implementation lead. A separate
image would need its own build, media, VM and trial, and its results would
qualify the machine for a different payload. Building the measurements into the
normal image means one payload, one medium and one trial that both qualifies the
machine and repeats the B2 observations, so the owner's time is spent once.

The bit-20 partner is read, never written, and is chosen from the final memory
map rather than reserved through a fixed-address allocation. A reserved partner
page would add an allocation that can fail at arbitrary addresses, extra
free/retry paths in the loader and an extra owned span. After exit, UEFI Table
7.10 assigns the loader's data and free memory to the image anyway. Code types
are excluded so no code bytes are read. A partner that is not eligible RAM gives
UNAVAILABLE, which counts as incomplete rather than as a pass.

Probing both before controller startup and after READY separates "firmware
handed over with A20 masked" from "self-test changed it". Either alias or a
controller-reported change of Gate A20/System Reset fails closed, because a
masked A20 would make every later owned write unsafe. The controller port read
is kept beside the physical probe: it is cheap, read-only and tells a controller
change apart from a platform one, but only the probe measures the effect.

The firmware's self-reported vendor and revision are captured as identity
evidence. They do not satisfy criterion 6's hash, and this ADR does not change
that criterion. The longer termination drain lets early-failure diagnostics reach
the serial capture; it ends in 1 s at most and does not extend the keyboard run.

No Apple material, firmware internals or third-party implementation is an input.
Real Macintosh/mini vMac validation, Macintosh 128K feasibility and all three
editions remain independently required.
