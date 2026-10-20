# IMPL-0057 / TEST-0065 - Original native window composition

2026-10-20;SRC-0326..0328;SPEC-0054;ADR-0062.
Original GPL-3.0-or-later stateless compositor,one borrowed ordinary-RAM surface
and existing cursor save slot. Hide cursor,reconstruct dirty backdrop/window,
advance revision,show cursor. Preserve early refusal and explicit later fault
stage;no stale rollback,extra framebuffer or rendering of external assets.

Copilot proposes262 header/source lines,within450 cap,one own-prompt view,no
edits/tests. Lead corrects bounds,known label,product return convention,span
validation order and implicit-copy risk. Claude attempt has transport retries
without artifact;Copilot first launch is blocked by sandbox cache EPERM,then
same installed CLI succeeds with approved service/cache access. No reinstall,
HQ/manual/untracked-original/UART input or provider quota inference.

Actual local tests:308 full Clang64 plus3 sanitized/3 GCC32 focused CTests pass.
Independent new-compositor oracle compares2,008 complete images over8layouts;
old backdrop API is shared,not independently reimplemented. Directed injected
failures prove reported stages and explicit repaint recovery,not hardware traps.
4 optimized Clang/GCC32/64 oracles pass;2 compiled defective compositions fail.
12width/endian/EFI object audits pass. Native68000 CI pending at this source
checkpoint.32-byte effect,no persistent compositor state or extra framebuffer.
[Exact evidence and source pins](../../docs/development/window-composition-evidence.json).

Existing pointer/window/cursor/base-scene APIs and boot workload source lists
remain unchanged. Owner/platform binding is next;Mac M1-M7,PC GUI/human and all
existing source/role/historical/edition/physical/full128K/B2/native-app/Linux/
mini-vMac/both System1-ROM gates persist.24originals/held brief/UART/old records,
released assets/tags and retired wiki preserved. No target trial or release.


Native followup:initial and diagnostic CI expose anO2 __modsi3 import in the
unchanged branding renderer. Add original signed remainder adapter to the isolated
compiler closure only,using admitted SRC-0081 interface facts and original
__umodsi3. No GCC implementation/archive or new release-runtime dependency.
Three final remainder CTests pass(10,110 cases each);four optimized32/64-bit
Clang/GCC suites pass. Retained final local total317;prior composition inputs
unchanged. Zero-divisor trap unexecuted. Native closure now42compiles/6twins.
Superseded failed workflows are retained/cancelled,never reported as passing.
