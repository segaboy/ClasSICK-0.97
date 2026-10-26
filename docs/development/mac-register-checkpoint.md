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

## Verified native follow-up -SRC-0478

Corrected source c40cd296ad538a7ac8b1d8b7d1b9082c46405458 passes both jobs of
CI40327277294. The Linux decoder step and normal Windows checks pass. Lead
reidentifies five64KiB images,43 committed inputs/seals/twins each. Normal and
finite-defer baseline hashes remain673a2cca...c4965/cfa16dc2...4235. All three
checkpoint profiles contain the exact72-byte original leaf with expected
own-RAM operands and spin entry+70;21 resealed native controls plus8 ordinary
corruptions each pass in CI. Lead separately detects17 resealed body defects
per profile (51 total). No extra checkpoint BSS/workspace;normal304BSS and
deferred312BSS remain. New idle ROM uses23184 bytes (+88 vs normal);strict
first-input23244;deferred first-input23720. Stack bounds4996/5000/5008 are
compiler bounds,not measurements. The assembly correction is now verified.

Two bounded original-ROM trials successfully expose MWR1 at own PC00400322,
SR2714 (IPL7),trace address00000B70,pointer256,171 and fault0. The idle
checkpoint reports500 services/zero losses/zero deliveries;first-input warmup
reports5000/zero/zero. The first record is saved directly from read-only AX;
the second is explicitly transcribed from the successful initial AX result.
A later attempted AX save occurs after close and fails;no raw AX file is
claimed for that second observation. Both ROM hashes remain unchanged and
owned processes close (76.81s stop request;180.45s wall-cap cleanup). No
guest-state patch or semantic shutdown.

The second trial's approved helper passes its zero-event guard and reports
one delivered host move. Its recorded full-client point is218,22;targeting
the guest image is not established. It is an empty/warmup control,not a guest
motion test. A full-window ownership guard cannot substitute for confirming
the guest viewport. Do not infer hardware/input failure or success from that
event count,or repeat this setup as a qualified movement test.

The new diagnostic read path works. It has not fixed native mouse behavior.
Normal Mac/PC human usability checks remain failed;the earlier phase/defer
physical check was not performed because its window had closed. No E3/E4,
release freeze/publication,physical-hardware,System1 or native-app waiver.
68 originalPNG/two priorGIF remain;no successful-drag animation is claimed.
Next:ensure visible cursor/verified guest-viewport targeting before another
movement test,and use this original register path for actual sampling evidence.
