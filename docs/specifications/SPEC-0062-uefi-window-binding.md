# SPEC-0062 - Opt-in UEFI window binding and diagnostic record

2026-10-22;original implementation refinement of SPEC-0059/0061.
[Native interface](../../platform/uefi/window.h). x86-64 UEFI profile only;
not a BIOS payload ABI, target qualification or historical specification.

The wrapper borrows truthful live aligned fixed h/binding/devices/memory/keyboard
objects, full map bytes, framebuffer,131072-byte arena and16384-byte trace region.
The binding's physical/size/base maps physical P to base+(P-physical). It covers
each actual bundle/framebuffer span without physical/native wrap. Caller promises
accessibility and exclusive synchronous ownership; numeric windows do not prove
mapping. The memory descriptor retains its separately validated ACPI read window.
Opaque callback contexts and instruction pointers remain caller lifetime duties.

Raw preflight validates fixed object ranges/alignment before following pointers,
nonnull port/I/O callbacks,seconds1..3600,full mutable arena/framebuffer/trace
spans and full readonly h/binding/devices/memory/keyboard/map spans. Reject wrap
or any writable alias, including readonly objects embedded in the borrowed arena.
Require declared arena131072/trace16384 and all translated pointer identities:
h at bundle base,dev.arena/framebuffer,map bytes and trace argument at the window
record's trace-base+2048. Use complete full native spans,not only used prefixes.
Any raw refusal returns ARGUMENT without caller writes,allocation,I/O or pixel
reads. All native objects must be truthful; numeric checks cannot catch faults.

After raw preflight only the160-byte record may change until every gate succeeds.
Require handoff magic0x4353303937484F46,version2,exact sizeof(h),bundle524288,
map_base=bundle+4096,map_size<=262144 and matching native memory size/stride/version,
arena_base=bundle+348160,trace_base=bundle+479232,stack_top=bundle+331776,
fault_top=bundle+348160 and valid bounded bundle/image physical extents. Require
EXITED/status0,ready1,cs_uefi_check_map and cs_uefi_check_owned success,span_count1..8
and an exact kind2 whole-bundle owned span. Other owned spans stay subject to the
existing ownership validator. Validate physical framebuffer with existing API and
min512x342;resource mapping translation must cover its entire declared size.
Missing/wrong handoff,exit,readiness,map,timer,8042,target or arena gets its named
wrapper result and raw dependency cause where one exists,otherwise0.

Validate ACPI window native/physical bounds;find PM timer and require8042 via
existing admitted APIs. ACPI reads must reject actual translated read spans
overlapping the mutable arena,trace or framebuffer before delegating to
cs_native_read;do not reject a large declared identity window merely because it
covers those addresses. Discovery reads do not initialize the scene/controller.
Validate the initial raw PM tick before arena writes/start. Then one cs_arena_alloc
of align_up(sizeof(session),alignof(max_align_t))+21888+48+4*width bytes,from a
fresh local arena descriptor. Partition state/canvas/payload/row at fixed addresses;
no later allocation/reset/reuse. max_align_t-aligned arena is a caller precondition.
No full physical RAM framebuffer. Retain the session graph after stop/fault.

Start the real SPEC-0061 session. On failure record SESSION with raw session result;
first-publication and owner failures remain distinguishable in session fields.
After startup publish a trace snapshot and optionally call observer. For each turn
clear current receipt/consumed/drained BEFORE acquiring the PM tick;sample/extend
through cs_pmtimer_sample,then call session_poll once. On value failure call
session_poll(clock_ok0),without controller/owner calls,then record CLOCK/raw timer
cause. Session CLOCK also maps to wrapper CLOCK;other failures map SESSION.
No seconds deadline or Escape termination. Count wrapper polls with saturating64
diagnostics;never use counters for freshness. A nonzero observer result explicitly
calls session_stop and records STOPPED. NULL observer runs until fault. Observer
sees valid readonly native trace/session only after startup or a turn,not on gate
failure. No automatic repair,detach,controller shutdown or firmware services.

Trace is160 bytes,little endian,within existing allocation at offset2048. It is
not the old keyboard/UART record. Offsets below are32-bit unless marked64:

|Offset|Field|
|---:|---|
|0/4/8|magic CWP1/version1/size160|
|12/16/20/24|wrapper phase/result/module/raw cause|
|28/32|native readiness/handoff stage|
|36/40/44/48/52|bus result/session phase/stage/result/raw cause|
|56/60|current delivery kind/consumed|
|64/68/72/76|logical pointer x/y/down/suppression|
|80/84/88/92/96|window x/y/dragging/pattern/product inputs|
|100/104|last complete RAM frame/complete publication counts|
|108/112/116/120|current publication phase/completed rows/required/raw cause|
|124|current keyboard drain count|
|128/136|64-bit saturating poll turns/extended PM ticks|
|144/148/152/156|arena bytes used/timer bits/timer port/reserved0|

Gate snapshots zero all unavailable session/progress fields. Active NONE/clock/
queue/controller turns reset publication phase/rows/required/cause to0;old complete
frame counts remain. Startup publication is current only if start reached PUBLISH
or BUS;later publication current only for session stage PUBLISH. Stop snapshot
retains that turn's truthful progress. A failed partial publication never advances
complete-frame count. Signed native coordinates/causes serialize modulo2^32.
Snapshot stores are software diagnostics,not an atomic debugger snapshot or
hardware/MMIO exception recovery guarantee. No UART observer is installed.

The explicit build switch -WindowDrawing sets CLASSICK_WINDOW_DRAWING=1 and is
mutually exclusive with selected/outline drawing. Its entry installs native CPU
tables as before,then directly enters this wrapper;old drawing/timer/UART entry
calls are bypassed. Other profile source behavior stays intact. Images are built
and inspected without execution. Dedicated trace reader accepts only this exact
size/magic/version/reserved profile;it never guesses a legacy record format.

TEST-0070 cases require individually identified actual coverage. Reusing a leaf
test is not a full binding pass. Original synthetic maps/tables/port transcripts
are E1 only. No new emulator/VM/hardware run,release,Mac M1-M7 waiver,full128K fit
or BIOS support is implied. New source images need separately gated qualification.
