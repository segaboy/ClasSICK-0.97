# Proposed original Windows relative-input test helper

Status: owner explicitly approved the fresh helper and isolated tests on2026-10-25 (SRC-0456). Original implementation and actual limitations are recorded in [the evidence](../windows-input-helper-evidence.md). The former example remains excluded.
Windows11. Codex remains sole writer/integrator/verifier. No implementation,
source example,SDK body,manual page or private HQ material is included here.

Purpose:diagnose the observed mismatch between GUI gestures and guest relative
mouse packets. This does not change guest input policy or waive E1/E2/E3/E4,
source,trial,hardware,native-application or release gates. It supplies no proof
that previous failures were caused by Windows,an emulator or our driver.

Implementation source boundary:
- One fresh supervised CLI worker,without earlier helper/example history.
- Independently reviewed published API declarations,data layouts and named
  constants only;no sample functions,implementation bodies,emulator source,
  manuals,HQ,credentials or untracked research. Retain exact source locators.
- Original small C99 command-line utility plus proposed deterministic backend
  tests. Return artifacts only;no execution,edits,network or installation.
- Codex reviews,integrates and executes host tests before any desktop input.

Hard behavioral limits:
1. A caller identifies exactly one currently owned isolated test process from
   its launch record. Match PID,window ownership and reviewed executable path/
   SHA256. Accept only the pinned Snow or VirtualBox test executable. Reject
   mismatches,missing windows and unrelated/preexisting sessions before input.
2. Read and report foreground ownership. A normal foreground request may be
   attempted once,then verify. Never attach to another app's input thread,
   change foreground-lock rules,inject modifier tricks,write another process,
   bypass a locked/security desktop or alter OS security/privacy preferences.
   No event is sent unless the foreground window belongs to the owned PID.
3. Only bounded relative mouse motion and primary-button down/up. No absolute
   positioning,arbitrary keyboard,text,clipboard,credential or file-dialog
   action. No interaction with Codex,authentication or security apps.
4. Maximum64 explicit steps,each dx/dy in-8..8,delay in10..1000ms,total sequence
   <=30seconds. Reject malformed/excessive input before the first event.
   All arithmetic checked;no silent truncation,clipping or default sequence.
5. Derive logical-primary mapping from the existing host preference;report it,
   do not change it. Validate legal down/up ordering,initial released state
   and one final release. Recheck target foreground ownership before every
   event. Never deliver a cleanup event into an unrelated foreground window;
   on interrupted ownership,stop and report whether a release remains owed.
6. Record requested relative deltas,button transitions,API return counts,
   observed target ownership and elapsed host times. These are host delivery
   receipts only,not guest acceptance or physical chronology. A delivery
   failure is explicit;never auto-replay an uncertain prefix.
7. No network,guest RAM/ROM patch,virtual-device injection,screen scraping,
   helper/runtime library body,driver,service,scheduled task,persistent hook,
   global preference change or arbitrary executable/window selector.
8. Default dry-run validates and prints a plan without input. A separate
   explicit live flag is required. Test the parser/order/failure/foreground
   guards with an injected original backend;those are not native input passes.

Planned live controls,only after source/implementation review and approval:
- Use an already verified original candidate in a fresh isolated approved
  Snow/VirtualBox session. Preserve existing processes,settings and media.
- A no-motion released-button control must preserve qualified guest state.
- Send one small relative axis move,then compare original guest diagnostics
  and framebuffer. A host event count is not an OS motion pass.
- A short down/move/up sequence must produce exactly the intended native
  capture/window translation,then accept later input with no lost/stuck button.
- Qualify exact paired candidates,retain original framebuffer-only evidence,
  and preserve failed controls. No automatic release publication.

Foreground refusal remains a possible result. This proposal promises no fix
to window activation and authorizes no workaround for a refused target.

Current guard refinement,SRC-0492:supersedes the former capture exception. A nonempty owned client and physical WindowFromPoint/root/client containment are required. Capture metadata never substitutes for containment. Query only the owned subtree,then recheck all base guards and refuse outside-client input. No broad desktop traversal or unsafe cleanup fallback. This is not a GUI input pass.
