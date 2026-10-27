# Mac timer refresh and PC stream-start boundary

2026-10-27;SRC-0497..0502/SPEC-0077/IMPL-0085/TEST-0098. Baseline0547842.

Two useful changes are host-verified:an opt-in Mac SCC-status-refresh experiment
and a reproduced PC startup byte-loss fix. Neither establishes usable native
movement/dragging. Paired release remains blocked;no release/tag is published.

The existing Mac trace can be read without inventing a new format. With CPU paused,
the original MWP1 v2/128B at ownBA8 reports23049 services,268 baselines,618 losses,
zero deliveries,pointer256,171. First UNSTABLE triplesBF4C4C/AF4C4C differ inPB4
with the same DCD values. This is before deliberately targeted guest motion,not
proof of a quiet host or physical cause. Existing fields are not repurposed.
Own Snow closes240.06s,original timer ROM7de4329b..86606 unchanged,no guest shutdown.

One existing first-motion PC diagnostic reaches verified own1002D0E4/CSPRAW01.
Raw2A/FF/01 decodes255/-255/buttons2,packet1/errors0,pointer511,341. The Sky gesture
includes positioning/buttons,not an isolated physical1pixel delta. Keep signed
9-bit decoding unchanged. Own VM closes with inventory,running states and media
preserved. This PRE-fix trace must not be labeled a post-fix result.

Claude Opus5.5 Low supplies original Mac code/tests in142.38s from31666 frozen
public/original bytes,zero tools/writes/tests. Output exceeds280-line cap. Lead
adopts the artifact and adds incoherence/Gray-loss/epoch checks. Macro0 remains
default;macro1 enables WR15DCD8 on both channels and resets external/statusA thenB
before each admitted sample. WR1/master remain off. Endpoint polling can still
miss cycles;this is an engineering hypothesis,not a historical or proven fix.
No new interface facts;reuse admitted SPEC-0072/SRC-0363,manuals stay lead-only.

Mac77 assertions and three focused CTests pass in Clang,GCC64,GCC32,ASan/UBSan.
Two compiled faults are detected;macro2 is compile-refused;three invalid builder
modes refuse without output. Default optimized LLVM IR matches after path-only
metadata normalization;this is not native-ROM byte proof. New CI builds original
refresh twins and retains manifest/ROM/map. Native/default-byte/seal/RAM/stack/IRQ
proof precedes any refresh trial;that build and boot are pending.

A distinct Copilot Sonnet5.5 Low review45.48s/25873B identifies AUX discard after
F4 ACK while keyboard-enableAE is still pending. Lead freezes a reactive oracle
BEFORE the fix:old source loses two immediate valid reports (packets0,aux12,
errors0,keyboard29,no fixture violations). One qualified-author fix114.45s/41101B
movesAE to phase20,D4 to21,F4 to22,ACK to23->READY24. Streaming starts last;resend
still repeatsD4/F4. No decoder/API/layout/deadline/retry policy changes.

The same frozen oracle now preserves both reports (packets2,seen2,aux12,errors0,
keyboard29,no violations);the public oracle also rejects compiled old source.
Lead removes a tautological IO check,corrects literal8data/7AUX accounting and
sanitizer-target inheritance. Initial failures remain recorded. PC focused groups
pass8/8 in three64-bit profiles,6/6 inGCC32;the additional frozen oracle passes
all four profiles. Tests prove startup loss and its correction,not GUI root cause.

Post-fix PC normal EFIe634d1a4..b955 and raw EFI9120a968..3fef each pass four
native twins/27 corruption controls;omission controls26/28 respectively. These
images are unbooted/unpackaged. Source-committed manifest checks and independent
media twins precede new trials. Reuse verified artifacts;do not rebuild unchanged
code or repeat unchanged failed gestures without new diagnostic purpose.

Three worker calls total;both Copilot calls use one view,no commands/writes/tests.
Two reported premium requests;Claude reports0.4980224USD session/list cost,not
an assumed subscription bill or balance. All-four remaining balances unavailable.
Grok/Argon idle;qualifications reused,no all-provider review or Codex subagents.
Raw captures remain outside Git. No measured Codex-savings claim.

72PNG/twoGIF unchanged;no successful-drag animation. Prior28612 compiler objects/
six refusals retained,unlinked/unrun;no new286 execution/backend claim. Preserve
24 research originals,held brief,UART,old records/tags/releases and retired wiki.
Published-source,role,native-app,System1,physical128K/B2criterion6 gates persist.

[Contract](../specifications/SPEC-0077-mac-timer-refresh.md),
[actual receipt](timer-refresh-stream-boundary.json).
