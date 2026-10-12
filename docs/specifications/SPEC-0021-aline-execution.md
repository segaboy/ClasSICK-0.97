# SPEC-0021 - Bounded 68000 A-line execution

Finalized v1, 2026-10-12, lead self-review, human provenance review pending.
SRC-0092-0094 / ADR-0024 / IMPL-0025 / TEST-0029.
Profile `classick-aline-probe-v1` extends SPEC-0020, not historical SPEC-0017.

## Published facts

SRC-0093: Motorola MC68000UM ninth edition 1993, 6.2.1 Table 6-2 (6-7),
6.2.4 Figure 6-5 (6-9/10), 6.3.6 (6-14): Axxx takes vector 10 at 0x28;
the 68000 ordinary frame stores SR then PC on the supervisor stack. The
interrupted A-line remains to be emulated; successful handling must advance it.
1992 M68000PRM MOVEQ 4-134, NOP 4-147, RTS 4-169 and RTE 6-84: MOVEQ
sign-extends its byte to Dn and updates N/Z, clears V/C, preserves X; RTS pops
four-byte PC, preserves SR; RTE restores SR/PC. Encoding/prose only; no samples.
Publisher-successor URLs are in SRC-0093. No local download/hash claimed.

## Independent boundary and limitations

The shared `cs_m68k_aline` accepts a fetched A-word and caller-owned value frame.
Require 24-bit even PC with room for a next word, supervisor S=1 and T=0.
Reject unsupported mode before calling SPEC-0020. Successful service dispatch
advances PC by two; every error preserves frame/directory. Reports distinguish
CPU argument/address/mode/opcode/service failures; service report retains the
offending instruction address and logical target. A7 never changes here.
Caller owns actual exception entry, memory fetch, stack and return.

Portable `cs_m68k_step` is a deliberately small interpreter: all MOVEQ forms,
NOP, RTS and Axxx only. Caller supplies separate nonoverlapping, truthful native
buffers for read-only executable bytes and writable big-endian stack, each in
24-bit even nonempty logical spans. No raw guest address becomes a native pointer.
Fetch/stack/return boundaries are checked without wrap; an RTS target must be
in the executable span. High address bits are refused, not silently aliased.
Unknown words stop explicitly without state changes. Trace/user mode refused.

Successful A-line steps use the shared handler. The interpreter leaves a six-byte
completed SR/next-PC frame below the unchanged A7, modeling immediate exception
return, with no vector fetch or general RTE instruction interpretation. Validate
all stack/fetch conditions first; service failure writes no frame. This collapsed
model is an engineering choice, not cycle/prefetch/interrupt/bus-error emulation.
No arbitrary guest patches, executable intrinsic stubs, user stack switching,
callbacks, Toolbox ABI, other instructions or legacy applications are supported.
Each step is bounded; the original probe caps execution at six steps.

## Original common instruction fixture and native integration

One twelve-byte independently authored fixture: MOVEQ #0x46,D0; A146;
MOVEQ #0x20,D0; A047; A146; RTS. Both C and native assembly consume the same
byte-list header. It looks up the original Get token, installs it in slot 0x20,
looks up that slot and returns. Symbolic tokens remain non-executable identities.
The portable probe checks six successful steps, three service calls, final
D0=32/A0=Get token/SR=2710, restored stack, untouched other registers and table.
Eight checks/status0/signature 414c4e31. Existing ten-check C probe stays separate.

Mac vector 10 points to an original assembly bridge. Save D0-D7/A0-A6 in a
72-byte reserved frame; frame A7 is supervisor pre-exception SSP, SR/PC copied
from the actual six-byte CPU frame. C validates saved PC against the original
twelve-byte fixture and owned stack range, fetches its word, dispatches shared
handler, and updates saved SR/PC. Assembly restores registers and executes RTE.
Mode failure stops; user A7 is deliberately unsupported. An original wrapper
tests all preserved register sentinels, SR and service results after RTS, restores
the native C ABI and prior SR. Reset slot 0x20 to unsupported after the portable
probe, so the hardware Set must install the entry itself before the final Get.
Require exactly three hardware A-line returns.
No actual CPU frame or legacy buffer is trusted merely from a C struct layout.

PC runs the bounded interpreter after descriptor installation, on owned stack
buffers and the existing loader-tail gateway. Store its result after the old
service result without moving that result. Export a separate terminal helper;
native selected-register proof requires successful original second-60 serial
completion before queries. RCX must equal bundle+503808+2088, R9=8/R10=0,
R11=414c4e31, R12=6/R13=3, R14=43534131; RIP/RSP remain in our ordinary
terminal/stack. Two delayed observations must agree in each of three fresh starts.
Mac appends diagnostics, preserving prior TRAP/fault offsets and 8-KiB stack.
Two fresh Snow starts must reach own scene/stage4 with zero fault, expected seal,
the old ten-check result, new eight-check/zero-status/signature/steps/calls and
exactly three native A-line calls. Only own diagnostic RAM/scene is observed;
no code/ROM/disassembly viewer or generated guest input. Close both windows and
power off the PC trial; verify unchanged media/prior machines.

## Verification and scope

Test all 2,048 MOVEQ words against independent sign/flag arithmetic, every
16-bit unsupported word, all A-format failure classes, span/stack/RTS edges,
transactional failure and guard bytes. Test full original program and two
independent gateways. Verify Clang/GCC widths, sanitizers, endian object audits,
no runtime imports and deterministic own EFI/ROM builds with negative controls.
Native trial claims require separate exact-image/platform admission and retained
observations; builds alone do not show CPU entry/RTE. Continue on the already
cleared platforms only under SRC-0092; retain earlier images/machines. Never
inspect firmware/emulator implementation, guest code dumps or Apple input.
SPEC-0017, full CPU/OS/physical/edition gates and B2 criterion 6 remain open.
