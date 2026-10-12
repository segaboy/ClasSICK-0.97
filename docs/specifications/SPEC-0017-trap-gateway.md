# SPEC-0017 — System 1 trap gateway (draft)

Draft 0, 2026-10-11, written by Claude in the research lane; not finalized, not
implemented, human provenance review pending. Profile `system1-1984`. Evidence:
documented facts from a later manual, *Inside Macintosh* Volumes I–III (SRC-0071),
which describes ROM version 105 and the System file of May 2, 1984 (I-4). Every
documented fact needs January 1984 confirmation (R-001/R-004) before this
contract is finalized. No ADR, implementation or test yet. Architecture:
[System 1 personality](../architecture/system1.md) and
[legacy 68000 strategy](../architecture/legacy-m68k.md).

Labels: **[DOC]** documented fact with page locator; **[D]** this project's design
decision; **[A]** unverified assumption; **[Q]** open question (listed at the
end). Page numbers are printed pages of SRC-0071.

## Purpose and boundary

This contract states what guest 68000 code observes when it calls the Toolbox or
Operating System through a trap word, and through the two routines that read and
replace dispatch entries. It covers trap word decoding, which routine runs,
register and stack conventions at the call boundary, patching, and failure.

It does not cover any individual routine's semantics (each manager needs its own
contract), package routine selectors, the System Error Handler's presentation,
68000 instruction or exception semantics (R-009, SRC-0069), or how ClasSICK
dispatches internally. The implementation's tables, decoding and data structures
are original design under [POLICY](../clean-room/POLICY.md); this document
describes behavior, not Apple's internal instruction sequences.

## Trap words

1. A guest instruction word with bits 15–12 equal to 1010 (hex $A) is a trap word
   **[DOC I-88–89]**. Executing one calls a Toolbox or Operating System routine.
   Words beginning with $F are not trap words; they raise the 68000's line-1111
   exception, which belongs to the CPU contract **[DOC I-88–89, III-45]**.
2. Bit 11 selects the format **[DOC I-89]**:
   - **Toolbox format (bit 11 = 1):** bits 0–8 are the trap number (0–511); bit 9
     is reserved; bit 10 is the auto-pop bit.
   - **Operating System format (bit 11 = 0):** bits 0–7 are the trap number
     (0–255); bit 8 is the A0 flag (see register-based calls); bits 9–10 are
     flags whose meaning depends on the routine **[DOC I-89]**.
3. Both formats index one table of 512 entries **[DOC I-87, I-89]**. A Toolbox
   word and an Operating System word with the same trap number therefore select
   the same entry. Flag bits never change which entry is selected.
4. The routines that read and replace entries take a trap number, not a trap word
   **[DOC III-215, II-384]**.
5. Most Toolbox routines use the stack-based convention and most Operating System
   routines the register-based one, with exceptions **[DOC I-90]**. The book ties
   the register handling below to register-based routines and the A0 flag to the
   Operating System format. This draft takes bit 11 as selecting the convention
   the gateway applies: 0 register-based, 1 stack-based **[A, Q2]**.

## Dispatch

1. Executing a trap word runs the routine currently installed for its trap
   number **[DOC I-88–89]**. Entries are set up when the system starts and may be
   replaced afterward **[DOC I-87]**.
2. When the routine returns, execution continues at the instruction after the
   trap word **[A]**: the book calls the stacked value the return address and the
   routine returns through it (I-91). The 68000's stacked program counter for this
   exception is a CPU-contract detail **[Q8]**.
3. A routine, including an installed replacement, may itself execute trap words;
   nested calls follow this contract **[D]**.
4. The trap does not change A5. Every routine preserves A2–A6 **[DOC I-94–95]**.

## Stack-based calls (bit 11 = 1)

