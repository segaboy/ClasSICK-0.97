# TEST-0073 - UEFI window binding,trace and candidate verification

2026-10-22;SPEC-0062;IMPL-0064;SRC-0357/0358.
348 final profile CTests;8 object audits;4 compiled faulty controls rejected;
4 EFI images with twin equality,27 corruptions/26 omitted-object refusals;
two-writer boot-media equality and independent payload inspection. All images
unbooted. Fault seams are dependency-result models,not executed partial MMIO rows.
Most assertions repeat clock-stall/padding checks;34/39 scenarios per64-bit profile.
Lead tests begin before worker implementation,then extend after integration;
not a strict final author-blind oracle. Existing TEST-0070 plan stays unchanged.

|TEST-0070 case|Actual scope here;E1 only|
|---|---|
|WB-01|E1 valid/failed exit and readiness;gate effects checked|
|WB-02|Partial:owned kind and readonly-arena alias;not every malformed capacity case|
|WB-03|E1 absent8042 and protected ACPI translated read|
|WB-04|Partial:small width/format;other geometry reused leaf evidence|
|WB-05|Partial:640 initial margin cell/padding;full pixel oracle remains SPEC-0060|
|WB-06|Partial:real updates/final counts;full margin oracle reused|
|WB-07|E1 first post-cancel frame before controller writes|
|WB-08|E1 unknown first down suppressed|
|WB-09|E1 actual release then title/body presses|
|WB-10|E1 UI2 vs legacy bus3 and motion/drag state|
|WB-11|Leaf-only identical-report coverage;no new full-binding saturation probe|
|WB-12|E1 corrupt AUX cancels drag and preserves position/pattern/count|
|WB-13|E1 idle NONE neither redraws nor advances UI|
|WB-14|Leaf-only saturating-counter receipt coverage|
|WB-15|E1 four actual Space/Escape queue events;no click/stop|
|WB-16|Existing timed/continuous regression tests and normal CI;no target run|
|WB-17|Leaf-only exact8 pop-cap injection|
|WB-18|Injected session queue-result/receipt propagation;real queue refusal reused|
|WB-19|E1 timer invalid/stall/controller fault and pre-read receipt clearing|
|WB-20|Leaf-only accepted-OK0 oracle;wrapper delegates without filtering|
|WB-21|E1 representative NULL/wrap/alias/raw-no-effect cases;not exhaustive|
|WB-22|Injected owner-prefix trace;real owner fault tests reused|
|WB-23|Injected partial-report/device prefix;real bridge fault oracle reused|
|WB-24|E1 receipt reset and prolonged NONE;identical fresh-report leaf oracle reused|
|WB-25|E1 exact arena_used plus one-allocation source closure|
|WB-26|E1 conflict switches/macros;legacy builds inspected in normal CI|
|WB-27|E1 healthy62s with metadata1/60/3600|
|WB-28|E1 distinct160-byte record/reader;entry excludes old UART call|
|WB-29|Partial:BGR640 here;RGB/varied pitches in SPEC-0060|
|WB-30|Partial:readonly-arena alias;other leaf/span cases reused|
|WB-31|Injected wrapper progress only;actual row refusal covered by SPEC-0060|
|WB-32|Injected startup cancel/presentation refusal propagation;real startup gates in SPEC-0061|

Partial/reused rows are not new complete full-binding passes. No E2device/E3GUI/E4human,Mac or BIOS qualification. [Exact results](../../docs/development/pc-window-boot-evidence.json).
