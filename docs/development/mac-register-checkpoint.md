# Original Mac checkpoint register export

2026-10-26;SPEC-0072/SRC-0476..0477/IMPL-0081/TEST-0094.
This makes an existing diagnostic snapshot readable without the emulator's
memory-address edit controls. It does not change the mouse producer,IRQ
policy,admission,recovery,first-input threshold,default image mode or release gates.

The existing idle/first-input diagnostic checkpoint now explicitly holds IPL7,
loads its copied original record into registers,and enters the separately
named cs_mac_window_idle_spin. It never returns;this is not guest shutdown.
The normal build must contain neither checkpoint symbol. No new CLI mode,
RAM object,MMIO access or guest-memory write is added by the checkpoint.
Native compilation,ROM cost/default-byte equality and execution are pending.

|Register|Original data|
|---|---|
|D0|MWR1 marker4D575231|
|D1|services|
|D2|losses|
|D3|deliveries|
|D4|first-loss reason|
|D5|first-loss RR3|
|D6|first-loss previous observation|
|D7|first-loss inbox phase|
|A0,A1,A2|first-loss snapshots0,1,2|
|A3,A4|signed32 pointer x,y|
|A5|diagnostic fault|
|A6|original trace address|

Every assembly data offset has a C static assertion. Address-register values
are data here;never follow them as memory addresses. The decoder accepts a
JSON object containing exactly D0..D7/A0..A6,each an eight-digit hexadecimal
string with optional0x prefix. It refuses duplicates,missing/extra fields,
bad types/width/digits and wrong marker. Only coordinates are sign-extended.
Run scripts/Read-MacRegisterTrace.py against the captured JSON file.

Passing the decoder only proves format validation. Lead must separately pin
the ROM/source/native audit,observe the exact own spin PC and IPL7,and classify
freshness/input provenance. Neither a fabricated format fixture nor arbitrary
register values constitute target evidence or successful mouse input.

One bounded Copilot Sonnet5.5 Low primary returns the two native replacements,
81-line decoder and73-line author tests in34.16s from17119 frozen public/original
bytes. One view,no worker writes/commands/tests;one reported premium request.
Lead fixes copyright,asserts the other two array offsets,adds the independent
literal full mapping and10 actual CLI cases,and integrates an exact72-byte
native instruction audit with17 resealed corruption sites. Native controls
are wired,not yet executed. No second worker or retry.

Actual host results:seven unittest groups and focused CTest pass;three faulty
decoders (swapped snapshots,unsigned coordinates,ignored marker) are detected.
An initial CTest TEMP permission error is retained in the private log;rerun
with owned TEMP/TMP passes. All four provider balances remain unavailable;
the reported charge is not a quota balance or measured Codex saving.
[Machine receipt](mac-register-checkpoint.json).

First native attempt,d100c88/CI40326299729:assembler rejects the short branch to the exported global spin symbol (out-of-range byte-field value). Lead uses a local numeric self-branch at the same exported location;the native audit still requires exact60fe. No target ran;fixed native proof remains pending.
