<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0165 - Banked register-based OS trap interception

2026-11-19. SPEC-0135 / TEST-0226. Original Codex-assisted implementation
in `personality/system1/banked-services.[ch]` uses the private logical
directory, a four-entry native return stack, and the guarded RAM provider.
The execution owner supplies a separate state for each CPU. A real A-line
instruction transfers control to installed executable guest RAM and RTS
through its recorded stack slot restores the register-service boundary.
Prior guest RAM handlers can be called by address through the existing CPU.
The earlier public stepping entries retain their narrower admission.

GPL-3.0-or-later. Inputs were the project's SPEC-0017/0020/0129, their
published interface references and original project source and fixtures.
Human provenance review remains open. See the [qualification receipt](../../docs/development/banked-patch-dispatch-20261119.md).
