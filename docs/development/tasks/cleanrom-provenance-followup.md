# Prompt for the original CLEANROM Claude conversation

Copy the complete prompt below into the conversation that produced the package.
This is a provenance request,not authorization to implement,rebuild or run it.

---

Continue your CLEANROM work with a provenance-only audit of the exact package
you delivered on2026-10-29. Our ClasSICK0.97 intake has verified these identities:

- CLEANROM.ROM:65536 bytes,SHA-256
  214fc980068a1b14e76d363b90d1681097edca258ca26bd7c28e694ccd01a970
- cleanrom-src.tar.gz:20247 bytes,SHA-256
  06e5c2445b0e2bf0ba4283090c672167b1b70987f5e991b45e586060b28a9e65
- CLEANROM-build-notes.pdf:439196 bytes,SHA-256
  0204d09c02a0d67371a86bab05bda8574b4a816d6f12d592b821e731ba9488c2

The separate ROM matches the one in the archive. Your report page2 declares
reading MAME and Mini vMac implementation source;page1 says the ROM had not been
booted and the test harness could not run. ClasSICK's rules exclude emulator
implementation source as well as Apple/Microsoft implementation,ROM/media code,
disassembly,manual sample algorithms and copied protected assets. A statement
that no Apple ROM was used does not resolve the additional source boundary.

Do not modify files,generate replacement code,rebuild,boot,run a test,fetch more
implementation sources or forward source bodies. Report from this conversation's
existing history. Treat missing history as unknown;do not reconstruct a cleaner
history from memory or substitute intended behavior for recorded actions.

Return a report of at most1200 words covering:

1. The original instruction boundaries and the prompts,source accesses and tools
   that materially shaped the deliverable. Identify source names,versions and
   already-recorded URLs where known,but do not quote implementation or reopen
   excluded files to fill gaps. Distinguish published manuals,interface facts,
   implementation-source reading,model knowledge and assumptions.
2. A file-level origin/dependency table for README.md,build.sh,src/rom.s,
   tools/gen_assets.py,tools/finish_rom.py,test/macemu.c and CLEANROM.ROM. Explain
   which files were influenced by excluded implementation reading. Do not paste
   code,derived hardware tables or source-specific implementation descriptions.
3. Whether any third-party source,generated third-party code or protected asset
   is included in the archive. Distinguish a declared external dependency from
   included code. State unknown provenance explicitly.
4. Asset authorship and derivation:how the font and cursor were originated,whether
   any existing bitmap,manual sample or code was copied/traced,and what licensing
   terms the owner can actually provide. Do not send images or bitmap arrays yet.
5. Exact build and test evidence already recorded:toolchain versions,commands
   actually executed,success/failure,artifact hashes and which assertions were
   observed. Separate assembly/linking,a prepared harness,an executed harness,
   emulator boot,physical boot and mouse/drag observations. No new runs.
6. Corrections required to the README/report,including any unsupported assertion
   about512K,Mini vMac,MAME,real hardware,clean-room status or working dragging.
7. Which contributions,if any,have a documented independent origin that can be
   assessed separately. Do not certify the rest or rewrite a specification to
   conceal its excluded source history. Fresh implementation requires a separate,
   unexposed role using independently reviewed eligible sources and owner approval.

Finish with a short list of missing evidence and decisions the project owner
must make. Research and worker statements are proposals until reviewed;this
audit grants no implementation,trial,release or source-policy exception.
