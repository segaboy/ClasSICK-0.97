# PC window boot worker prompts

2026-10-22;SRC-0357;baseline `fa4c63a34317ceca62247f6d99445369057b7f35`.
Original public-only input folders;captured output only;no OS isolation claim.
Exact input identities in the [receipt](../pc-window-boot-evidence.json).

```text
ClasSICK0.97,Windows11,supervised Claude primary. Codex sole repo writer.
Implement ONLY platform/uefi/window.c from frozen original public SPEC/API below.
Return JSON {files:{"platform/uefi/window.c":"complete source"},handoff:{result,
actionable_findings,tests_executed:[],blockers,decisions_needed}}. <=400 C lines,
handoff<=120words. No tests/docs/build files/header changes. No tools/reads/commands/
edits/web/MCP/subagents/manuals/HQ/untracked originals/UART/proprietary inputs.
All inputs inline through EOF. Original C11,GPL-3.0-or-later,Copyright(C)2026 Dean
Howell. Strict -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion.
x86-64 only,freestanding,no libc/memcpy/memset,allocation/global state or firmware
calls. Scalar field stores/copies to avoid O0 compiler helpers. Native stack frame
must fit2048B;session itself in caller arena,not on stack. One cs_arena_alloc.
Use explicit byte LE stores;no packed structs or type punning. Validate full
numeric ranges and aliases before writes;objects truthful/aligned by caller.
Gate result may write validated160B trace only;raw argument refusal writes none.
Native h/binding/devices/memory/keyboard readonly descriptors can share each other;
mutable arena/fulltraceallocation/fullframebuffer disjoint from all readonly map/
fixed objects. Resource mapping translates physical spans,not arbitrary pointers.
ACPI reader wrapper must reject actual translated reads into these mutable spans
before cs_native_read;large declared identity window itself is NOT such a read.
Call existing map/owned APIs only after raw preflight;require exact bundle owned
span and real handoff/exit/native-ready checks. FADT has8042,initial PM read valid
before scene/controller. The trace magic/version/size/offsets in SPEC are frozen.
Clock sampling must clear session current receipt before hardware callback. On
bad timer sample,session_poll(clock_ok0) establishes terminal before trace return.
No stale publication rows on NONE/clock/queue/controller. Do not infer freshness
or completion from saturating count differences. Optional observer can request
explicit stop;NULL means continuous healthy execution,no60s or Escape deadline.
Start config uses local scalar stores;canvas/payload/row partitions disjoint,
state never copied after binding. Trace retains session result/cause separately
from wrapper cause;startup may fail before owner/bus initialization,so snapshot
must not read uninitialized owner/cursor/bus fields. Zero new session storage by
explicit byte loop before start (no libc) to make all unavailable fields defined.
No VM/emulator/reference/hardware run or release is authorized. No UART telemetry.

```

```text
ClasSICK0.97,Windows11,supervised Claude primary. Codex sole repo writer.
Implement ONLY platform/uefi/window.c from frozen original public SPEC/API below.
Return JSON {files:{"platform/uefi/window.c":"complete source"},handoff:{result,
actionable_findings,tests_executed:[],blockers,decisions_needed}}. <=400 C lines,
handoff<=120words. No tests/docs/build files/header changes. No tools/reads/commands/
edits/web/MCP/subagents/manuals/HQ/untracked originals/UART/proprietary inputs.
All inputs inline through EOF. Original C11,GPL-3.0-or-later,Copyright(C)2026 Dean
Howell. Strict -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion.
x86-64 only,freestanding,no libc/memcpy/memset,allocation/global state or firmware
calls. Scalar field stores/copies to avoid O0 compiler helpers. Native stack frame
must fit2048B;session itself in caller arena,not on stack. One cs_arena_alloc.
Use explicit byte LE stores;no packed structs or type punning. Validate full
numeric ranges and aliases before writes;objects truthful/aligned by caller.
Gate result may write validated160B trace only;raw argument refusal writes none.
Native h/binding/devices/memory/keyboard readonly descriptors can share each other;
mutable arena/fulltraceallocation/fullframebuffer disjoint from all readonly map/
fixed objects. Resource mapping translates physical spans,not arbitrary pointers.
ACPI reader wrapper must reject actual translated reads into these mutable spans
before cs_native_read;large declared identity window itself is NOT such a read.
Call existing map/owned APIs only after raw preflight;require exact bundle owned
span and real handoff/exit/native-ready checks. FADT has8042,initial PM read valid
before scene/controller. The trace magic/version/size/offsets in SPEC are frozen.
Clock sampling must clear session current receipt before hardware callback. On
bad timer sample,session_poll(clock_ok0) establishes terminal before trace return.
No stale publication rows on NONE/clock/queue/controller. Do not infer freshness
or completion from saturating count differences. Optional observer can request
explicit stop;NULL means continuous healthy execution,no60s or Escape deadline.
Start config uses local scalar stores;canvas/payload/row partitions disjoint,
state never copied after binding. Trace retains session result/cause separately
from wrapper cause;startup may fail before owner/bus initialization,so snapshot
must not read uninitialized owner/cursor/bus fields. Zero new session storage by
explicit byte loop before start (no libc) to make all unavailable fields defined.
No VM/emulator/reference/hardware run or release is authorized. No UART telemetry.

Corrected call after concrete missing-header refusal. All required type definitions
now supplied;header comments/blank lines omitted for compactness,source identities
retained. ABI excerpt1..24 supplies only native macro/status facts;the actual
compiler sees its complete existing include tree. cs_native_read takes a
cs_native_memory* as its void* context,as native.h states. ACPI8042 discovery uses
the reader/memory/FADT API,not keyboard I/O. Trace bus result is s->bus_result;
do not invent bus.stage or init marker. Zero session bytes before start ensures
unavailable fields are defined. No new source/spec/API changes authorized.
Return only requested <=400-line source JSON and <=120-word handoff.

```

First call15.52s missing-type blocker,no code;one corrected176.84s call returns355 lines. Zero tools/writes/tests. Lead fixes/actual results recorded separately.
