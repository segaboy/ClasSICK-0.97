# TEST-0070 - PC window boot-binding planned cases

2026-10-22;SPEC-0059;SRC-0346..0348. All PLANNED/UNEXECUTED;no fixtures or code generated. Worker cases tightened by lead;32 case identifiers are not32 passing tests. Applicable results require future approved implementation/native interfaces.

- WB-01 not exited or status nonzero -> validated diagnosis only,no owner/controller/framebuffer writes.
- WB-02 trigger: descriptors not ready. expect: stop before begin and before present.
- WB-03 trigger: bad map, missing RSDP, PM timer, or 8042. expect: no owner and no publication.
- WB-04 trigger: width<512 or height<342 or row>32768. expect: target refusal, canvas unused.
- WB-05 first successful publication at even/odd centered dimensions -> one full physical row at a time,white margins,unscaled canvas,reserved byte0,padding/tail untouched,no framebuffer reads.
- WB-06 trigger: later full-canvas present. expect: margins remain white; only logical rows change.
- WB-07 startup with no mouse traffic -> begin then one cancel then ONE first publication,count0,suppression set;no controller poll before success.
- WB-08 trigger: first PACKET with LEFT held. expect: no press increment; suppress holds; motion still delivered once.
- WB-09 trigger: observed LEFT release then a new press. expect: release counts nothing; later press counts once.
- WB-10 trigger: PACKET. expect: one cs_pc_window_packet; UI shows owner presses only.
- WB-11 trigger: identical PACKET. expect: one delivery, not dropped; no extra count beyond that edge.
- WB-12 trigger: LOST. expect: one cancel; count, pattern, and position kept; drag ends; suppress until release.
- WB-13 trigger: NONE. expect: no owner call and no publish.
- WB-14 trigger: saturated bus packet or error totals. expect: delivery still follows this receipt only.
- WB-15 trigger: Space or Escape in this profile. expect: no synthetic click and no session end.
- WB-16 Escape in an existing timed profile -> old exit behavior;existing continuous outline ignores Escape and retains no60s deadline.
- WB-17 exactly8 valid queued keys -> at most8 pop calls and no mouse synthesis;do not invent a ninth valid queued record in capacity8.
- WB-18 queue error after poll but before mouse dispatch -> terminal;no current-turn owner call;earlier accepted state retained,diagnosis allowed.
- WB-19 clock/stall fault before poll OR controller terminal -> local current-turn receipt NONE/zero before clock,terminal receipt never consumed,no owner call;prior turns retained.
- WB-20 trigger: accepted OK0 that changes cursor pixels. expect: one coherent canvas publish.
- WB-21 raw bridge/session argument preflight NULL,wrap or alias -> no caller-byte change,allocation,I/O or framebuffer write;owner refusal later may change session diagnosis only.
- WB-22 trigger: owner fault after a partial consumer prefix. expect: prefix retained, no success publish, no auto repair, later packets not applied.
- WB-23 trigger: present fails after RAM is complete. expect: presentation fault, not owner fault; partial MMIO kept; no rollback.
- WB-24 old PACKET bytes retained externally followed by a fresh NONE turn -> no owner replay;distinct identical fresh PACKET still delivers once.
- WB-25 reports after one arena plan -> no additional allocation/reset;product.pixels=NULL,54704 max fixed buffers plus state/alignment/art costs.
- WB-26 trigger: selected or outline built instead. expect: window session not entered; those profiles unchanged.
- WB-27 trigger: this profile passes a 60s metadata value. expect: no hidden deadline stop while healthy.
- WB-28 window path entry -> old UART bodies/observer unchanged;dedicated trace is not parsed as old UART/keyboard output.

- WB-29 RGB and BGR targets with varied pitch -> exact black/white channels and reserved0;all visible margins and row padding/tail checks match the contract.
- WB-30 mutable/read-only graph aliases or insufficient row/canvas/arena capacity -> preflight refusal before owner,controller or publication;no fit claim from buffer arithmetic alone.
- WB-31 injected row-presenter refusal after k completed rows -> exact partial-publication progress,no complete-frame success,no rollback;arbitrary MMIO exception is a separate unknown outcome.
- WB-32 initial cancel or first publication failure -> retain reached prefix;do not enable controller or publish success;later terminal calls do no I/O or replay.

Planned oracle inputs are original logical samples/Mono bits and RAM-backed
volatile-target views,not emulator/device observations. Freeze interfaces first;
independent test author does not receive new implementation. E1 cases never prove
E2 device direction,E3 capture/GUI,E4 human,Mac hardware,ROM/128K or BIOS support.
