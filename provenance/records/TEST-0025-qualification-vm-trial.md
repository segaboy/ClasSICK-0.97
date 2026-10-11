# TEST-0025 — Qualification-candidate VirtualBox observations

2026-10-11. SPEC-0015 isolation / SPEC-0016 diagnostics / ADR-0020;
owner SRC-0065. Status: measured qualification passed in four cold starts;
formal B2 partial on criterion 6. OpenAI Codex resumed the Windows/operator
lane from Claude's qualification handoff. Human provenance review pending.

Both clean repositories were fast-forwarded to the supplied heads: public
16dbab99789b61062087cfc6b03bd18a48bc4e91 and HQ
b43b3c7c9aa605be5eed68f66d3b3399c6397190. Scoped read-only review covered the
qualification contract, loader vendor-copy, keyboard probe/map gating and PS/2
state transitions/tests, entry interface and candidate admission. No blocker
was found in those inspected paths. Existing UART implementation review/role
history is preserved; no new UART algorithm/spec/fixture changes are made.
Pinned public CI 37930178034 at 7eed40a67b19f7bbf4471812136d8b69322420cd and
HQ CI 37931714514 at d0dbf37b90c57d589b0720e745d58f3c2c0562ef were independently
queried and confirmed completed/success. Those runs did not execute this image.

Planned admission: a fresh Verify-BootMedia.ps1 -Sanitizers root, 876 CTest
checks across nine configurations, O2/O0/raw identities from TEST-0024; a new
machine with a recorded raw/VDI/raw round trip and reviewed isolated settings.
Only the new project UUID may be mutated. Existing machines remain intact.

Three headless runs receive no generated keyboard input; a fourth visible run
requires separate owner confirmation of two Space presses and visible changes.
Each cold start gets fresh serial/capture/host-clock records. Qualification
requires both alias values PASS=0, both output ports read and bits 0–1 equal,
guest second 60 and result zero. UNAVAILABLE/unread is incomplete, never pass;
MACHINE=14 is a recorded failure. No firmware internals are a debugging input.
Criterion 6 remains open even if these measurements pass; vendor/revision are
only self-reported identity. No historical or physical/OS-edition gate closes.

The actual fresh Windows chain passes all 876 checks, the O2/O0/raw hashes and
all prior controls. Log SHA-256:
`6aab1918b1c501efa12afa71b03a6753bc42da07569822a7d4736253bc7914b1`.
The candidate preparation helper completes end to end; raw/VDI/raw agrees and
46 settings plus disk/controller/monitor/share/serial binding were reviewed.
The exact missing-PK refusal was retained with public SecureBoot=off; no key
enrollment or NVRAM inspection. No other preparation command failed.

Three no-input headless cold starts and one operator GUI cold start all show
port=00000003/00000003, a20=00000000/00000000, sec=0000003C and result=0/kbd=1.
No generated keyboard input occurs. The owner pressed several times, confirmed
visible changes, and the actual final Space count is six. This is live candidate
input acceptance, not an invented exact-two-count report. First GUI enumeration
was empty; unique window selection/activation succeeded on refresh.

RIP/RSP queries after READY place RIP in our loaded image and RSP within the
declared 64-KiB owned stack in every run. Firmware self-report is EDK II,
revision 00010000, complete; it is not a hash. The first run's owned addresses
differ from runs two/three, so complete serial byte identity is not claimed.
Qualification/frame/result sequences agree and headless final captures match.
Ready-to-terminal host arrivals are 59.995503/59.913701/60.058071/59.977745 s,
with retained actual poll brackets and stated serial/scheduling limitations.

[The evidence snapshot](../../docs/development/qualification-vm-evidence.md)
and JSON retain the bounded own output and sanitized command sequences. All
collectors completed; observer draft line-continuation errors were corrected
before any execution. Final VM state is powered off and VDI hash unchanged.
The SPEC-0012 measurement precondition now passes on this declared configuration.
Formal B2 still remains partial on unchanged criterion 6. Physical/edition/Mac
and human provenance gates remain independent. The observed publication CI result is recorded below.

Publication verification: [Windows CI 38180032136](https://github.com/segaboy/ClasSICK-0.97/actions/runs/38180032136)
passed at `8f330b8127ce7d29128a4b5da424ccf967ba50d8`: 296-file repository
guard, pinned setup, fresh bootstrap twins and the full nine-configuration
876-check hosted/unloaded/media chain. All 64 local/CI comparison labels and
58 distinct fingerprints match, including unchanged O2/O0/raw candidate hashes.
This CI did not run the VM, operator input or register observations. This
follow-up records observed results only.
