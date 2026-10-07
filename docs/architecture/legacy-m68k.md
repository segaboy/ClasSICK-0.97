# Legacy Motorola 68000 application strategy

Status: staged design. No CPU interpreter, Apple application, ROM, or executable
reference payload is included.

## Native OS, optional guest execution

On x86-64/ARM64, ClasSICK core and personality compile to native instructions.
Only legacy application instruction streams enter an optional 68000 interpreter.
There is no emulated Macintosh motherboard beneath the OS. Hardware drivers,
graphics, filesystems, manager logic, and scheduling remain native.

On Motorola 68000, supported guest instructions can execute directly. An
independently implemented exception/trap boundary adapts guest register/stack
conventions to our native C APIs. Direct execution is not an assurance of memory
protection or arbitrary application compatibility on hardware without an MMU.

## Guest services

- A bounded guest address space translates addresses to native storage, with
  endian-aware reads/writes and defined access/alignment exceptions.
- A register/condition-code and exception model targets the **original 68000**
  instruction set; later CPU behavior is not silently substituted.
- Instruction fetch/step/cycle-accounting policy is independent of device timing.
  Instructions and exceptions require published ISA evidence before implementation.
- A-line decoding enters the personality's trap dispatcher. Trap patching,
  callbacks, stack discipline, register preservation, and reentrancy are separate
  contracts. Do not equate all A-line encodings with interchangeable API calls.
- Segmented CODE resources, jump-table/entry conventions, low-memory views,
  handles, and return paths require exact-version behavioral specifications.
- Guest code receives no raw host pointer or direct Win32/firmware service access.
  Instruction budgets and explicit interruption prevent a guest loop blocking
  hosted testing indefinitely.

## Implementation stages

1. Approve an eligible ISA manual and synthetic instruction conformance corpus.
2. Implement the guest memory contract and interpreter state independently.
3. Execute independently authored assembly probes and trap/callback tests.
4. Load an independently authored synthetic resource-fork application.
5. Run a named legally supplied original application locally; publish only safe
   behavior reports and independently authored probes/tests.
6. Consider a JIT only after interpreter correctness, invalidation semantics, and
   workload evidence justify it. JIT introduction requires a new ADR.

A third-party CPU engine is a possible later alternative, subject to license,
complete provenance, 68000 accuracy, trap hooks, host support, and integration
review. Public availability alone is insufficient. No engine is adopted now.

## Compatibility limits to measure

Supervisor instructions, direct device access, private ROM calls, guest code
patching, memory aliasing, 24-bit/address-mask details, exception frames, and
execution timing are explicit research questions. Do not guess them from host
compiler ABI behavior. The guest ABI can differ from our 32-bit-int native m68k
C ABI; changing global `int` width to mimic guest conventions is rejected
(SRC-0006 describes the compiler option and its ABI effects).
