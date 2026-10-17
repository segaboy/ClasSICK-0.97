# Grok prepared-port ownership prompt

You are Grok, one focused ownership reviewer for ClasSICK 0.97 on Windows11/PowerShell.
Codex is sole repo writer/integrator. CONTRACT-ONLY task after SPEC43 native core.
Do NOT use tools,files,commands,web,MCP,agents,tests or writes. No HQ,credentials,
manuals,untracked research originals,Apple/MS implementation,ROM/media/binaries,
disassembly,emulator source,manual samples or UART bodies. Supplied content is
original project engineering; no historical assertions. Return one JSON object
{"findings":[{"id":"G-1","severity":"blocking|important|minor","locator":"section",
"problem":"...","proposed_fix":"..."}],"tests_executed":[],"handoff":"..."}.
At most3 actionable findings,<=300words TOTAL. Do not restate the whole contract.
No implementation or test artifact required. One bounded call,requested grok4.7/low.

Review ONLY stopped-owner lifetime and one-way retirement handback in the original
draft excerpts below. Concrete open risk: normal synchronous quiescence ends
borrowed views/capability; image retains scalar profile/g/generation/epoch/consumed/
stop state only. How can retirement validate expected-g ownership WITHOUT hidden
native view pointers,then clear g and hand back to original RELEASE while preserving
consumed-generation protection if RELEASE fails? If quiescence is unprovable,
quarantine must not falsely release a borrow or promise future destruction is safe.
Find any actual missing invariant and return a precise clause fix. Do not demand
runtime proof in this contract-only review. Truthful live native caller and no
async callbacks/reentry are preconditions,not a hostile native sandbox.

Other lead review corrections already planned (don't duplicate as findings):
exact-owner profile seal must cover all scalar identity,not just tag; RMW guest
cell self-overlap is a different domain from native-control vs guest-backing
disjointness (latter never relaxed); all registered bindings are validated each
call; commit holds pinned destinations with no fallible per-byte relookup.
Default/rectangle modes retain zero header. Other28 protected header bytes and
jump table remain checked; malformed guest cell contents don't block cleanup.
No native view pointer in guest report. No opaque guest-record content hashing.


Exact frozen public inputs/excerpts (baseline `39cb7c89f82e5bca2fe5f24c236a1979bcf792af`):

Original Claude proposal sections2/3/5/7 only, SHA256 `0aa7740296f6f038018b25d3cec16304897d0ac452e683d2f35f08b4d77f974c`. Actual input SHA256 `278e3f3c4119eadc21a01cf21cf142a4358b929904247ed10a42469bee1e33dd`; selected excerpts retained outside Git, no manuals or research originals.
