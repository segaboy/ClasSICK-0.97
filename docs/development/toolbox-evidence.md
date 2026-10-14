# Typed Toolbox dispatcher checkpoint - 2026-10-14

SPEC-0029 / ADR-0033 / IMPL-0033 / TEST-0037 / SRC-0139-0141.
Compiled source `9b4a96a7d9d72cfdc83a1eb86ae8b4938f705736`; [CI](https://github.com/segaboy/ClasSICK-0.97/actions/runs/38543836785)
passes both jobs. Complete original68000 manifests match local/CI.
Lead self-review; human provenance review pending.

The original hosted dispatcher uses explicit per-routine convention/completion
records, checked mixed-width stack arguments, bounded strings/fixed spans and
staged result/output writes. Normal return and procedure auto-pop use checked
control frames. All fallible checks precede effects. Output destinations cannot
overlap executable memory or the owned stack. Actual A9F4 ExitToShell ends a
successful root session through SPEC-0028; caller restoration and retained prior
effects use the existing session contract. No Finder or heap teardown is implied.

Seven original hosted suites plus one freestanding check cover scalar encoding
(including all256 bool high bytes), declaration order, bounds/permissions/aliases,
atomic refusal, malformed records/replies, output scatter, observable gateway
patches, normal/auto-pop/root exit, callback exit refusal, fault/budget precedence,
restoration and reuse. The wholly original30-byte push/trap/pop/RTS probe returns
in eight attempts; its encoding is independently checked by the assembler.
Synthetic trap words test this OS/core boundary, not historical manager behavior.
Claude's HT/LO/launch/re-entry cases have not been executed or turned into fixtures.

The initial complete nine-profile1614 CTests pass. After the final permission
correction all72 affected checks repeat, with1542 unchanged results retained.
Ten final width/endian object audits and both compiler debug twins pass. GNU68000
fourteen module audits/two complete relocatable links pass without runtime imports
or mutable globals. Runtime/report/record/argument/input/reply storage is
68/28/34/20/104/24 bytes; the runtime embeds its28-byte report, so these are not
independent aggregate allocations. Closure compiler frame maxima are388/O0 and
376/Os. These are partial costs, not cumulative stack or final128K measurements.

CI runs the complete corrected-source1614 matrix plus384 bootstrap checks.
Six existing EFI profiles and both own ROM profiles/input fingerprints are
unchanged. Thirteen original research drafts and four concurrent arrivals remain
unchanged/untracked with byte-identical private copies. The concurrent arrivals
remain unreviewed. The thirteen-routine register interface registry is unchanged.
Preparation corrections and superseded audits are recorded in the JSON evidence.

Register Toolbox exceptions and LoadSeg continuation are typed refusals. Installed
Pascal patches, direct built-in veneers, retained pointers and manager callbacks
remain deferred. No manager/QuickDraw, loader, heap reset, native application/SDK,
default boot binding, VM/emulator/reference trial, UART body work, prohibited
code/assets, delegation, chats, automation or OneNote. Native owner, historical/
full OS/physical/final128K/parity/three-edition/B2criterion6 gates persist.
Next proposed contract: heap lifecycle/reset with staging, explicit commit/fault
semantics, scrap/reference ownership and zone epochs, before CODE-only launch.

[Machine-readable evidence](toolbox-evidence.json).
