# Native PC fault checkpoint - 2026-10-12

SPEC-0019 / SPEC-0008 v2 / ADR-0022 / IMPL-0023 / TEST-0027;
owner SRC-0085, original artifacts SRC-0086, observations SRC-0087.

**The bounded checkpoint passes:** three invalid-instruction (#UD) cold starts,
three invalid-selector (#GP) cold starts and one normal control. On the exact
cleared VirtualBox platform, our own handler captures both frame types and stops
on its separate emergency stack. The normal image completes on its ordinary stack.
Formal B2 remains partial solely on unchanged criterion 6's firmware image hash.

[Sanitized records](fault-checkpoint-evidence.json) retain all own serial events,
selected terminal registers, image/media fingerprints and timings. Local full
logs, UUIDs, VM inventory, private paths and pre-banner serial bytes are excluded.
No guest memory/code, CR2, emulator implementation or firmware internals read.

## Admission and build checks

Compiled/source and observer revision: `2f64689521fe57866b7a70572f2f3bb8a7bbb008`.
[CI 38250103771](https://github.com/segaboy/ClasSICK-0.97/actions/runs/38250103771)
passes both jobs. All six O0/O2 normal/UD/GP EFI fingerprints match local builds;
CI performs hosted/unloaded checks, not VM observations.

Fresh Windows verification passes 876 CTest checks in nine configurations,
including x64 sanitizers and unchanged prior device/core tests. Three classifier
profiles and sixteen invalid-run controls pass. Each EFI profile has four builds
and deterministic twins per optimization: normal rejects 25 corruptions, each
injection 26 plus normal-profile misclassification; ten omitted objects reject
per profile. Exact audits retain all 256 stubs and check record capture,
publication, register exports and both complete/nested terminal destinations.

Nine normal media writers produce one original 64-MiB GPT/FAT32 disk; thirty
media corruption controls and three writer refusals pass. Both injected disks
also pass the independent checker. All three O2 payloads are 43,520 bytes,
loaded image extent 57,344 bytes. No imports or target runtime libraries.

A single new project VM is created and its before/after inventory differs only
by that identity. Exact VBoxManage version 7.2.16r174877 and SHA-256 match the
previous clearance. One CPU, EFI64/Secure Boot off, owned fixed disk only,
no network/audio/USB/shares/clipboard/drag/drop/remote display; settings and
bindings are reviewed before every launch. Each RAW-to-fixed-VDI-to-RAW round
trip reproduces its raw disk hash. Swap only owned media while powered off.
No prior VM/media modified or deleted. All three VDIs remain unchanged.

## Seven observed cold starts

Every run reaches keyboard READY, qualification port 03/03 and both bit-20 alias
probes PASS, guest second 60 and `result=00000000 kbd=00000001` before any CPU
query. No generated input or new live-operator claim. Each launch begins powered
off; no reset/resume/saved-state run is counted.

| Run | Profile | Captured outcome | Ready-to-result host arrival difference | Result |
| --- | --- | --- | --- | --- |
| cold-01 | ud | vector 6, error 0x0, IST1 | 60.014169 s | passed |
| cold-02 | gp | vector 13, error 0x28, IST1 | 59.961792 s | passed |
| cold-03 | ud | vector 6, error 0x0, IST1 | 59.932627 s | passed |
| cold-04 | gp | vector 13, error 0x28, IST1 | 59.991990 s | passed |
| cold-05 | ud | vector 6, error 0x0, IST1 | 59.992535 s | passed |
| cold-06 | gp | vector 13, error 0x28, IST1 | 59.960546 s | passed |
| cold-07 | none | ordinary stack | 59.933206 s | passed |

UD uses real UD2, with normalized zero error; GP loads DS with selector 0x28
outside our 39-byte GDT limit and receives CPU error 0x28. Saved RIP equals the
owned image base plus the audited offending-instruction RVA in every fault run.
No software INT is used as an error-frame substitute.

Two delayed observations per run have identical selected registers. Fault runs
show published state 2, marker 0x43534631, owned state pointer at bundle+495616,
saved CS=8/SS=16/IF clear, old RSP in the ordinary 64-KiB stack, and live RSP at
IST1 top minus seven qwords (56 bytes). Live RIP stays at the owned halt loop.
Normal control stays in that loop on its ordinary stack. Captured values are
loaded from our first-fault record; these are not interrupted GPR snapshots.
The handler makes no calls, device accesses, firmware calls or return.

Nominal 100-ms host serial polling uses a monotonic clock and UTC. Arrival
differences include buffering/scheduling and do not calibrate the guest clock.
The first boot also incurs initial EFI setup delay; admission timing is not a
fault-handler latency measurement. Each VM run is powered off after observation;
full local serial/command/observation records remain available.

The host batch command used an invalid trailing redirection after its six
sequential observers had all returned successfully; that outer command exited 1.
Individual records/management logs retain all seven passes, successful power-offs
and unchanged disks. The wrapper error is retained separately; it is not a guest
fault-test failure. Final public state/inventory checks confirm powered off and
only the newly created identity added. No native rerun was needed for logging.

## Boundaries and next work

Only these two vectors and IST1 on this platform are observed. NMI, double faults,
nested-fault runtime behavior, other vectors, recovery, all PCs and physical
hardware are unverified. Terminal faults intentionally stop rather than recover.
The firmware container/vendor/revision still cannot replace an image fingerprint;
SRC-0059's no-extraction boundary and formal B2 criterion 6 remain unchanged.

UART source/spec/test bodies were not edited or reviewed by the coordinator;
its existing contract and aggregate verification are reused. No Apple ROM/code,
assets, uncertain discovery excerpts or emulator source used. Human provenance
review remains pending; no legal/independently staffed clean-room certification.

The original Mac ROM result remains intact. Next functional work is the shared
trap/service gateway, starting with review of SPEC-0017's historical questions.
Full System 0.97 services, Mac/mini vMac/physical validation, complete budgets,
original/native parity and all three complete OS editions remain open.
