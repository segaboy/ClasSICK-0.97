# SPEC-0046 Drawing through the selected prepared port

Accepted engineering profile,2026-10-18; owner "Continue with the next task."
extends SPEC-0045 with one explicit opt-in entry,without a new global manager.
Same original native C on68000/x86; interpreter executes legacy applications.
No target/reference/hardware run,release or historical behavior claim.

Frozen decisions:
1. cs_image_run_selection_draw uses the existing request/workspace/report and
   PRF2 (0x50524632). Existing cs_image_run_selection remains PRF1 selection-only
   and still rejects PaintRect/EraseRect. Common retirement accepts both fully
   validated owner profiles. Profile equality is checked against the root-local
   admitted mode every instruction; a recognized tag alone never grants access.
2. PRF2 supports exact A873/A874 and A8A2/A8A3 only,each P=4. All four directory
   slots must be unsupported at admission and before every supported service.
   PRF1 retains its two-slot rule. Unsupported variants/patches stop pre-effect.
3. Every existing complete native envelope,one-use generation,owner identity,
   request/config snapshot,seal,current/table/record/view integrity,lifetime,
   stop/fault,quarantine and protected RELEASE rule remains. No native pointers
   persist past proven quiescence. Unresolved borrows remain live and inert.
4. For drawing,re-read current from guest g after the ordinary owner checks.
   Validate the entire selection table; current NIL/unknown is a guest policy
   refusal before pixels/counters/scratch/frame. Resolve its actual prepared
   native view; apply its offset and physical rectangular clip via SPEC-0041.
   Both ports remain disjoint borrowed native storage; no guest GrafPort parsing.
5. Drawing parameter is an even guest address of eight BE signed16 Rect bytes.
   Entire span must be owned below-A5 or owned stack ending at/before root;
   below-A5 span excludes the206-byte reservation and every108-byte record.
   No header,above-A5,code,heap,other RAM,odd or high-byte address is admitted.
   Snapshot all8 bytes BEFORE scratch/native writes; overlap with consumed
   parameter or exception scratch is allowed when it satisfies these spans.
6. Validate original SR,A5,SP,parameter and RX continuation; pin six scratch
   bytes before calling the native view helper. Stage next frame and scratch.
   Helper refusal is pre-effect; after a successful helper only total scratch
   SR/PC stores and frame PC+=2,A7+=4 remain. Drawing leaves current and all
   other guest bytes unchanged except scratch; registers/CCR/caller preserved.
   Each successful draw bumps its selected port's saturating operation counter
   exactly once,including empty/inverted/fully clipped rectangles. Services and
   last_word include actual completed Get/Set/Paint/Erase calls.
7. Original views/capability immutable throughout root; pixels/counters are
   intentional output,not control hashes. Use an owner-made root-local view
   snapshot for drawing,after equality checks. Unexpected helper refusal after
   validation quarantines as BINDING before scratch/frame stores.
8. Workspace adds rect_bytes[8]; no pixel snapshot/rollback,allocation,callbacks,
   libc,mutable globals,private structures or new global dispatch records.
   Preserve code-run.c and old default/fixed/selection semantics/workloads.
   Compile/link checks do not imply the updated code booted on either target.

Independent oracle must check actual pixels of both ports,offset/clip and fresh
direct guest current writes; copied Rect/scratch alias; empty/saturation;
NIL/unknown/odd/outside/record/reservation spans; patched four-slot admission;
unknown variants; PRF1 refusal; full binding validation; retained earlier pixels
on later fault; actual cleanup/one-use lifetime. Proposed cases are not executed.
