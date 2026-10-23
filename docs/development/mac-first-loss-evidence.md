# Mac first-loss observation

SRC-0424; SPEC-0072 / IMPL-0074 / TEST-0087. Follow-up to the original ROM's
active interrupts but zero accepted pointer samples. One Copilot Sonnet5.5 low
primary supplies original code and tests from a frozen public-only packet.
Codex integrates its artifact, separates hard faults from losses and corrects
two test expectations. No second worker or external retry. Two reads of the
same prompt (one redundant); no other inputs, writes, commands or executed
worker tests.108.83s; one reported premium request, three provider requests.
Account balances unavailable; no cross-provider quota-saving claim.

A28-byte first-loss record captures the already-read RR3, three raw PB/RR0
snapshots, previous phase/button bits and entry inbox phase. Reason distinguishes
missing phase, unstable sampling, pending mismatch and overflow, in existing
decision order. No added hardware transaction or change in sample admission.
It survives recovery/stop and clears on fresh prepare. Hard faults, unstable
WAIT and already-LOSS calls do not create it. Native MWP1 v2 appends the record
after its unchanged100-byte prefix; size128. Separate diagnostic prefix unchanged.
Readers retain existing interrupt exclusion; this is not an atomic/public ABI.

Actual verification:5750 checks/32 phase cases; two targeted CTests pass in
Clang Debug, GCC Release and Clang ASan/UBSan. Four compiled original faulty
controls are rejected: overwrite-first, swapped snapshots, wrong reason and
hard-fault-as-loss. Test proposal is not test execution; lead runs these checks.
Native compilation/trial pending. No successful Mac input/PC GUI drag, release,
physical or System1 claim. Prior286 evidence remains valid and is not restarted.
Full receipt: [JSON](mac-first-loss-evidence.json).
