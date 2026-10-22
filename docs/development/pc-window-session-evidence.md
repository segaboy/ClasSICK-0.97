# Firmware-neutral PC window session - actual evidence

2026-10-22;SPEC-0061/ADR-0070;IMPL-0063/TEST-0072;SRC-0353..0355.

The native session joins the existing controller receipt, shared window owner and
Mono1 row presenter. Startup binds a hidden transparent plus, begins and cancels
the owner, publishes once, then initializes the controller. Polling clears its
receipt before clock checks, polls once, drains at most8 keyboard events and
dispatches PACKET/LOST exactly once. Accepted unchanged owner results still
publish. Fatal paths retain their prefix; stop quiesces without implicit detach.

- 323 final Clang64 regression CTests and3 sanitized/3 GCC32 focused tests pass
  (329 profile CTests). Ten Clang/GCC32/64 O0/O2 and ARM little/big-endian object
  audits admit only named project imports and reject mutable global data.
- Per Clang64/sanitized profile:30 normal scenarios/257 checks/11 complete
  presentations;43 fault scenarios/4000732 checks/36 complete presentations.
  GCC32:29/254 and42/4000729 respectively,with the same presentation counts.
  Most fault assertions exercise the1000000-turn bounded clock-stall detector;
  they are not millions of independent scenarios.
- Four compiled defective controls fail:stale startup receipt,NONE fabricating
  publication,unchanged owner result skipping publication,and exclusive endpoint
  wrap. The identical fail-fast assertion harness passes the unmodified control.
  Initial ordinary continue-on-failure mutation run exceeded15s because broken
  freshness caused repeated full publications;no runtime timeout is counted as
  one of the four successful defect rejections.
- Real dependencies run against original in-memory framebuffer/port transcripts:
  unknown startup down suppression,real release,title drag,keyboard Space/Escape
  drain and corrupt AUX cancellation. The transcript installs the already-tested
  controller READY state;it does not newly execute its initialization handshake.
  Renamed-import seams separately assert startup/order,queue caps/faults,owner
  refusal,partial-publication diagnostics,retained prefix and terminal inactivity.

One Claude Opus5.5 low primary receives34035 frozen public/original bytes690 lines.
At420.09s the call caps without complete JSON,handoff or usage/cost receipt.
Partial stream yields a complete272-line implementation and375-line truncated
test draft,647 combined lines exceeding600 cap. No tools/writes/tests observed;
no retry,second worker or requalification. Codex adopts the usable code,finishes
the test ending and adds concrete gap checks. Lead fixes exclusive endpoint,
cursor mask and a GCC32 type-limits warning;also corrects its own new test's
counter-field name before passing verification. Primary tests share their author
with production;reuse existing independent component oracles,not a new claim of
independence. Timeout does not prove backend cancellation or zero spending.

Session storage is984 bytes LP64/888 bytes ILP32 in these profiles. Together with
21888 canvas+48 payload+32768 maximum row,the corresponding subtotal is55688/
55592 bytes;art is already embedded. It excludes config,stack,trace,other kernel
state and image. Compiler stack files are per-function diagnostics,not whole
kernel/ROM/128K certification.

[Exact receipts and input identities](pc-window-session-evidence.json);
[actual worker assignment](tasks/pc-window-session-worker.md).
Raw captures/results:C:/ClasSICK/window-session-20261022-a,outside Git.
No fresh external account allowance snapshot or measured net Codex saving.
Future primary artifacts should fit a smaller450-line deliverable;do not repeat
this combined scope just to retry. Grok,Copilot,Argon idle.

No UEFI/ROM source list,boot binding,image,device ABI,UART,release or target run
changed. TEST-0070 full-binding cases stay planned,not executed by this leaf.
24 originals,held brief,old records/assets/tags and retired wiki are preserved.
Mac M1-M7,PC GUI/human/CPUID,source/role/historical/edition/physical/full128K/B2/
native-app/Linux/mini-vMac/both System1 directions persist. Next:opt-in UEFI
window binding with the owned arena and versioned trace,then separately gated
candidate/trial qualification. This result advances paired-native-preview-2;
it does not supply missing Mac boot work or authorize publishing a release.
