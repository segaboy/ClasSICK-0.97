# ADR-0054 - Explicit native rectangle outlines

Accepted2026-10-18; SPEC-0048; SRC-0280/0282/0284. Engineering contract only.
Use one prospective portable helper over the current native view: explicit two-axis
stroke sizes, internal set-union coverage before clipping, black OR/set, and one
saturating paint count per valid call, including empty calls. Existing port/view
layouts, entry points and candidate inputs stay unchanged. Historical FrameRect,
guest ABI, global pen state/patterns/modes and new scene integration are deferred.
No runtime code, header, fixture, executed outline test, build, target trial or
release is added. [Contract](../specifications/SPEC-0048-native-rectangle-outline.md),
[lead review](../development/native-outline-lead-review.md).
