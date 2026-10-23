# SPEC-0073 - Diagnostic-only PC input-loss observation

2026-10-23; SRC-0413/0414 / IMPL-0075 / TEST-0088. This is a measurement
candidate, excluded from normal previews and release packaging.

The unchanged SPEC-0062 readonly observer runs after a receipt, before the
next poll. NONE/PACKET returns0 without effects. First LOST reads only the
live session's own scalar diagnostics into eight64-bit words, then enters an
original nonreturning native backend. It neither changes the loss policy nor
mutates the graph, pixels, trace or hardware. No allocator, UART or guest-memory
debugger read is used. This is not semantic STOPPED.

The Windows x64 backend accepts the array in RCX, copies its pointer before
loading r8..r15, then stays in its own pause/jump loop without further memory
access. It never returns; normal no-loss observer paths obey the C ABI. Native
interrupts are already disabled by the enclosing preview. Host tests intercept
the backend and never execute that loop.

| Register | High32 | Low32 |
| --- | --- | --- |
| r8 | Original64-bit magic435350524F424531 | (whole magic) |
| r9 | delivery kind | bus phase |
| r10 | errors | packets |
| r11 | auxiliary bytes | last byte |
| r12 | decoder header | decoder used |
| r13 | pointer down | pointer suppress |
| r14 | product input count | decoder X byte |
| r15 | pointer X, uint32 encoding | pointer Y, uint32 encoding |

Counters retain their documented saturation; they are not deltas. Before
interpreting a register read, verify RIP lies in the exact own linked spin
symbol and verify the magic. No memory scan, firmware inspection or emulator
implementation reading is authorized. This snapshot can identify a guest-side
drop; it cannot by itself identify a host cause or prove human usability.

`Verify-UEFIImage.ps1 -WindowDrawing -WindowInputProbe` is explicit and refuses
other drawing/fault combinations. It requires linked observer/backend/spin and
the existing image/object/twin/corruption controls, plus two omitted-probe
controls. Normal builds keep a NULL observer and exclude both probe modules.
Require byte equality of the normal candidate after integration. Any actual
probe trial requires the current release's owner-approved isolated original
candidate scope and exact artifact verification; publication remains separate.
