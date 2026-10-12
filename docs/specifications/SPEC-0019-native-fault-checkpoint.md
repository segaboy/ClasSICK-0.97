# SPEC-0019 - Native PC terminal-fault checkpoint

Finalized v1, 2026-10-12, lead self-review; human provenance review pending.
SRC-0085 / SRC-0043 / SRC-0059; ADR-0022 / SPEC-0008 v2. Original PC test
extension, no historical Macintosh behavior or firmware eligibility waiver.

The normal build retains its ordinary halt. Two separately built test images
inject a real CPU fault at `cs_native_halt`, after the normal native loop returns:
UD2 (#UD, vector 6, normalized error zero), or loading DS with selector 0x28
(#GP, vector 13, CPU error 0x28). Our GDT limit is 39; that descriptor starts
at byte 40. Saved RIP must identify UD2 or MOV DS respectively. Do not use
software INT as an error-frame substitute. No UART source/entry changes.

The common handler retains the one-shot record and no-call/no-device policy.
After state 2 it exports recorded vector/error/RIP/old-RSP/state/CS into
R9/R10/R11/R12/R13/R15, and RFLAGS/old-SS into RSI/RDI. RCX identifies our
state. R14 contains marker 0x43534631. These are terminal observer outputs,
not preserved interrupted GPRs. Every fault path halts at a separate label;
nested faults must never reach the test injection again. No CR2 observation.

Host admission requires audited deterministic normal/UD/GP images, original
SPEC-0014 media, fixed-VDI raw round trips, exact cleared VirtualBox binary and
reviewed isolation in one fresh VM. Existing VMs and media stay untouched.
Swap only these owned media while powered off; attach only one at once.

For each of three cold starts per fault profile, wait for our banner, ownership,
keyboard readiness, successful qualification and result=0/kbd=1 at second 60.
An injection on an earlier failure is not acceptance. Only then query selected
public CPU registers, never guest/firmware memory, code, CR2 or complete dumps.
Require the expected vector/error, saved RIP equal to owned image base plus
audited probe RVA, state 2/marker, RCX at bundle+495616, live RSP at IST1 top
minus 56, interrupted RSP in the separate 64-KiB ordinary stack, saved CS=8,
SS=16 and IF clear. Two delayed observations must remain in the owned terminal
halt loop with identical captured values. One normal control must still complete
and halt on its ordinary stack. Retain failures and timings; no recovery claim.

Audits reject changed capture/export bytes, wrong probe profile, injection opcode,
terminal destination and omitted objects. Host classifier controls reject wrong
vector/error/RIP/stack/marker/state and missing successful own completion.
Public CI builds/tests only; never runs firmware/emulators. Formal B2 criterion 6,
unobserved vectors/NMI/double faults, physical machines, Mac services, full
System 0.97 compatibility and complete edition/budget gates remain open.
