# Paired package evidence locator

2026-10-25,SRC-0454/0455;ADR-0057/IMPL-0054/TEST-0060.
The package tools can now bind a later window release to its own evidence
file. Config schema2 requires an explicit ASCII docs/...json path;both builder
and independent auditor reject malformed paths,wrong schema/keys and duplicate
JSON keys. The SOURCE.txt URL pins that path to the separate evidence revision.
Review metadata stays schema1;only schema2 configs add evidence_path.
This checks syntax,not evidence existence,eligibility or release acceptance.

Schema1 retains its original exact keys and fixed preview1 locator. All10 files
in a complete synthetic tree match the old680cdec builder byte-for-byte. Final
local regression passes115 checks including that comparison;the portable CTest
passes114 without the optional old-builder check (11.52s locally). The35 unchanged pre-read literal
cases overlap author cases and are not35 extra unique requirements. Two faulty
builder/auditor old-locator controls are detected. Each component's wrong,
same-length locator is rejected even after inner/outer checksums and review
metadata are resealed. Only original tiny nonexecutable fixture bytes are used.
No admitted native package or source/release input is created or changed.

One Copilot Sonnet5.5 low primary returns nine adopted replacements and211 test
lines in146.28s from30543B frozen original public input. One prompt read plus
one input-directory listing exceeds the one-read cap;no extra source content,
commands,writes or tests. The lead rejects its
unrequested review schema bump,fixes string/bytes and ZIP fixture-type errors,
adds the resealed controls and registers CTest/Linux verification. The initial
fixture failures are retained,not counted as passes. A first lead control used
a different-length locator and correctly hit the earlier size guard;the final
equal-length control checks exact content. Raw output stays outside Git.

[Native verification](mac-input-native-verification.json) records full680cdec
Windows/native CI success and two unbooted first-input diagnostic ROMs. The
normal ROM remains byte-identical;the observer adds80ROM/zeroBSS/+4 conservative
stack-bound bytes in each policy profile. No physical/time/full128K/System1
proof is inferred. Alternate desktop-helper approval remains pending. Mac
motion and PC GUI dragging still block the paired release. No new image/GIF,
freeze,tag,package approval or release publication;63PNG/twoGIF preserved.

Copilot reports one premium request for this slice;no second provider/retry.
All-four remaining balances are unavailable. This is no measurement of Codex
quota savings. [Machine receipt](paired-evidence-locator.json).
