# TEST-0079 - PC286 compile-only preparation evidence

2026-10-23;IMPL-0067/ADR-0077,SRC-0386..0389.
[Final receipt](../../docs/development/pc286-toolchain-evidence.json).

| Actual final group | Checks | Result |
| --- | ---: | --- |
| ABI/arithmetic/far/C99/C11 probes,3 memory models | 15 | 12 objects,3 expected C11 rejections |
| Lead ABI controls,3 models | 3 | 3 objects;compile-time narrow-mask hazard retained |
| Selected header ABI,3 models | 3 | 3 objects;size_t/uintptr widths recorded |
| Existing guest/surface/traps/m68k C units,3 models | 12 | 12 objects;helpers remain external references |
| False int/far assertions | 2 | Both rejected |
| Default stack-helper control | 1 | Object references __STK |
| Isolated Alignof and c11-mode attempt | 2 | Both rejected |
| Unoptimized far-call probe | 1 | Object/listing available;not runtime execution |
| Harness archive/output refusal controls | 2 | Refused wrong identity/existing output;no overwrite |

39 final compiler invocations,32 objects,7 expected rejections;2 separate harness
controls. Preliminary ad-hoc/first-harness runs are retained privately and not
counted as new distinct tests. First harness false-positive read a DATA label as
an instruction;restricting own-TEXT scanning fixes it before final39-case run.
All32 objects report80286/USE16 and pass selected listing guards,not an exhaustive
ISA validation. No linking,feature/runtime test,benchmark,286 execution,CI target
build,emulator/VM/hardware trial or Word compatibility pass. Lead ABI controls
predate reading primary artifacts;final integrated suite is not entirely author-blind.
Existing P286-V1 remains partial and no other original phase/gate closes.
