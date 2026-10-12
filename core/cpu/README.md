# Original bounded 68000 instruction boundary

SPEC-0021 supports MOVEQ, NOP, RTS and the SPEC-0020 intrinsic A-line services
in a checked supervisor-only probe profile. Every memory access uses a supplied
bounded native buffer and a separate logical address. The native Macintosh
bridge owns real exception entry/RTE; PC uses an immediate-completion model.
This is neither a full 68000 emulator nor historical application compatibility.
