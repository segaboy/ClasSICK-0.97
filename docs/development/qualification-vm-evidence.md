# Qualification-candidate VM evidence — 2026-10-11

SPEC-0016 / SPEC-0015 isolation / ADR-0020 / TEST-0025 / IMPL-0021;
owner SRC-0065. The new candidate passes its measured controller/self-test and
bit-20 memory qualification in three no-input headless cold starts and one
owner-operated visible cold start. Every run completes guest second 60 with
`result=00000000 kbd=00000001`. The owner confirmed live Space scene changes.

**Formal B2 remains partial on criterion 6's firmware image hash.** The measured
self-test qualification gap is closed for this declared VM configuration; the
firmware's own vendor/revision and installed container fingerprint do not supply
an image hash. No criterion or SRC-0059 boundary was relaxed.

[Sanitized records](qualification-vm-evidence.json) retain complete own serial
lines, line-arrival brackets, declared settings, register observations, command
option sequences and hashes. Project UUIDs, local paths, owner inventory,
pre-banner serial bytes, firmware internals and PNG payloads are excluded.
Safe original-scene captures and complete management logs remain local.

## Admission

| Item | Identity/result |
| --- | --- |
| Checkout at build/attachment | `16dbab99789b61062087cfc6b03bd18a48bc4e91` |
| Compiled code/test source | `7eed40a67b19f7bbf4471812136d8b69322420cd`; candidate/native code unchanged in this trial |
| O2 EFI | 43,520 bytes; `920ed92c2b30609718d1ed95867e0cf79aab82ae5f9ce76ffee2c9b806ce5d16` |
| O0 EFI | 60,928 bytes; `6a7469c98d75f977b7ce4044901180f14d12728708d0978edfe67eb621ad05f1` |
| Raw GPT/FAT32 disk | 67,108,864 bytes; `e2e6a431722b39d6ed7c215bbc60727aef74ae776f74934f60ddfd0e6f4069e9` |
| VDI | `93d1ef50a6efcedf022fc6f5b6a2f8219e6d8b2f4c54d6ab6ef7b1f5e505af84`; same after trials |
| Conversion | RAW to fixed VDI to RAW gives the original raw hash |
| Platform | VirtualBox `7.2.16r174877`, EFI64/Secure Boot off, SPEC-0015 isolation; 46 settings plus monitor/controller/disk/serial/share checks reviewed before launch |
| Fresh Windows chain | 876 CTest checks in nine configurations: 110/110/110/83/110/95/95/95/68; x64 sanitizers and prior object/link/debugger/EFI controls pass |
| Reproducibility/media | 64 comparison labels/58 distinct fingerprints; nine identical writer images, independent checker, 30 corruption controls and three tool refusals |

Retained full Windows log SHA-256:
`6aab1918b1c501efa12afa71b03a6753bc42da07569822a7d4736253bc7914b1`.
Prior [candidate CI 37930178034](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37930178034)
was independently confirmed successful; the fresh local chain reproduces its
candidate identities. The observed publication CI result is recorded below; CI does not provide
VM/operator evidence.

## Four retained cold starts

| Run | Input | Port before/after | Alias before/after | Native end | Ready-to-result host arrival difference |
| --- | --- | --- | --- | --- | --- |
| cold-01 headless | None | `03/03` | PASS/PASS | 60 s, result 0 | 59.995503 s |
| cold-02 headless | None | `03/03` | PASS/PASS | 60 s, result 0 | 59.913701 s |
| cold-03 headless | None | `03/03` | PASS/PASS | 60 s, result 0 | 60.058071 s |
| cold-04 GUI | Owner Space input, six initial presses | `03/03` | PASS/PASS | 60 s, result 0 | 59.977745 s |

Each fresh serial path starts from powered-off state on the new project machine,
with only its own admitted disk. No saved state, reset/resume or generated key
input is counted. No other project VM ran concurrently; prior machines are intact.

The full qualification line is identical in every run:
`qual port=00000003/00000003 a20=00000000/00000000`.
Both D0 values were read; bits 0–1 survived self-test. Both physical bit-20 probes
passed. Their partner descriptors are type 4; the reviewed native gate requires
eligible data/free RAM, write-back and no runtime attribute before partner access.
This qualifies the named precondition and tested pairs, not every address or PC.

Post-READY public queries read only RIP/RSP. In all four observations RIP is in
our 57,344-byte loaded image, and RSP is between declared stack top minus 64 KiB
and stack top. This supplies an observed owned-stack placement that the earlier
trial inferred from transition code. No guest code/memory or firmware was dumped.

The first headless run uses a different owned bundle/stack/partner address from
the next two. Therefore raw own transcripts are not all byte-identical. The
qualification outcomes, firmware self-report and frame/result sequence match;
all three final original-scene capture hashes match. Runtime allocation addresses
are retained rather than hidden to manufacture byte identity.

The firmware reports `vendor=EDK II`, `rev=00010000`, `state=00000000`
(complete). This is self-reported identity. Firmware derivation remains uncertified;
the container SHA-256 is explicitly not a firmware image hash.

Host observations use a monotonic clock, UTC and nominal 100-ms serial polling.
Actual ready/result arrival-difference brackets span approximately 59.800–60.177
seconds across the four runs. They include delivery/buffering/scheduling and
observer work; no exact guest-clock calibration or physical baud proof is claimed.
Initial/intermediate/final own-scene captures were inspected and retained.

## Operator evidence and deviations

The owner accepted the live test and later confirmed:
“I pressed Space several times and saw the scene change.” The run records six
initial Space presses. The requested two-tap exercise was exceeded; this report
does not claim an exact-two-count trial. It establishes repeated live input and
visible changes on the candidate, separate from the old payload's operator test.
Computer Use only brought the unique project window forward and observed it;
no automated keyboard input was sent.

Secure Boot disable refused without an enrolled platform key. The existing
SPEC-0015 procedure admitted only this documented refusal with explicit public
`SecureBoot="off"`; no key was enrolled or NVRAM contents inspected. All other
preparation commands succeeded. The updated preparation helper completed end to
end, including empty output records. An early GUI window enumeration was empty;
refreshed unique selection succeeded before operator input. Observer draft syntax
was corrected before execution; all four actual collectors completed successfully.

The new VM is powered off and its VDI hash is unchanged. No failed observation
was overwritten, existing VM removed or protected payload introduced.

## What remains

Criterion 6's firmware image hash remains open under the owner's unchanged D2
decision and no-extraction rule. B2 is partial even though these qualification
measurements pass. Native exception delivery, full RAM header inspection and
physical UART/USB/other-PC conformance remain outside this bounded result.

All three full editions remain `not-started`, parity `not-tested`. Historical
System 0.97 APIs, Macintosh 128K feasibility, independent replacement startup,
real Macintosh and mini vMac booting remain required. Human provenance review
is pending. Claude's source-foundation and 128K documents-only lanes remain
with Claude; no new implementation, emulator clearance or OneNote operation
is authorized by this trial.

Publication verification: [Windows CI 38180032136](https://github.com/segaboy/ClasSICK-0.97/actions/runs/38180032136)
passed at `8f330b8127ce7d29128a4b5da424ccf967ba50d8`: 296-file repository
guard, pinned setup, fresh bootstrap twins and the full nine-configuration
876-check hosted/unloaded/media chain. All 64 local/CI comparison labels and
58 distinct fingerprints match, including unchanged O2/O0/raw candidate hashes.
This CI did not run the VM, operator input or register observations. This
follow-up records observed results only.
