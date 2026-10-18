# Drawing through the selected port -2026-10-18

SPEC-0046 / ADR-0052 / IMPL-0048 / TEST-0053; SRC-0270..0273.
PRF2 adds canonical PaintRect/EraseRect to the real prepared-port owner. It reads
current afresh,validates both bindings,selects the corresponding root-local native
view and applies its offset and rectangular clip. Eight guest Rect bytes are
copied before any overlapping scratch or native pixel write. Every fallible guest
and control check precedes helper completion; only total scratch/frame stores
follow successful drawing. Empty calls count once,counters saturate,prior drawing
survives a subsequent guest fault. Old PRF1/default/fixed modes keep their behavior.
The same original service/owner C compiles for68000/x86; only original legacy
application instructions use the interpreter. UI workloads and code-run.c are
unchanged. No target/emulator/VM/hardware/reference trial or release occurred.

## Worker artifacts and lead review
Frozen header/contract precede both calls and remain byte-identical. Claude
returned18 exact replacements in80.06s,zero tool calls/no edits/tests. Codex
reviewed the complete transaction,mode binding and scalar-only retirement before
integrating. Copilot's first160.75s call exhausted its per-call budget without a
test artifact and opened the provided Claude settings file despite packet-only
instructions. No private/excluded content was present there; deviation retained.
One narrower correction reuses old fixture helpers instead of copying them.
It returned145 test lines in68.89s with two view calls limited to the packet.
No new implementation/history/author logs were supplied to either oracle call.
Workers did not write repositories or execute tests. Codex is sole integrator.
Final oracle corrections/lead gap additions and actual assertions are recorded
in the independent test source and compact JSON receipt; raw streams stay outside Git.
Lead tightened soft result assertions,ended resolved fixture lifetimes,and added
same-root port switch,saturation,Rect/scratch alias,exact refused frame/scratch,
unsupported variants,nonselected invalid view,post-draw trusted mismatch and
retirement after former views end. Entry-table JMP counts as an instruction;
lead corrected its initial step/fault-schedule expectations. The old scalar-tamper
regression exposed a stopped PRF1-to-PRF2 profile change passing retirement.
Lead retained admitted_profile separately and validates equality in the complete
owner record; both old and new tamper cases now reject before cleanup effects.
This is invariant checking,not authentication against hostile native code.

## Executed verification
816 local CTest invocations pass across hosted Clang,ASan/UBSan and GCC32;
the final drawing suite checks361585 assertions per run. Earlier focused debug
attempts exposed the corrected owner-record and lead-oracle issues above.
Twenty strict32/64-bit and little/big-endian owner/coordinator objects pass their
exact import/no-mutable-data audits. Broken implementation controls are assertion
rejected; see the JSON receipt. Tests execute original hosted fixtures only.
CI/native68000 closure results are pending in this implementation commit.

## Limits and next work
Compile/link checks are not a target boot observation. Existing ROM/EFI assets,
previous boot observations and releases retain their exact identities. No full
QuickDraw/GrafPort manager,global dispatch registry,JSR/callback/SDK or native
external loading is implied. Source/edition/physical/full128K/B2/human and both
System1 boot requirements stay open as before. OneNote/wiki remain retired.
Next: a bounded original workload demonstrating selected-port drawing on both
native targets,with fresh owner go-ahead before any target trial.
All four remaining AI account allowances are unknown from these task receipts.
Reported worker consumption is retained without converting provider counters to
ChatGPT quota savings or subscription charges. Grok/Argon qualifications reused.
