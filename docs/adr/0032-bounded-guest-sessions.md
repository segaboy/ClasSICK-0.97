# ADR-0032 - Reuse bounded calls for an owned guest session

Accepted, 2026-10-13. SRC-0132/0133 / SPEC-0028.

The returning-call service already bounds stack, token completion, service depth
and ancestor instruction attempts. Application execution also needs a distinct
session lifetime and explicit stop. Reuse that service behind a separate owned
session API and trusted root-step exit request. Restore the complete caller frame
on every termination and expose a separate guest-stop snapshot; preserve completed
guest effects rather than implying heap rollback or teardown.

The session occupies the first returning-call depth slot. Its callback entry uses
the same executor and ancestor budgets. No-return exits inside returning callbacks
are refused until typed completion propagation is contracted. The default CPU
instruction and existing call/patch APIs remain unchanged. Host callbacks and
exit requests are trusted native OS interfaces, never guest-derived pointers.

Actual ExitToShell/Toolbox marshalling, continuation dispatch, loader and heap
reset remain subsequent work. The owner's next-task authorization covers this
bounded hosted OS/core slice and original checks, not an emulator/VM trial or
opening the native-application gate. Historical questions stay open.
