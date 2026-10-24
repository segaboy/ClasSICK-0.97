# Mac automatic idle diagnostic - bounded original implementation

Windows11, public baseline679dfb5. Codex sole writer/integrator/verifier.
One Copilot Sonnet5.5 low primary. Read this frozen packet only; no commands,
other reads, writes, web, MCP, delegation or tests. No manuals, HQ, untracked
research, Apple/MS code/media, emulator source or manual samples.

Deliver a small usable original patch to the three supplied public files.
Current GUI-driven pause contaminates a putative idle input observation. Add
--window-idle-probe, allowed ONLY with --window-drawing. Default normal ROM
must remain byte-identical. This diagnostic is observability, not an input fix.

Design: default CLASSICK_MAC_IDLE_PROBE=0, reject values other than 0/1.
With flag1 only, after observe() has copied its existing trace under the
existing foreground IPL7 exclusion, within !dead, if motion.services>=500,
call an original noreturn assembly symbol cs_mac_window_idle_checkpoint
that branches to itself with exact 60fe bytes. Keep IPL7 held, no MMIO or
quiesce/recovery, no new BSS, no record/layout or admission-policy changes.
No new fault/status value; this is a diagnostic halt, not session STOPPED.
Label must be absent in default ROM. No by-value tiny sample aggregates.
Service threshold is a minimum, not exactly500 or a precise time promise.
Put optional spin in window-irqs.S conditional on numeric macro, C extern
noreturn declaration similarly guarded. IRQ veneers remain exact/unchanged.

Builder: new boolean switch and refusal before any mkdir/compiler for all
non-window modes, matching -D macro only for the opted-in window build;
manifest window_idle_probe boolean and threshold500 when enabled, own symbol
address available through existing symbols. Extend native audit to require
60fe at exact own label when enabled, absence when disabled, and reject a
resealed single-byte spin corruption in addition to existing native controls.
Report actual native-control count5 for probe vs4 normal. Preserve legacy
modes and metadata. Do not add mandatory default code or RAM/trace fields.

Return JSON ONLY {replacements:[{path,old,new}],handoff:{result,tests_executed:[],
blockers:[],decisions_needed:[]}}. old strings must each occur exactly once.
<=180 new/changed lines. Proposals not executed tests. No extra design review,
new test framework, unrelated edits or historical/physical claims.
