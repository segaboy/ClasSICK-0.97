# Mac motion prerequisites and capture boundary

2026-10-22; SRC-0362..0365. Source review and original engineering direction for
paired-native-preview-2; no hardware adapter, runtime edit or new target trial.
This extends the dated [M1-M7 assessment](mac-pointer-source-review.md), which is
preserved as a historical checkpoint. The native PC window trial is reused.

The useful next implementation boundary is a small, ordered capture/drain helper
feeding the **existing** native input owner. SCC programming and interrupt entry
remain a separate prerequisite. A once-per-frame poll is not admitted as an
all-edge motion capture method; interrupt delivery alone would not prove that
all physical transitions were retained either.

## Source findings

|Reviewed interface fact|Eligible locator|
|---|---|
|SCC even byte reads, odd byte writes, no word accesses, minimum2.2us spacing|SRC-0080 III-25,PDF32,visually checked|
|PB5/PB4/PB3 are Y2/X2/button; current direction/latch settings leave them inputs/live|SRC-0080 III-39..40,PDF46..47;SRC-0078 PDF3,printed4-5|
|PB reads can clear CB1/CB2 flags under current dependent-control mode;reads are not side-effect-free|SRC-0078 PDF6,printed10-11;current SPEC-0018 PCR0|
|VIA0x64,SCC0x68,combined0x6C are listed vectors;earlier hardware prose conflicts|SRC-0080 III-46,PDF53 versus III-25/41;do not erase disagreement|
|One register pointer is shared between SCC channels and resets after the selected control access|SRC-0363 p30,PDF37|
|Enabled external/status sources latch together **within a channel**;not an event queue|SRC-0363 pp56-59,PDF63-66|
|RR0 may mix live/latched conditions;NMOS/CMOS reads differ from enhanced read-latching|SRC-0363 pp57-58,174-177|
|DCD can latch both transitions;parity of intervening transitions is not an edge count|SRC-0363 p59,PDF66|
|WR15 DCD-enable and RR0 DCD are bit3;RR3 pending is read only from channelA|SRC-0363 pp173,175,177,181|

