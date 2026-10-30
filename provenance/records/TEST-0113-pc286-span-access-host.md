# TEST-0113 - 286 checked-access host qualification

2026-10-30; SRC-0564. Public source baseline 40909e06b84992a077aac2547a7ffe21e3027a8a; r4 intake ZIP SHA-256 0a9d5881a443911bb1ff76ce4d0cc5ae89e29ad515acaf1b09d007c135b67d24. Seven original proposal inputs (size and SHA-256):

- span-access.c: 3634, a2f4df24484c343992c6f91ecda52280b01cd87a68087fecdf743c77f867b749
- span-access.h: 1126, 48009516eea12d4d26b568483f9e317b214d4168747babaacf4fafc3301d07f4
- span-bind.c: 2239, 1572b74fa88ae90df97aa463d33421f20ac81557cf4be7a031126191878acdb7
- span-bind.h: 3001, b263921ed282fa278cb4c403b1c90e495c82356f0942b761406280a543d0b36c
- pc286-span-access.c: 9494, 73ef302340ee74f3907e6c4f781b46427868248837bba2204561fa6ae49a1502
- pc286-span-access-oracle.c: 8857, f75a471d4501da3471772563565566a54a23439ecb7b0b0000b60d71c033cfa9
- pc286-span-access-parity.c: 5187, 6c45887081588888825ff3fb56b780a8ab9c7f33335421c8245e0e23ca8f1276

The seven scratch copies matched those originals after execution. Each compiler, GCC 16.2.0 and Clang 23.1.1, compiled the three tests with C99/O2 and warnings as errors. Both passed: 46 author literal cases; 200,000 author differential operations (26,696 accepted); and 300,000 author-generated comparisons against existing guest.c (39,961 accepted). The first scratch compile failed because an existing c16.h dependency was not copied; the header was added unchanged before the six successful runs. No author mutation driver was run. Original proposal sources are not integrated.

These host results are not native 286 compiler, ABI, link, execution, ownership, hardware, performance, PC/Mac input or release qualification. The author wrote the test generators and literal oracle. Refer to the [intake](../../docs/development/pc286-followup-intake.md) for dispositions and remaining prerequisites.
