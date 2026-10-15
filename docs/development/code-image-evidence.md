# Owned CODE image verification - 2026-10-15

SPEC-0033 / IMPL-0037 / TEST-0041 implements original host-only eager CODE staging, exclusive locked handles, A5-world data population and explicit image retirement. A first allocator OOM retains PREPARED; a later ordinary OOM retains PARTIAL with explicit cleanup and no world writes. Unexpected faults retain exact effects and block further admission. No application execution, guest binding, reset/boot integration, native application or VM/emulator/reference trial. Historical/source/native/physical/full128K/parity/edition/B2 and human-review gates remain open; both required System1/ROM boot directions remain untested.

All2010 final CTests pass across nine configurations: [236, 236, 236, 209, 236, 221, 221, 221, 194]. Both compilers'
debug executable twins match. The inherited full verification chain passes;
a final complete matrix follows every source/test amendment. Ten final production
32/64-bit O0/O2 and ARM64 little/big-endian object audits pass with only8 permitted
own imports, no mutable process globals and no test-injection symbol. ASan/UBSan
are exercised with their inherited positive controls. Pre-final failed attempts
are corrected and excluded. Compiled source `99d4a02ce184c72324afa847a965262ce6dc80ea`;
[publication CI](https://github.com/segaboy/ClasSICK-0.97/actions/runs/38737368099) passes both jobs with2482 CTests:2010 final-matrix
and472 additional fresh-bootstrap checks.

[Implementation and lead dispositions](code-image-implementation.md);
[machine-readable evidence](code-image-evidence.json);
[original test record](../../provenance/records/TEST-0041-owned-code-image.md).

The11 suites include136 original segment partitions, whole-world/payload/padding/
parameter/slack/marker oracles, actual freed source before LOAD, exact64/16 and
524816 staging bounds on a deliberately larger hosted map, real locked first/later
OOM and real retained compaction after later OOM, shared32 slots and scrap/native
preservation, changed identity/epoch/seals/physical aliases/cache/session gates,
and native-only cancellation with independently faulted runtime. Independently
counted135 LOAD and85 RELEASE semantic units are each faulted, checking exact
masks/byte progress/canonical output/low/system/stack and retained first snapshots.
Six before-call fault seams and one PREPARE refusal run separately. Workers execute
zero cases. These are tests of ClasSICK policy, not historical parity or boot proof.

GNU68000 passes16 module import audits and2 complete relocatable links with zero
remaining imports. Service5202, workspace3588, description426, report210, request80,
context40, profile16 and segment20 bytes are measured native68000 costs. The first
five together cost9506 bytes, excluding profile/context, staging up to524816,
actual guest extents/world/stack, manager672/lifecycle112/resource-service3314,
maps/backings/session/gateway/other inventory and cumulative stack. Individual
closure frame maxima608/O0 and604/Os remain partial compiler costs. No complete
physical128K/ROM or cumulative-stack fit is proven. The complete local/CI manifest matches; fingerprints include exact Git-LF source/spec/test/audit inputs.

Claude Code2.1.287 reports claude-opus-5-5 on both low-effort reviews. The first
one-Read/two-turn review is truncated at lines1..668/902 and explicitly partial;
separate one-Read/two-turn20,001-byte tail input is fully read. They report32564
and22075 aggregate tokens (including4564/4899 output and2788/3074 thinking), list
estimates0.278409/0.191341USD. Reports625/722 words exceed requested600/500.
Grok CLI1.0.52 stable0920c8169e27 reports grok-4.7-build, low effort, one completed
read_file/two turns,36082 total tokens including9002 output/7842 reasoning,
list estimate0.03305752USD;598 words fit600. Exact packets/reports/calls and usage
are recorded; no extra reads, execution, edits, web/MCP/subagents. CLI list estimates
are not actual billing, subscription quota or model benchmarks. Final amendments
receive lead review and actual final-source checks, no external re-review claim.

All24 research originals remain unchanged and untracked with byte-identical private
copies. Earlier source/interfaces/boot/assets/UART/build/edition data is preserved.
The complete local boot-media result object matches the previous checkpoint.
The complete CI own-ROM manifest also equals the previous checkpoint, including
image hashes and all source inputs; no new integration or boot trial follows.
Both mandatory original-System1/own-ROM and own-System1/identified-original-ROM
boot directions remain untested. No application CODE, new boot/VM/emulator/reference
trial or native application runs. R-001/AL-2/LQ and prior historical questions,
physical/full128K/parity/three-edition/B2criterion6/human-review gates stay open.
Next is the controlled execution bridge contract, not an automatic application run.