The new source is Zilog's published *SCC/ESCC User Manual*, UM010902-0609,
June2009,from [the publisher](https://www.zilog.com/docs/serial/scc_escc_um.pdf).
88,330,952 bytes,394 PDF pages,SHA-256
`2a13683ee0e423ee3cdf2781a4ce6b4df65c1e069a269e4234fa6964b4a3bea0`.
Printed page n = PDF n+7 for checked sections. Front matter/edition and selected
register/latch figures were checked. The other three source copies match their
existing byte sizes,hashes and page counts;identities/locators are in the receipt.
Manuals and private page renders stay outside Git and worker inputs. Application
notes,sample code and printed symbolic listings are excluded from adoption.
No Apple algorithm or serial initialization sequence is reconstructed.

These published interface facts do not establish the exact old SCC revision,
physical DCD-to-RR0 polarity,simultaneous SCC/VIA observations,an interrupt
latency bound,or January1984 software behavior. Reading RR0 twice cannot recover
an even number of missed transitions or synchronize it with a separate VIA read.
The old Synertek IER-bit7-read discrepancy remains resolved by the already
recorded AN5 qualification;this profile continues not to read IER.

## Lead dispositions on the engineering proposal

Grok supplied the bounded proposal;the following dispositions are authoritative.
They select a design direction,not a C ABI or a cleared hardware recipe.

|Proposal|Disposition|Adopted direction|
|---|---|---|
|MG1|adopt|Bound capture only;no rendering/allocation/callback in interrupt path.|
|MG2|adopt with changes|One SCC selector/I/O owner also owns relevant VIA reads. PB reads can clear CB1/CB2 flags under current PCR mode;do not promise unrelated flag preservation or reuse startup writes as a shared-driver contract.|
|MG3|adopt with changes|Preserve every accepted normalized record in order. Reject displacement coalescing:clamping and drag paths are nonlinear,even without intervening button changes.|
|MG4|adopt with changes|Sticky detectable loss discards unread prefix and calls existing owner cancel. Rebaseline never creates a press;held left remains suppressed until a later released observation,not any arbitrary edge. Hidden hardware loss has no guaranteed flag.|
|MG5|adopt with changes|Deliver each committed record once through cs_window_input_sample;cancel through cs_window_input_cancel. Do not directly call pointer/window leaves or duplicate counter/cursor composition. On owner fault retain prefix and stop;no auto-replay/repair.|
|MG6|adopt with changes|Quiescent begin and bounded stop barriers;never publish a partial record. Stop ends admission and abandons undelivered records,cancels once if owner ready,retains diagnostics. Later begin cannot replay. Exact masks/publication/epoch exhaustion need the concrete contract.|
|MG7|adopt|Fixed caller-owned storage and bounded stack;exact layout/capacity and measurements remain prerequisites,not claimed128K fit.|
|MG8|adopt with changes|First next slice:freeze an original pure capture/drain API and fixed limits,then sequential synthetic tests/native68000+x64 object checks before MMIO. Current intake creates no API/code/fixtures or test execution.|

The important correction is **no motion coalescing**. Starting at x=0,the
ordered steps -1,+1 clamp to0 then move to1;their sum0 would leave x=0. Window
dragging can also depend on the path. Keep the producer's accepted order rather
than replacing a run of records with a sum. Captured order is software order,
not a claim to know the physical order of simultaneous axes.

Delivery goes through cs_window_input_sample/cancel,which already coordinate
pointer,counter,window,cursor and compositor. Rendering is foreground work.
A capture routine stores bounded complete records or sticky uncertainty and
does no rendering,allocation or arbitrary callbacks. No lock-free guarantee or
volatile-only publication scheme is selected. A later MC68000 contract must
define masks,critical sections,complete-record publication,start/stop barriers
and exhaustion. Partial records are diagnostic state,never deliverable samples.
The frozen worker F3 phrasing meant a DDRB **bit value**0=input,1=output;
it did not assign those functions to PB0/PB1. Lead also adds the VIA read-side
effects missing from that brief and prevents shared-driver preservation claims.

## Remaining admission gaps

|Gap|Status|Still required|
|---|---|---|
|M1|partial|Published access width/parity/spacing,selector and latch semantics reviewed. Exact MMIO profile,pin polarity,register programming/reset/ack/IUS/master-enable remain.|
|M2|partial|Input/latch and PB side effects reviewed;current isolated profile understood. Shared VIA ownership/flags and cross-chip coherence remain.|
|M3|partial|VIA0x64/SCC0x68/combined0x6C vector table corroborated;earlier prose disagreement retained. Actual CPU mask/vector/entry-save/RTE/reentry/start-stop contract remains.|
|M4|partial|Coalescing/hidden-transition and cross-chip sampling limitations identified. No valid latency bound or all-edge guarantee.|
|M5|design direction reviewed|Ordered bounded capture,sticky visible-loss/cancel and one foreground owner selected;no code or concurrency proof.|
|M6|open|2009 family documentation distinguishes NMOS/CMOS/ESCC;does not prove a particular1984 part,Snow model or physical board.|
|M7|open|No new ROM/RAM/stack/IRQ measurements or target tests.|

No M-gap is declared fully closed. The immediate next slice is to freeze the
pure helper's caller-owned layout/capacity,normalized-record semantics and
refusal/lifecycle API,then implement and verify it sequentially on hosted builds
and native68000/x64 objects. That does not install an ISR or drive MMIO. Hardware
binding requires the exact register/timing/polarity/CPU contract and later a
separately approved source-pinned trial. Existing VIA polling must not become a
competing read owner in that future profile. Mac compositor framebuffer binding
and total ROM/RAM/stack cost also remain release prerequisites.

## Verification and orchestration

[TEST-0075](../../provenance/records/TEST-0075-mac-motion-capture-plan.md) contains
12 revised **unexecuted** cases. No fixture,code/header,feature test,compiler
build,VM/emulator/reference/hardware run,release or version-pointer change here.
Existing PC qualification/shared-helper tests are reused,not rerun or credited
as Mac motion evidence. Document/source guards validate records only.

One Grok4.7 low call resolved grok-4.7-build,99.03s,17,051 original/reviewed-public
bytes,242 lines;70 artifact lines/1160 words within150/1700 caps. Zero observed
tools/writes/tests,retry,second worker or requalification. Lead adopts the draft
with the concrete coalescing,owner-route,release-suppression and lifecycle
corrections above. Raw captures stay in C:/ClasSICK/mac-motion-prerequisites-20261022-a.
[Prompt/input controls](tasks/mac-motion-prerequisites-grok.md),[receipt](mac-motion-prerequisites.json).
Reported19,054 task tokens/$0.0221272 list/session cost are not remaining balance
or measured Codex savings. All current external allowances unavailable;other
workers idle. Qualification/CLI settings are reused instead of a fresh audit.

Preserve24 originals,held brief,UART bodies,old records/checkpoints,release
assets/tags and retired wiki. Native-app,historical/edition,physical/full128K,
B2,GUI/human,BIOS,Linux and both System1-ROM gates remain. No Mac parity or
paired release acceptance claimed. Ordinary public/private HQ Git records only.


Dated follow-up2026-10-23:SPEC-0063 sequential implementation is verified,but
is not an IRQ queue. [SPEC-0064 binding review](mac-scc-binding-review.md) selects
a separate inbox/short claim and PRIMING baseline staging before unmasked
foreground delivery. Fourteen cases planned;no hardware/CPU binding admitted.
Additional SCC enable/IP/IUS/reset facts reviewed;WR0 diagram/prose encoding
disagreement and physical status polarity remain unresolved. M1-M7 stay open
or partial. Next proposal:pure inbox/interleaving implementation before driver.
