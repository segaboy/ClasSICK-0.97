# ADR-0041 - First loaded rectangle calls

Accepted 2026-10-16; SRC-0202..0206, SPEC-0036, IMPL-0040, TEST-0044.

After the first experimental ROM release, the next visible runtime milestone
is drawing caused by our loaded68k application. The existing owner/interpreter
path deliberately refuses all A-lines; installing a general dispatcher would
exceed the reviewed fixed-root profile. Introduce an explicit fixed-port owner
entry for two canonical rectangle calls, with complete preflight before effects.
Ordinary cs_image_run and guarded CPU behavior retain their existing refusals.

Use original native128x64 mono port storage, black patCopy/white background and
rectangular clipping, with no guest GrafPort layout or private region encoding.
These selected cases follow published1985 routine contracts, not a January1984
identity or full QuickDraw claim. Later port state, callbacks, recording, patch
and Get/Set/direct-call equivalence need their own manager/typed-binding work.
Refuse installed directory patches before entry rather than bypass their meaning.

Snapshot Rect bytes before saving exception scratch, including overlapping
guest stack spans. The full native port and persistent trap scratch join the
existing disjoint ownership envelope. No guest-selected host address/handler;
successful earlier drawing survives later stops and explicit image retirement.
Render those actual retained pixels, not a host-generated substitute for the
application result. Keep source/host/compiler/actual boot observations distinct.

One Claude artifact implements the bounded byte renderer; Codex authors the
independent per-pixel oracle, actual loaded guest program, coordinator and tests.
One Grok packet check targets ownership/partial effects only. The old published
alpha and its tag remain unchanged. No new release or trial is authorized.
Existing source/native-app/UART/historical/physical/full128K/parity/edition/B2/
human and both System1/ROM requirements persist. [Contract](../specifications/SPEC-0036-loaded-rectangle-calls.md).
