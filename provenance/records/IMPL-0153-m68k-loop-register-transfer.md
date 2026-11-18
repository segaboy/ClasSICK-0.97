<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0153 - Bounded DBcc and MOVEM transactions

2026-11-18; SPEC-0123, published facts SRC-0616/0618. Original shared bus
interpreter code adds DBcc and nonzero-mask base-68000 MOVEM, preserving SR,
checked targets, original predecrement base values and postincrement aliases.
The additive atomic transfer descriptor stages up to 64 bytes. The native
286 adapter resolves complete banked destinations before any publication;
old scalar backend layout and entry points remain compatible.

[Canonical receipt](../../docs/development/m68k-transfer-20261118.md) and its
numeric companion own implementation details, exact source/build identities,
review correction, native costs and limits. An independently found misaligned
wrapper member-access defect was fixed before final qualification. All code
and fixtures are independently authored GPL-3.0-or-later. No owner program,
complete CPU, System startup, persistence or physical behavior is claimed.