1. **Caller:** for a function, reserve result space on the stack; push the
   parameters in their Pascal declaration order; execute the trap word
   **[DOC I-91]**. Every stack item is 2 or 4 bytes **[DOC I-91]**:
   - 2 bytes: INTEGER, CHAR (code in the low-order byte), BOOLEAN (value in bit 0
     of the high-order byte), Byte and SignedByte (value in the low-order byte).
   - 4 bytes: LONGINT, Ptr, Handle and any VAR parameter (the variable's address).
   - Real-number types and strings are passed as a 4-byte pointer; SINGLE,
     DOUBLE and COMP values are converted to EXTENDED first.
   - A record or array of 4 bytes or fewer is pushed by value, padded to a word;
     a larger one is passed as a 4-byte pointer.
2. **On entry to the routine:** the top of the stack is the 4-byte return
   address, then the parameters, last parameter nearest the top. The processor
   status word that the 68000 pushed with the exception has been removed
   **[DOC I-91, I-92 Figure 4]**.
3. **On return:** the routine has removed its own parameters. A function leaves
   its result in the reserved space, now on top of the stack; a procedure leaves
   the stack as it was before the call **[DOC I-91]**.
4. **Registers:** A2–A6 and D3–D7 are preserved; A0, A1 and D0–D2 may be
   changed **[DOC I-95]**. The condition codes after return are not documented
   **[Q10]**.
5. **Auto-pop (bit 10 = 1):** a language system may call a small sequence that
   holds the trap word followed by a return. The stack then holds two return
   addresses: the trap's on top and, beneath it, the one pushed by the guest's
   own subroutine call. With bit 10 set, when the routine finishes, the trap's
   return address is discarded and control returns directly to the address the
   subroutine call pushed, with that address removed too **[DOC I-89]**. A
   function's result stays on top of the stack **[A]**.
6. **Bit 9 = 1** in a Toolbox word is reserved; behavior is not stated **[Q3]**.

## Register-based calls (bit 11 = 0)

1. **Parameters and results** are in registers, normally an address in A0 and
   other data in D0; routines with more parameters receive a parameter block
   pointer in A0. Each routine's own contract is authoritative **[DOC I-93]**.
2. **On return to the caller** **[DOC I-94–95]**:
   - A1, D1 and D2 have their values from before the call;
   - A0 has its value from before the call if bit 8 = 0, or the routine's
     result if bit 8 = 1;
   - D0 and A7 are as the routine left them;
   - A2–A6 and D3–D7 are preserved by the routine.
3. **Condition codes on return** reflect a word test of D0's low 16 bits, so Z
   is set exactly when that word is 0 **[DOC I-94]**. N, V, C and X follow the
   68000's word-test definition **[A, Q8]**. The book describes the test for
   register-based calls in general and warns that some routines leave D0
   unchanged or return 32 bits in it (I-94); the flags then reflect whatever
   D0's low word holds **[A]**.
4. **On entry to a replacement routine:** the registers that the routine's own
   contract names hold the caller's values, and the top of the stack is a return
   address. Other registers and anything deeper on the stack are not part of this
   contract **[A, Q6]**. How a replacement learns the trap word's flag bits is
   not documented **[Q5]**.

## Callbacks and processor state

1. A guest routine passed to the Toolbox or Operating System as a parameter is
   called with the stack-based convention **[DOC I-93]**. When ClasSICK calls
   it, A5 holds the application's A5 value; the routine may rely on no other
   register **[DOC I-95]**. The correct value is in the global `CurrentA5`
   **[DOC I-95]**.
2. In the original, applications run in supervisor mode on a single stack; the
   user stack pointer is unused, and applications are told not to rely on
   supervisor-only capabilities **[DOC I-92]**. The stack pointer must stay
   even; an odd stack pointer there causes a double bus fault and restart
   **[DOC I-92–93]**. How ClasSICK models supervisor state and an odd stack
   pointer belongs to the CPU contract **[Q8]**.

## Reading and replacing entries

1. **GetTrapAddress**, trap word $A146 **[DOC III-217]**: on entry D0's low word
   is a trap number; on exit A0 holds the address currently installed for it
   **[DOC II-384]**. Bit 8 of its trap word is set, consistent with returning A0.
2. **SetTrapAddress**, trap word $A047 **[DOC III-220]**: on entry A0 holds a
   routine address and D0's low word a trap number; the routine is installed for
   that number **[DOC II-384]**.
3. After `SetTrapAddress(p, n)`, trap words selecting `n` run `p` with the
   convention of the trap word executed, and `GetTrapAddress(n)` returns `p`
   **[A, Q4]**.
4. A replacement may call the address it saved earlier. That call, like any
   direct call to an address obtained from GetTrapAddress, bypasses the gateway:
   the routine preserves only A2–A6 and D3–D7, and none of the register-based
   restoration or condition-code setting happens **[DOC II-384]**.
5. Programs may treat an installed address below `ROMBase` as a patch
   **[DOC II-383]**. In the original, RAM entries reach only 64 KB above the start
   of the system heap, so replacements belong in the system heap
   **[DOC I-88, II-384]**. What happens for an address outside that range, an odd
   address, or a trap number above 511 is not stated **[Q4]**.

## Addresses of built-in routines [D]

ClasSICK contains no Apple ROM, so built-in routines need guest addresses of the
project's own choosing:

1. GetTrapAddress for an implemented, unreplaced trap number returns an even
   guest address at or above `ROMBase` and below `ROMBase + $10000`, the range an
   original ROM entry can reach (I-87–88), so the patch test above stays valid.
   Each trap number has its own address, so saving and reinstalling it restores
   exactly that routine.
2. A direct call to that address performs the routine under the direct-call rules
   above.
3. These addresses are not Apple's ROM addresses, and the bytes a guest reads
   there are not ROM contents. Programs that depend on specific ROM addresses or
   ROM bytes are unsupported (R-017). The value of `ROMBase` comes from the
   low-memory contract (R97-Q-04); the book gives $400000 on the 128K and 512K
   (III-18).

## Failures and unsupported traps

1. In the original, reaching an entry with no implemented routine is system
   error 12, "unimplemented core routine", and a failure of the dispatcher
   itself is system error 9 **[DOC II-364]**. Their presentation belongs to a
   System Error Handler contract.
2. Which trap numbers the January 1984 system implements is not established;
   Appendix C (III-215–226) lists the later set **[Q7]**. Until it is, historical
   error 12 applies only to numbers shown unassigned for the profile.
3. A trap the profile implements but ClasSICK does not yet **[D]** stops the
   guest application deterministically with a ClasSICK unsupported-trap result,
   distinct from error 12. The trap word, guest program counter and profile go
   in the compatibility report. It never returns as if it succeeded and is never
   forwarded to Apple code ([System 1 personality](../architecture/system1.md)).

## Not in this contract

- Direct guest reads or writes of the dispatch table in memory: its address is
  not stated in SRC-0071 (R97-Q-04) **[Q1]**.
- Package traps and their routine selectors; interrupt-level calls; timing and
  cycle counts.
- `classick-extended` changes, which need their own extension contracts.

## Open questions

| ID | Question | Resolution path |
| --- | --- | --- |
| Q1 | Where is the dispatch table, and do 1984 programs read or write it directly? | Low-memory probe plan (R97-Q-04) |
| Q2 | Does the gateway choose register or stack handling from bit 11 alone? | Owner observation; per-routine contracts |
| Q3 | What happens when Toolbox bit 9 is set? | Owner observation |
| Q4 | Result of SetTrapAddress with an out-of-range or odd address, or either routine with a trap number above 511; does GetTrapAddress return exactly what was installed? | Owner observation |
| Q5 | How does a replacement routine learn an Operating System trap word's flag bits? | Further published source or observation |
| Q6 | What is on the stack below the return address when a register-based routine is entered? | Observation; replacement-routine compatibility reports |
| Q7 | Which trap numbers does the January 1984 system implement, and how does its ROM differ from version 105? | R-001 version identification; observation |
| Q8 | Stacked program counter, status word and condition codes for the line-1010 exception and word test; supervisor state; odd stack pointer | SRC-0069 locators; CPU contract (R-009) |
| Q9 | Are the 1985 descriptions of this mechanism unchanged from January 1984? | Cross-check with the March 1985 Promotional Edition (SRC-0068, assembly chapter 1/22/85); observation |
| Q10 | What condition codes does a stack-based call leave? | Observation; treat as unspecified meanwhile |

## Acceptance before finalization

- Every **[DOC]** item confirmed for the January 1984 profile, or explicitly
  marked as a later-manual rule the profile adopts.
- Q2–Q8 resolved or turned into explicit design decisions.
- An ADR for the built-in address range and unsupported-trap result.
- Independently written guest probes for an interpreter test harness: decoding
  of every $Axxx word into format, trap number and flags; stack layout at entry
  and return; register preservation by format and bit 8; condition codes; auto-pop;
  GetTrapAddress/SetTrapAddress round trips; replacement chaining and direct
  calls; nested traps; the unsupported-trap path. No Apple code, ROM bytes or
  sample code are used as fixtures.
