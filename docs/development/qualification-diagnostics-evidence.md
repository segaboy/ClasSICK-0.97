# Qualification diagnostics evidence — 2026-10-09

[SPEC-0016](../specifications/SPEC-0016-qualification-diagnostics.md) /
ADR-0020 / IMPL-0020 / TEST-0024. The native image now measures what
[the B2 audit](b2-qualification-audit.md) found missing, and reports the
firmware's own name and revision. Hosted and unloaded evidence only: this image
has not run in a VM. B2 stays partial.

## What the image now reports

After the banner `ClasSICK 0.97 UART v2` the serial log gains four lines:

| Line | Meaning |
| --- | --- |
| `fw rev=… state=… vendor=…` | Firmware's self-reported revision and name (state 0 complete, 1 truncated, 2 absent). Not a firmware hash |
| `own img=… bun=… stk=…` | Our image base, bundle base and stack top, so a register query can place RSP in the owned stack |
| `qual port=<before>/<after> a20=<before>/<after>` | Controller output port read before and after self-test; alias result before startup and after keyboard READY (0 pass, 1 alias, 2 unavailable, 3 not run) |
| `alias q=… type=…` | The bit-20 partner address and its memory-map type |

A run qualifies the machine for the SPEC-0012 precondition only if every run
shows `a20=00000000/00000000`, equal port bits 0–1 and no result `0000000E`
(MACHINE). Unavailable (`2`) is incomplete, not a pass. Frame and result lines
are unchanged.

## Candidate identity (local prediction)

Built locally with the pinned LLVM-MinGW 20260908 Linux release, which reproduces
the earlier audited payload byte for byte:

| Item | Value |
| --- | --- |
| O2 payload | 43,520 bytes, SHA-256 `920ed92c2b30609718d1ed95867e0cf79aab82ae5f9ce76ffee2c9b806ce5d16` |
| O0 payload | 60,928 bytes, SHA-256 `6a7469c98d75f977b7ce4044901180f14d12728708d0978edfe67eb621ad05f1` |
| Raw disk | 64 MiB, SHA-256 `e2e6a431722b39d6ed7c215bbc60727aef74ae776f74934f60ddfd0e6f4069e9` |

Pinned Windows CI is the authority; its result is recorded in TEST-0024.

## Trial plan (needs the owner's go-ahead)

On the SPEC-0015 machine configuration, prepared fresh with
`Prepare-VMTrial.ps1 -Candidate SPEC-0016`: three headless cold starts with
serial capture and no input, then one visible session in which the owner presses
Space twice. Each run's transcript must show the qualification result above,
`sec=0000003C` and `result=00000000` (or `0000000C` after a deliberate Escape).
A failure is recorded as observed; it is not investigated inside the firmware.
