# C16 surface span proof and remaining boundary

2026-10-23; SPEC-0067 / IMPL-0070 / TEST-0083. Proof proposal only for the
current positive-dimension validation in `core/graphics/surface.c`; no core edit.

For pinned16-bit SIZE_MAX65535 and PTRDIFF_MAX32767, mono row bytes are
floor(width/8)+(width mod8 !=0), expressible using32-bit shift/remainder by8.
For RGBA, reject width>floor(65535/4) before computing width*4. Keep row overflow
before stride refusal. A valid stride is then at least row>=1, so division is
defined. Replacing the old span overflow tests by height>floor(32767/stride)
preserves the same overflow result: for nonnegative values, the latter is
exactly stride*height>min(SIZE_MAX,PTRDIFF_MAX). Only after that proof may height
be narrowed and the product evaluated in the native width. Storage refusal
still follows overflow. Argument/format/dimension preflight stays unchanged.

The executed22,176 boundary combinations compare those mathematical forms,
including the512x342 Mono1 span21888 and a32768-byte refusal. This does not
prove compiler-generated target execution, surface_fill alias/transaction
behavior, or permission to cast a guest address. The original production wide
arithmetic is untouched; its old helper imports remain unadmitted. The new
facade's own six objects import no helper. General alignment and a complete
segmented guest backend remain separate prerequisites; a length fitting one
native chunk does not establish that an arbitrary far pointer can address it.
